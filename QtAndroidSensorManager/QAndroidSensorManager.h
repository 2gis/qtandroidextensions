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

#pragma once

#include <QtCore/QObject>

#include <QJniHelpers/IJniObjectLinker.h>



class QAndroidSensorManager : public QObject
{
	Q_OBJECT
	JNI_LINKER_DECL(QAndroidSensorManager)

public:
	QAndroidSensorManager(QObject * parent);
	virtual ~QAndroidSensorManager();

	bool start(int32_t sensorType, int32_t delayUs = -1, int32_t latencyUs = -1);
	void stop(int32_t sensorType);
	void stop();
	bool isStarted(int32_t sensorType) const;

	float getAzimuthByRotationVector(bool applyDisplayRotation);
	float getAzimuthByMagneticField(bool applyDisplayRotation);

	static int32_t getSensorType(const char * sensorTypeName);

signals:
	void dataUpdated(int32_t sensor_type, int64_t timestamp_ns, std::vector<float> data);

private:
	void onUpdate(int32_t sensor_type, int64_t timestamp_ns, std::vector<float> & data);

private:
	friend void JNICALL Java_QAndroidSensorManager_onUpdate(
		JNIEnv * env,
		jobject,
		jlong inst,
		jint sensor_type,
		jlong timestamp_ns,
		jfloatArray jdata);
};
