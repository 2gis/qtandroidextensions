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

#include "QGeoPositionInfoSourceAndroidGPS.h"

#include <QtPositioning/QGeoPositionInfo>
#include <QAndroidQPAPluginGap.h>
#include "QAndroidGooglePlayServiceLocationProvider.h"


Q_DECLARE_METATYPE(QGeoPositionInfo)


QGeoPositionInfoSourceAndroidGPS::QGeoPositionInfoSourceAndroidGPS(QObject * parent) :
	QGeoPositionInfoSource(parent),
	m_error(NoError),
	regularProvider_(NULL),
	updatesRunning_(false)
{
	qRegisterMetaType< QGeoPositionInfo >();

	setPreferredPositioningMethods(AllPositioningMethods);
	regularProvider_ = new QAndroidGooglePlayServiceLocationProvider(this);

	if (regularProvider_)
	{
		QObject::connect(regularProvider_, SIGNAL(locationRecieved(const QGeoPositionInfo&)),
						  this, SLOT(processRegularPositionUpdate(const QGeoPositionInfo&)));

		QObject::connect(regularProvider_, SIGNAL(statusChanged(int)),
						  this, SLOT(onStatusChanged(int)));
	}
}


QGeoPositionInfoSourceAndroidGPS::~QGeoPositionInfoSourceAndroidGPS()
{
	stopUpdates();
}


bool QGeoPositionInfoSourceAndroidGPS::isAvailable()
{
	return QAndroidGooglePlayServiceLocationProvider::isAvailable();
}


void QGeoPositionInfoSourceAndroidGPS::startUpdates()
{
	if (updatesRunning_)
	{
		return;
	}

	if (preferredPositioningMethods() == 0) 
	{
		m_error = UnknownSourceError;
		emit QGeoPositionInfoSource::error(m_error);
		return;
	}

	updatesRunning_ = true;

	Q_ASSERT(regularProvider_);
	regularProvider_->setUpdateInterval(updateInterval(), minimumUpdateInterval());
	regularProvider_->startUpdates();
}


void QGeoPositionInfoSourceAndroidGPS::stopUpdates()
{
	Q_ASSERT(regularProvider_);
	updatesRunning_ = false;
	regularProvider_->stopUpdates();
}


void QGeoPositionInfoSourceAndroidGPS::setUpdateInterval(int msec)
{
	int previousInterval = updateInterval();
	msec = (((msec > 0) && (msec < minimumUpdateInterval())) || msec < 0)? minimumUpdateInterval() : msec;

	if (msec == previousInterval)
	{
		return;
	}

	QGeoPositionInfoSource::setUpdateInterval(msec);

	if (updatesRunning_)
	{
		reconfigureRunningSystem();
	}
}


QGeoPositionInfo QGeoPositionInfoSourceAndroidGPS::lastKnownPosition(bool fromSatellitePositioningMethodsOnly /*= false*/) const
{
	Q_UNUSED(fromSatellitePositioningMethodsOnly);
	Q_ASSERT(regularProvider_);
	return regularProvider_->lastKnownPosition();
}


void QGeoPositionInfoSourceAndroidGPS::setPreferredPositioningMethods(PositioningMethods methods)
{
	PositioningMethods previousPreferredPositioningMethods = preferredPositioningMethods();
	QGeoPositionInfoSource::setPreferredPositioningMethods(methods);
	if (previousPreferredPositioningMethods == preferredPositioningMethods())
		return;

	if (updatesRunning_)
	{
		reconfigureRunningSystem();
	}
}


QGeoPositionInfoSource::PositioningMethods QGeoPositionInfoSourceAndroidGPS::supportedPositioningMethods() const
{
	return QGeoPositionInfoSource::AllPositioningMethods;
}


int QGeoPositionInfoSourceAndroidGPS::minimumUpdateInterval() const
{
	return 1000;
}

QGeoPositionInfoSource::Error QGeoPositionInfoSourceAndroidGPS::error() const
{
	return m_error;
}


void QGeoPositionInfoSourceAndroidGPS::requestUpdate(int timeout)
{
	Q_ASSERT(!"Not implemented");
	emit updateTimeout();
	return;
}


void QGeoPositionInfoSourceAndroidGPS::processRegularPositionUpdate(const QGeoPositionInfo& location)
{
	emit positionUpdated(location);
}


void QGeoPositionInfoSourceAndroidGPS::locationProviderDisabled()
{
	m_error = QGeoPositionInfoSource::ClosedError;
	emit QGeoPositionInfoSource::error(m_error);
}


void QGeoPositionInfoSourceAndroidGPS::onStatusChanged(int status)
{
	Error newErrorCode = QGeoPositionInfoSource::NoError;

	switch (status)
	{
		case QAndroidGooglePlayServiceLocationProvider::S_DISCONNECTED:
			newErrorCode = QGeoPositionInfoSource::ClosedError;
			break;

		case QAndroidGooglePlayServiceLocationProvider::S_CONNECTED:
			newErrorCode = QGeoPositionInfoSource::NoError;
			break;

		default:
			newErrorCode = QGeoPositionInfoSource::UnknownSourceError;
			break;
	};

	if (m_error != newErrorCode)
	{
		m_error = newErrorCode;
		emit QGeoPositionInfoSource::error(m_error);
	}
}


void QGeoPositionInfoSourceAndroidGPS::reconfigureRunningSystem()
{
	if (!updatesRunning_)
	{
		return;
	}

	stopUpdates();
	startUpdates();
}


