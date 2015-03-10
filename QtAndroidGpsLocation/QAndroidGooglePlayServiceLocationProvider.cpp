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

#include "QAndroidGooglePlayServiceLocationProvider.h"
#include <QAndroidQPAPluginGap.h>
#include <QtPositioning/QGeoPositionInfo>


static const char * const c_full_class_name_ = "ru/dublgis/androidgpslocation/GooglePlayServiceLocationProvider";



static void setPositionAttributeFloat(QGeoPositionInfo &info, QGeoPositionInfo::Attribute attr, 
										QJniObject &location, const char* szCheck, const char* szGet)
{
	if (location.callBool(szCheck))
	{
		jfloat val = location.callFloat(szGet);
		info.setAttribute(attr, val);
	}
}


static QGeoPositionInfo positionInfoFromJavaLocation(JNIEnv * jniEnv, const jobject &jlocation)
{
	QGeoPositionInfo info;
	QJniObject location(jlocation, true);

	if (!location)
	{
		qWarning() << "null location";
		return QGeoPositionInfo();
	}

	jdouble latitude = location.callDouble("getLatitude");
	jdouble longitude = location.callDouble("getLongitude");
	QGeoCoordinate coordinate(latitude, longitude);

	if (location.callBool("hasAltitude"))
	{
		jdouble value = location.callDouble("getAltitude");
		coordinate.setAltitude(value);
	}

	info.setCoordinate(coordinate);

	jlong timestamp = location.callLong("getTime");
	info.setTimestamp(QDateTime::fromMSecsSinceEpoch(timestamp));

	setPositionAttributeFloat(info, QGeoPositionInfo::HorizontalAccuracy,	location, "hasAccuracy", 	"getAccuracy");
	setPositionAttributeFloat(info, QGeoPositionInfo::GroundSpeed,		 	location, "hasSpeed",		"getSpeed");
	setPositionAttributeFloat(info, QGeoPositionInfo::Direction,			location, "hasBearing", 	"getBearing");

	return info;
}



Q_DECL_EXPORT void JNICALL Java_GooglePlayServiceLocationProvider_locationStatus(JNIEnv * env, jobject, jlong param, jint state)
{
	try
	{
		if (param)
		{
			void * vp = reinterpret_cast<void*>(param);
			QAndroidGooglePlayServiceLocationProvider * proxy = reinterpret_cast<QAndroidGooglePlayServiceLocationProvider*>(vp);
			proxy->onStatusChanged(state);
			return;
		}
		else
		{
			qWarning() << __FUNCTION__ << "Zero param!";
		}

	}
	catch (std::exception & e)
	{
		qWarning() << __FUNCTION__ << " exception: " << e.what();
	}
}


Q_DECL_EXPORT void JNICALL Java_GooglePlayServiceLocationProvider_locationRecieved(JNIEnv * env, jobject, jlong param, jobject location, jboolean initial)
{
	try
	{
		if (param)
		{
			void * vp = reinterpret_cast<void*>(param);
			QAndroidGooglePlayServiceLocationProvider * proxy = reinterpret_cast<QAndroidGooglePlayServiceLocationProvider*>(vp);
			QGeoPositionInfo posInfo = positionInfoFromJavaLocation(env, location);
			proxy->onLocationRecieved(posInfo, initial);
			return;
		}
		else
		{
			qWarning() << __FUNCTION__ << "Zero param!";
		}
	}
	catch (std::exception & e)
	{
		qWarning() << __FUNCTION__ << " exception: " << e.what();
	}
}


QAndroidGooglePlayServiceLocationProvider::QAndroidGooglePlayServiceLocationProvider(QObject * parent)
	: QObject(parent),
	reqiredInterval_(1500),
	minimumInterval_(1000)
{
	preloadJavaClasses();

	// Creating Java object
	handler_.reset(new QJniObject(c_full_class_name_, "J",
		jlong(reinterpret_cast<void*>(this))));
}

QAndroidGooglePlayServiceLocationProvider::~QAndroidGooglePlayServiceLocationProvider()
{
	if (handler_)
	{
		handler_->callVoid("cppDestroyed");
		handler_.reset();
	}
}


QGeoPositionInfo QAndroidGooglePlayServiceLocationProvider::lastKnownPosition() const
{
	QMutexLocker lock(&lastLocationSync_);
	return lastLocation_;
}


void QAndroidGooglePlayServiceLocationProvider::onStatusChanged(int status)
{
	emit statusChanged(status);
}


void QAndroidGooglePlayServiceLocationProvider::onLocationRecieved(const QGeoPositionInfo &location, jboolean initial)
{
	Q_UNUSED(initial)

	{
		QMutexLocker lock(&lastLocationSync_);
		lastLocation_ = location;
	}

	emit locationRecieved(location);
}


void QAndroidGooglePlayServiceLocationProvider::preloadJavaClasses()
{
	static volatile bool preloaded_ = false;

	if (!preloaded_)
	{
		try
		{
			preloaded_ = true;

			QAndroidQPAPluginGap::preloadJavaClasses();
			QAndroidQPAPluginGap::preloadJavaClass(c_full_class_name_);

			qDebug() << "Pre-loading Java classes for Google Play Services positioning...";
			QJniClass ov(c_full_class_name_);
			static const JNINativeMethod methods[] = {
				{"getActivity", "()Landroid/app/Activity;", (void*)QAndroidQPAPluginGap::getActivity},
				{"googleApiClientStatus", "(JI)V", (void*)Java_GooglePlayServiceLocationProvider_locationStatus},
				{"googleApiClientLocation", "(JLandroid/location/Location;Z)V", (void*)Java_GooglePlayServiceLocationProvider_locationRecieved},
			};

			if (!ov.registerNativeMethods(methods, sizeof(methods)))
			{
				qWarning() << "Failed to register native methods";
			}
		}
		catch(std::exception & e)
		{
			qWarning() << "Exception while registering native methods: " << e.what();
		}
	}
}


void QAndroidGooglePlayServiceLocationProvider::setUpdateInterval(int64_t reqiredInterval, int64_t minimumInterval)
{
	reqiredInterval_ = reqiredInterval;
	minimumInterval_ = minimumInterval;
}


void QAndroidGooglePlayServiceLocationProvider::startUpdates()
{
	if (handler_)
	{
		handler_->callVoid("requestGoogleApiClientLocationUpdatesStart", reqiredInterval_, minimumInterval_);
	}
}


void QAndroidGooglePlayServiceLocationProvider::stopUpdates()
{
	if (handler_)
	{
		handler_->callVoid("requestGoogleApiClientLocationUpdatesStop");
	}
}


bool QAndroidGooglePlayServiceLocationProvider::isAvailable()
{
	preloadJavaClasses();

	try 
	{
		qDebug() << "Checking for Google Play Services positioning availability...";
		QJniClass clazz(c_full_class_name_);
		if (!clazz.jClass())
		{
			qWarning() << "Failed to instantiate: " << c_full_class_name_;
			return false;
		}
		jboolean result = clazz.callStaticParamBoolean("isAvailable", "Landroid/app/Activity;", QAndroidQPAPluginGap::Context().jObject());
		qDebug() << "....GP positioning availability result:" << result;
		return result;
	}
	catch (const QJniBaseException &e)
	{
		qWarning() << "QJniBaseException catched! " << e.what();
	}

	return false;
}

