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
#include <QtAndroidSensorManager/QAndroidSensorManager.h>
#include <QtCore/QMutexLocker>
#include <cmath>



QAndroidAccelerometer::QAndroidAccelerometer(QObject * parent)
	: QObject(parent)
	, started_(false)
{
	sensor_manager_ = new QAndroidSensorManager(this);

	QObject::connect(
		sensor_manager_.data(),
		&QAndroidSensorManager::dataUpdated,
		this,
		&QAndroidAccelerometer::onUpdate
	);
}


QAndroidAccelerometer::~QAndroidAccelerometer()
{
	stop();
}


void QAndroidAccelerometer::preloadJavaClasses()
{
	QAndroidSensorManager::preloadJavaClasses();
}


void QAndroidAccelerometer::start(int32_t delayMicroSeconds /*= -1*/, int32_t latencyMicroSeconds /*= -1*/)
{
	const static int32_t type = sensor_manager_->getSensorType("TYPE_ACCELEROMETER");
	started_ = sensor_manager_->start(type, delayMicroSeconds, latencyMicroSeconds);
}


void QAndroidAccelerometer::stop()
{
	sensor_manager_->stop();
	started_ = false;
}


bool QAndroidAccelerometer::isStarted() const
{
	return started_;
}


float QAndroidAccelerometer::getAcceleration()
{
	float square = 0.f;

	assert(3 == data_.size());

	std::for_each(
		data_.cbegin(),
		data_.cend(),
		[&square](const float val)
		{
			square += val * val;
		}
	);


	return std::sqrtf(square);
}


void QAndroidAccelerometer::onUpdate(int32_t sensor_type, int64_t timestamp_ns, std::vector<float> data)
{
	data_ = data;
	emit accelerationUpdated();
}
