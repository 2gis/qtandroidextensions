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

#include "QGeoPositionInfoSourceAndroidGms.h"

#include <QtPositioning/QGeoPositionInfo>
#include <QJniHelpers/QAndroidQPAPluginGap.h>
#include "QAndroidGmsLocationProvider.h"
#include "QLocationManagerProvidersListener.h"


Q_DECLARE_METATYPE(QGeoPositionInfo)


/*!
	\class QGeoPositionInfoSourceAndroidGms
	\inherits QGeoPositionInfoSource
	\inheaderfile QGeoPositionInfoSourceAndroidGms.h
	\inmodule QtAndroidLocation
	\ingroup QtAndroidLocationGroup
	\ingroup GeoPositionInfoSources
	\brief Geo position info source for android GMS

	GMS position info source. Recieves data from QAndroidGmsLocationProvider
	\sa QAndroidGmsLocationProvider
*/


QGeoPositionInfoSourceAndroidGms::QGeoPositionInfoSourceAndroidGms(QObject * parent)
	: QGeoPositionInfoSource(parent)
	, regularProvider_(NULL)
	, providersListener_(NULL)
	, updatesRunning_(false)
	, m_error(NoError)
	, activeProvidersDisabled_(false)
{
	qRegisterMetaType<QGeoPositionInfo>();

	providersListener_ = new QLocationManagerProvidersListener(this);
	regularProvider_ = new QAndroidGmsLocationProvider(this);
	setPreferredPositioningMethods(NonSatellitePositioningMethods);

	if (providersListener_)
	{
		QObject::connect(providersListener_, &QLocationManagerProvidersListener::providersChange,
							this, &QGeoPositionInfoSourceAndroidGms::onProvidersChange);

		onProvidersChange(providersListener_->isActiveProvidersEnabled());
	}

	if (regularProvider_)
	{
		QObject::connect(regularProvider_, &QAndroidGmsLocationProvider::locationRecieved,
						  this, &QGeoPositionInfoSourceAndroidGms::processRegularPositionUpdate);

		QObject::connect(regularProvider_, &QAndroidGmsLocationProvider::statusChanged,
						  this, &QGeoPositionInfoSourceAndroidGms::onStatusChanged);
	}
}


QGeoPositionInfoSourceAndroidGms::~QGeoPositionInfoSourceAndroidGms()
{
	stopUpdates();
}


void QGeoPositionInfoSourceAndroidGms::preloadJavaClasses()
{
	QAndroidGmsLocationProvider::preloadJavaClasses();
}


bool QGeoPositionInfoSourceAndroidGms::isAvailable(bool allowDialog /*= false*/)
{
	return QAndroidGmsLocationProvider::isAvailable(allowDialog);
}


int QGeoPositionInfoSourceAndroidGms::getGmsVersion()
{
	return QAndroidGmsLocationProvider::getGmsVersion();
}


void QGeoPositionInfoSourceAndroidGms::startUpdates()
{
	if (updatesRunning_)
	{
		return;
	}

	Q_ASSERT(regularProvider_);
	const PositioningMethods methods = preferredPositioningMethods();

	if (methods == 0) 
	{
		setError(UnknownSourceError);
		return;
	}

	QAndroidGmsLocationProvider::enPriority priority = QAndroidGmsLocationProvider::PRIORITY_LOW_POWER;

	if (QGeoPositionInfoSource::NonSatellitePositioningMethods & methods)
	{
		priority = QAndroidGmsLocationProvider::PRIORITY_BALANCED_POWER_ACCURACY;
	}

	if (QGeoPositionInfoSource::SatellitePositioningMethods & methods)
	{
		priority = QAndroidGmsLocationProvider::PRIORITY_HIGH_ACCURACY;
	}

	regularProvider_->setPriority(priority);

	updatesRunning_ = true;

	Q_ASSERT(regularProvider_);
	regularProvider_->setUpdateInterval(updateInterval(), minimumUpdateInterval());
	regularProvider_->startUpdates();

	onProvidersChange(providersListener_->isActiveProvidersEnabled());
}


void QGeoPositionInfoSourceAndroidGms::stopUpdates()
{
	Q_ASSERT(regularProvider_);
	updatesRunning_ = false;
	regularProvider_->stopUpdates();
}


void QGeoPositionInfoSourceAndroidGms::setUpdateInterval(int msec)
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


QGeoPositionInfo QGeoPositionInfoSourceAndroidGms::lastKnownPosition(bool fromSatellitePositioningMethodsOnly /*= false*/) const
{
	Q_UNUSED(fromSatellitePositioningMethodsOnly);
	Q_ASSERT(regularProvider_);
	return regularProvider_->lastKnownPosition();
}


void QGeoPositionInfoSourceAndroidGms::setPreferredPositioningMethods(const PositioningMethods methods)
{
	const PositioningMethods previousPreferredPositioningMethods = preferredPositioningMethods();
	QGeoPositionInfoSource::setPreferredPositioningMethods(methods);

	if (previousPreferredPositioningMethods == preferredPositioningMethods())
	{
		return;
	}

	if (updatesRunning_)
	{
		reconfigureRunningSystem();
	}
}


QGeoPositionInfoSource::PositioningMethods QGeoPositionInfoSourceAndroidGms::supportedPositioningMethods() const
{
	return QGeoPositionInfoSource::AllPositioningMethods;
}


int QGeoPositionInfoSourceAndroidGms::minimumUpdateInterval() const
{
	return 500;
}

void QGeoPositionInfoSourceAndroidGms::setError(Error errval)
{
	if (m_error != errval)
	{
		m_error = errval;
		emit QGeoPositionInfoSource::error(error());
	}
}


QGeoPositionInfoSource::Error QGeoPositionInfoSourceAndroidGms::error() const
{
	if (activeProvidersDisabled_)
	{
		return QGeoPositionInfoSource::ClosedError;
	}

	return m_error;
}


void QGeoPositionInfoSourceAndroidGms::requestUpdate(int timeout)
{
	Q_ASSERT(regularProvider_);
	regularProvider_->requestUpdate(timeout);
}


void QGeoPositionInfoSourceAndroidGms::processRegularPositionUpdate(QGeoPositionInfo location)
{
	emit positionUpdated(location);
}


void QGeoPositionInfoSourceAndroidGms::locationProviderDisabled()
{
	setError(QGeoPositionInfoSource::ClosedError);
}


void QGeoPositionInfoSourceAndroidGms::onProvidersChange(bool status)
{
	activeProvidersDisabled_ = !status;
	emit QGeoPositionInfoSource::error(error());
}


void QGeoPositionInfoSourceAndroidGms::onStatusChanged(int status)
{
	switch (status)
	{
		case QAndroidGmsLocationProvider::S_DISCONNECTED:
		case QAndroidGmsLocationProvider::S_CONNECT_ERROR:
		case QAndroidGmsLocationProvider::S_CONNECT_SUSPEND:
			setError(QGeoPositionInfoSource::ClosedError);
			break;

		case QAndroidGmsLocationProvider::S_CONNECTED:
			setError(QGeoPositionInfoSource::NoError);
			break;

		default:
			setError(QGeoPositionInfoSource::UnknownSourceError);
			break;
	};
}


void QGeoPositionInfoSourceAndroidGms::reconfigureRunningSystem()
{
	if (!updatesRunning_)
	{
		return;
	}

	stopUpdates();
	startUpdates();
}


