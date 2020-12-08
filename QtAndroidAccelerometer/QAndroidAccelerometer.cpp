/*
  Offscreen Android Views library for Qt

  Author:
  Eugene Samoylov <ghelius@gmail.com>

  Distrbuted under The BSD License

  Copyright (c) 2020, DoubleGIS, LLC.
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


#include "QAndroidAccelerometer.h"

#include <QtCore/QSharedPointer>
#include <QtCore/QMutexLocker>
#include <QJniHelpers/QAndroidQPAPluginGap.h>
#include <QJniHelpers/TJniObjectLinker.h>


Q_DECL_EXPORT void JNICALL Java_AccelerometerProvider_onUpdate(JNIEnv * env, jobject, jlong inst)
{
	Q_UNUSED(env);

	JNI_LINKER_OBJECT(QAndroidAccelerometer, inst, proxy)
	proxy->onUpdate();
}


static const JNINativeMethod methods[] = {
	{"getActivity",
	 "()Landroid/app/Activity;",
	 reinterpret_cast<void *>(QAndroidQPAPluginGap::getActivityNoThrow)},
	{"onUpdate", "(J)V", reinterpret_cast<void *>(Java_AccelerometerProvider_onUpdate)},
};


JNI_LINKER_IMPL(
	QAndroidAccelerometer, "ru/dublgis/androidaccelerometer/AccelerometerProvider", methods)


QAndroidAccelerometer::QAndroidAccelerometer(QObject * parent)
	: QObject(parent)
	, jniLinker_(new JniObjectLinker(this))
	, started_(false)
{
}


QAndroidAccelerometer::~QAndroidAccelerometer()
{
	stop();
}


void QAndroidAccelerometer::start(int32_t delayMicroSeconds /*= -1*/, int32_t latencyMicroSeconds /*= -1*/)
{
	if (!started_ && isJniReady())
	{
		started_ = jni()->callParamBoolean(
			"start", "II", static_cast<jint>(delayMicroSeconds), static_cast<jint>(latencyMicroSeconds));
	}
}


void QAndroidAccelerometer::stop()
{
	if (isJniReady())
	{
		jni()->callVoid("stop");
		started_ = false;
	}
}


bool QAndroidAccelerometer::isStarted() const
{
	return started_;
}


float QAndroidAccelerometer::getAcceleration()
{
	jfloat data;

	if (isJniReady())
	{
		data = jni()->callFloat("getAccelerationModule");
	}

	return data;
}


void QAndroidAccelerometer::onUpdate()
{
	emit accelerationUpdated();
}
