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

#include <algorithm>
#include "QAndroidGmsLocationProvider.h"
#include <QAndroidQPAPluginGap.h>
#include <QtPositioning/QGeoPositionInfo>
#include <QtGui/QGuiApplication>
#include <TJniObjectLinker.h>


static const char * const c_full_class_name_ = "ru/dublgis/androidlocation/GmsLocationProvider";


static void setPositionAttributeFloat(QGeoPositionInfo & info, QGeoPositionInfo::Attribute attr,
                                      QJniObject & location, const char * szCheck, const char * szGet)
{
	if (location.callBool(szCheck))
	{
		jfloat val = location.callFloat(szGet);
		info.setAttribute(attr, val);
	}
}


static QGeoPositionInfo positionInfoFromJavaLocation(JNIEnv * jniEnv, const jobject & jlocation)
{
	QGeoPositionInfo info;
	QJniObject location(jlocation, false);

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
	JNI_LINKER_OBJECT(QAndroidGmsLocationProvider, param, obj)

	if (obj)
	{
		obj->onStatusChanged(state);
	}
	else
	{
		qWarning() << __FUNCTION__ << "Zero param!";
	}
}


Q_DECL_EXPORT void JNICALL Java_GooglePlayServiceLocationProvider_locationRecieved(JNIEnv * env, jobject, jlong param, jobject location, jboolean initial, jlong requestId)
{
	if (0x0 == location)
	{
		return;
	}

	JNI_LINKER_OBJECT(QAndroidGmsLocationProvider, param, obj)

	if (obj)
	{
		QGeoPositionInfo posInfo = positionInfoFromJavaLocation(env, location);
		obj->onLocationRecieved(posInfo, initial, requestId);
	}
	else
	{
		qWarning() << __FUNCTION__ << "Zero param!";
	}
}


static const JNINativeMethod methods[] = {
	{"getActivity", "()Landroid/app/Activity;", (void*)QAndroidQPAPluginGap::getActivity},
	{"googleApiClientStatus", "(JI)V", (void*)Java_GooglePlayServiceLocationProvider_locationStatus},
	{"googleApiClientLocation", "(JLandroid/location/Location;ZJ)V", (void*)Java_GooglePlayServiceLocationProvider_locationRecieved},
};


JNI_LINKER_IMPL(QAndroidGmsLocationProvider, c_full_class_name_, methods)


QAndroidGmsLocationProvider::QAndroidGmsLocationProvider(QObject * parent)
	: QObject(parent)
	, jniLinker_(new JniObjectLinker(this))
	, reqiredInterval_(1500)
	, minimumInterval_(1000)
	, priority_(PRIORITY_NO_POWER)
	, regularUpdadesId_(0)
	, requestUpdadesId_(0)
	, requestTimer_(this) // should be set due to parent's move to moveToThread operation
{
	QObject::connect(&requestTimer_, &QTimer::timeout,
	                 this, &QAndroidGmsLocationProvider::onRequestTimeout);

	QObject::connect(qApp, &QGuiApplication::applicationStateChanged,
	                 this, &QAndroidGmsLocationProvider::onApplicationStateChanged);

	QObject::connect(this, &QAndroidGmsLocationProvider::checkRequest,
	                 this, &QAndroidGmsLocationProvider::onCheckRequest);

	onApplicationStateChanged(QGuiApplication::applicationState());
}


QAndroidGmsLocationProvider::~QAndroidGmsLocationProvider()
{
}


QGeoPositionInfo QAndroidGmsLocationProvider::lastKnownPosition() const
{
	QMutexLocker lock(&lastLocationSync_);
	return lastLocation_;
}


void QAndroidGmsLocationProvider::onStatusChanged(int status)
{
	qDebug() << __FUNCTION__ << ": status = " << status;
	emit statusChanged(status);
}


void QAndroidGmsLocationProvider::onLocationRecieved(const QGeoPositionInfo &location, jboolean initial, jlong requestId)
{
	{
		QMutexLocker lock(&lastLocationSync_);
		lastLocation_ = location;
	}

	emit checkRequest(requestId);

	if (!initial)
	{
		emit locationRecieved(location);
	}
}


void QAndroidGmsLocationProvider::setPriority(enPriority priority)
{
	priority_ = priority;
}


void QAndroidGmsLocationProvider::setUpdateInterval(int64_t reqiredInterval, int64_t minimumInterval)
{
	reqiredInterval_ = reqiredInterval;
	minimumInterval_ = minimumInterval;
}


void QAndroidGmsLocationProvider::startUpdates()
{
	qDebug() << __FUNCTION__;
	stopUpdates();

	if (isJniReady())
	{
		jlong maxWaitTime = 0;
		jint numUpdates = 0;
		jlong expirationDuration = 0;
		jlong expirationTime = 0;

		jlong id = jni()->callParamLong("startLocationUpdates", "IJJJIJJ",
		                                   (jint)priority_,
		                                   reqiredInterval_,
		                                   minimumInterval_,
		                                   maxWaitTime,
		                                   numUpdates,
		                                   expirationDuration,
		                                   expirationTime);

		{
			QMutexLocker lock(&lastLocationSync_);
			regularUpdadesId_ = id;
		}

		qDebug() << "updates id =" << id;
	}
}


void QAndroidGmsLocationProvider::stopUpdates()
{
	jlong id = 0;

	{
		QMutexLocker lock(&lastLocationSync_);
		id = regularUpdadesId_;
		regularUpdadesId_ = 0;
	}

	stopUpdates(id);
}


void QAndroidGmsLocationProvider::stopUpdates(jlong requestId)
{
	qDebug() << __FUNCTION__ << "(" << requestId << ")";

	if (isJniReady())
	{
		jni()->callParamVoid("stopLocationUpdates", "J", requestId);

		bool bStopTimer = false;

		{
			QMutexLocker lock(&lastLocationSync_);
			bStopTimer = (requestId == requestUpdadesId_);
		}

		if (bStopTimer)
		{
			requestTimer_.stop();
		}
	}
}


void QAndroidGmsLocationProvider::onApplicationStateChanged(Qt::ApplicationState state)
{
	if (isJniReady())
	{
		jboolean enable = (Qt::ApplicationActive == state);

		switch (state)
		{
			case Qt::ApplicationSuspended:	//onStop
			case Qt::ApplicationActive:		//onStart
				jni()->callParamVoid("activate", "Z", enable);
				break;
			default:
				// do nothing
				break;
		}
	}
}


void QAndroidGmsLocationProvider::onCheckRequest(long requestId)
{
	bool stop = false;

	{
		QMutexLocker lock(&lastLocationSync_);
		stop = requestId != regularUpdadesId_;
	}

	if (stop)
	{
		QAndroidGmsLocationProvider::stopUpdates(requestId);		
	}
}


void QAndroidGmsLocationProvider::onRequestTimeout()
{
	jlong id = 0;

	{
		QMutexLocker lock(&lastLocationSync_);
		id = requestUpdadesId_;
		requestUpdadesId_ = 0;
	}

	stopUpdates(id);
}


void QAndroidGmsLocationProvider::requestUpdate(int timeout /*= 0*/)
{
	qDebug() << __FUNCTION__;

	if (requestTimer_.isActive() || timeout < 0)
	{
		return;
	}

	if (0 == timeout)
	{
		timeout = std::max((jlong)1000, 2 * reqiredInterval_);
	}

	requestTimer_.setSingleShot(true);
	requestTimer_.start(timeout);

	if (isJniReady())
	{
		jlong maxWaitTime = 0;
		jint numUpdates = 1;
		jlong expirationDuration = timeout;
		jlong expirationTime = 0;

		jlong id = jni()->callParamLong("startLocationUpdates", "IJJJIJJ",
		                    (jint)priority_,
		                    reqiredInterval_,
		                    minimumInterval_,
		                    maxWaitTime,
		                    numUpdates,
		                    expirationDuration,
		                    expirationTime);

		{
			QMutexLocker lock(&lastLocationSync_);
			requestUpdadesId_ = id;
		}

		qDebug() << "request id =" << id;
	}
}


int QAndroidGmsLocationProvider::getGmsVersion()
{
	preloadJavaClasses();

	try
	{
		QJniClass clazz(c_full_class_name_);

		if (!clazz.jClass())
		{
			qWarning() << "Failed to instantiate: " << c_full_class_name_;
			return 0;
		}

		return clazz.callStaticParamInt("getGmsVersion", "Landroid/app/Activity;", QAndroidQPAPluginGap::Context().jObject());
	}
	catch (const QJniBaseException & e)
	{
		qWarning() << "QJniBaseException catched! " << e.what();
	}

	return 0;
}


bool QAndroidGmsLocationProvider::isAvailable(jboolean allowDialog)
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
		jboolean result = clazz.callStaticParamBoolean("isAvailable", "Landroid/app/Activity;Z", QAndroidQPAPluginGap::Context().jObject(), allowDialog);
		qDebug() << "....GP positioning availability result:" << result;
		return result;
	}
	catch (const QJniBaseException & e)
	{
		qWarning() << "QJniBaseException catched! " << e.what();
	}

	return false;
}

