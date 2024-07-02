/*
  Offscreen Android Views library for Qt

  Author:
  Vyacheslav O. Koscheev <vok1980@gmail.com>
  Eugene A. Samoylov <ghelius@gmail.com>

  Distrbuted under The BSD License

  Copyright (c) 2024, DoubleGIS, LLC.
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.
  * Neither the name of the DoubleGIS, LLC nor the names of its contributors
    may be used to endorse or promote products derived from this software
    without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
  BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
  THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "QAndroidSensorManager.h"

#include <QJniHelpers/QJniHelpers.h>
#include <QJniHelpers/TJniObjectLinker.h>
#include <algorithm>



Q_DECL_EXPORT void JNICALL Java_QAndroidSensorManager_onUpdate(
	JNIEnv * env,
	jobject,
	jlong inst,
	jint sensor_type,
	jlong timestamp_ns,
	jfloatArray jdata)
{
	std::vector<float> data = QJniEnvPtr(env).convert(jdata);
	JNI_LINKER_OBJECT(QAndroidSensorManager, inst, proxy)
	proxy->onUpdate(sensor_type, timestamp_ns, data);
}


static const JNINativeMethod methods[] = {
	{"getActivity", "()Landroid/app/Activity;", reinterpret_cast<void*>(QAndroidQPAPluginGap::getActivityNoThrow)},
	{"onUpdate", "(JIJ[F)V", reinterpret_cast<void*>(Java_QAndroidSensorManager_onUpdate)},
};


JNI_LINKER_IMPL(
	QAndroidSensorManager,
	"ru/dublgis/androidsensormanager/SensorProvider",
	methods)


QAndroidSensorManager::QAndroidSensorManager(QObject * parent)
	: QObject(parent)
	, jniLinker_(new JniObjectLinker(this))
{
	qRegisterMetaType<int32_t>("int32_t");
	qRegisterMetaType<int64_t>("int64_t");
	qRegisterMetaType<std::vector<float>>();
}


QAndroidSensorManager::~QAndroidSensorManager()
{
	stop();
}


bool QAndroidSensorManager::start(int32_t sensorType, int32_t delayUs /*= -1*/, int32_t latencyUs /*= -1*/)
{
	if (isJniReady())
	{
		return jni()->callParamBoolean(
			"start",
			"III",
			static_cast<jint>(sensorType),
			static_cast<jint>(delayUs),
			static_cast<jint>(latencyUs));
	}

	return false;
}


void QAndroidSensorManager::stop()
{
	if (isJniReady())
	{
		jni()->callVoid("stopAll");
	}
}


void QAndroidSensorManager::stop(int32_t sensorType)
{
	if (isJniReady())
	{
		jni()->callParamVoid("stop", "I", static_cast<jint>(sensorType));
	}
}


bool QAndroidSensorManager::isStarted(int32_t sensorType) const
{
	if (isJniReady())
	{
		return jni()->callParamBoolean(
			"isStarted",
			"I",
			static_cast<jint>(sensorType));
	}

	return false;
}


void QAndroidSensorManager::onUpdate(int32_t sensor_type, int64_t timestamp_ns, std::vector<float> & data)
{
	emit dataUpdated(sensor_type, timestamp_ns, data);
}


// "TYPE_ACCELEROMETER"
// "TYPE_GYROSCOPE"
// "TYPE_MAGNETIC_FIELD"
// "TYPE_ROTATION_VECTOR"
int32_t QAndroidSensorManager::getSensorType(const char * sensorTypeName)
{
	const jint type = QJniClass("android/hardware/Sensor").getStaticIntField(sensorTypeName);
	return type;
}


float QAndroidSensorManager::getAzimuthByRotationVector(bool applyDisplayRotation)
{
	float data = 0.f;

	if (isJniReady())
	{
		data = jni()->callParamFloat("getAzimuthByRotationVector", "Z", jboolean(true));
	}

	return data;
}


float QAndroidSensorManager::getAzimuthByMagneticField(bool applyDisplayRotation)
{
	float data = 0.f;

	if (isJniReady())
	{
		data = jni()->callParamFloat("getAzimuthByMagneticField", "Z", jboolean(true));
	}

	return data;
}
