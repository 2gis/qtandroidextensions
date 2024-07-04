/*
  Offscreen Android Views library for Qt

  Author:
  Vyacheslav O. Koscheev <vok1980@gmail.com>

  Distrbuted under The BSD License

  Copyright (c) 2015, DoubleGIS, LLC.
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


#include "QAndroidCompass.h"
#include <QtAndroidSensorManager/QAndroidSensorManager.h>


QAndroidCompass::QAndroidCompass(QObject * parent)
	: QObject(parent)
	, started_(false)
	, mode_(MODE_UNKNOWN)
{
	sensor_manager_ = new QAndroidSensorManager(this);

	QObject::connect(
		sensor_manager_.data(),
		&QAndroidSensorManager::dataUpdated,
		this,
		&QAndroidCompass::onUpdate
	);
}


QAndroidCompass::~QAndroidCompass()
{
	stop();
}


void QAndroidCompass::start(int32_t delayUs /*= -1*/, int32_t latencyUs /*= -1*/)
{
	const static int32_t type_rotation_vector = sensor_manager_->getSensorType("TYPE_ROTATION_VECTOR");
	if (MODE_UNKNOWN == mode_ || MODE_ROTATION == mode_)
	{
		started_ = sensor_manager_->start(type_rotation_vector, delayUs, latencyUs);
	}

	if (started_)
	{
		mode_ = MODE_ROTATION;
	}
	else
	{
		const static int32_t type_accelerometer = sensor_manager_->getSensorType("TYPE_ACCELEROMETER");
		const static int32_t type_magnetic_field = sensor_manager_->getSensorType("TYPE_MAGNETIC_FIELD");

		started_ =
			sensor_manager_->start(type_accelerometer, delayUs, latencyUs) &&
			sensor_manager_->start(type_magnetic_field, delayUs, latencyUs);

		if (started_)
		{
			mode_ = MODE_MAGNETIC;
		}
	}
}


void QAndroidCompass::stop()
{
	sensor_manager_->stop();
	started_ = false;
}


bool QAndroidCompass::isStarted() const
{
	return started_;
}


float QAndroidCompass::getAzimuth()
{

	if (MODE_ROTATION == mode_)
	{
		return sensor_manager_->getAzimuthByRotationVector(true);
	}
	else
	{
		return sensor_manager_->getAzimuthByMagneticField(true);
	}

}


void QAndroidCompass::onUpdate(int32_t sensor_type, int64_t timestamp_ns, std::vector<float> data)
{
	emit azimuthUpdated();
}


