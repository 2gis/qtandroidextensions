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
#include <cassert>
#include "QAndroidGmsLocationProvider.h"
#include <QAndroidQPAPluginGap.h>
#include <QtPositioning/QGeoPositionInfo>
#include <QtGui/QGuiApplication>
#include <TJniObjectLinker.h>
#include "PositionInfoConvertor.h"


static const char * const c_full_class_name_ = "ru/dublgis/androidlocation/GmsLocationProvider";




Q_DECL_EXPORT void JNICALL Java_GooglePlayServiceLocationProvider_locationStatus(JNIEnv *, jobject, jlong param, jint state)
{
	JNI_LINKER_OBJECT(QAndroidGmsLocationProvider, param, obj)
	obj->onStatusChanged(state);
}


Q_DECL_EXPORT void JNICALL Java_GooglePlayServiceLocationProvider_locationAvailable(JNIEnv *, jobject, jlong param, jboolean available)
{
	JNI_LINKER_OBJECT(QAndroidGmsLocationProvider, param, obj)
	obj->onLocationAvailable(available);
}


Q_DECL_EXPORT void JNICALL Java_GooglePlayServiceLocationProvider_locationRecieved(JNIEnv *, jobject, jlong param, jobject location, jboolean initial, jlong requestId)
{
	if (0x0 == location)
	{
		return;
	}

	JNI_LINKER_OBJECT(QAndroidGmsLocationProvider, param, obj)
	QGeoPositionInfo posInfo = positionInfoFromJavaLocation(location);
	obj->onLocationRecieved(posInfo, initial, requestId);
}


static const JNINativeMethod methods[] = {
	{"getActivity", "()Landroid/app/Activity;", reinterpret_cast<void*>(QAndroidQPAPluginGap::getActivityNoThrow)},
	{"googleApiClientStatus", "(JI)V", reinterpret_cast<void*>(Java_GooglePlayServiceLocationProvider_locationStatus)},
	{"googleApiClientLocationAvailable", "(JZ)V", reinterpret_cast<void*>(Java_GooglePlayServiceLocationProvider_locationAvailable)},
	{"googleApiClientLocation", "(JLandroid/location/Location;ZJ)V", reinterpret_cast<void*>(Java_GooglePlayServiceLocationProvider_locationRecieved)},
};


/*!
	\class QAndroidGmsLocationProvider
	\inheaderfile QAndroidGmsLocationProvider.h
	\inmodule QtAndroidLocation
	\ingroup QtAndroidLocationGroup
	\brief Data provider for geo position info source for android GMS

	\sa QGeoPositionInfoSourceAndroidGms
*/


JNI_LINKER_IMPL(QAndroidGmsLocationProvider, c_full_class_name_, methods)


QAndroidGmsLocationProvider::QAndroidGmsLocationProvider(QObject * parent)
	: QObject(parent)
	, jniLinker_(new JniObjectLinker(this))
	, reqiredInterval_(1500)
	, minimumInterval_(1000)
	, priority_(PRIORITY_NO_POWER)
	, regularUpdadesId_(0)
{
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
	if (isJniReady())
	{
		jni()->callVoid("lastKnownPosition");
	}

	QMutexLocker lock(&lastLocationSync_);
	return lastLocation_;
}


void QAndroidGmsLocationProvider::onLocationAvailable(jboolean available)
{
	emit locationAvailable(available);
}


void QAndroidGmsLocationProvider::onStatusChanged(jint status)
{
	const char * szStatus = "Unknown";

	switch (status)
	{
		case S_DISCONNECTED:
			szStatus = "Disconnected";
			break;
		case S_CONNECTED:
			szStatus = "Connected";
			break;
		case S_CONNECT_ERROR:
			szStatus = "Error";
			break;
		case S_CONNECT_SUSPEND:
			szStatus = "Suspended";
			break;
		default:
			assert(!"Unexpected status");
	}

	qDebug() << __FUNCTION__ << ": status = " << status << " (" << szStatus << ")";
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


void QAndroidGmsLocationProvider::setUpdateInterval(int reqiredInterval, int minimumInterval)
{
	qInfo() << __FUNCTION__ << ": reqiredInterval" << reqiredInterval << ", minimumInterval" << minimumInterval;
	reqiredInterval_ = reqiredInterval;
	minimumInterval_ = minimumInterval;
}


void QAndroidGmsLocationProvider::startUpdates()
{
	qDebug() << __FUNCTION__;
	stopUpdates();

	if (isJniReady())
	{
		jlong maxWaitTime = reqiredInterval_ * 3 / 2;
		jint numUpdates = 0;
		jlong expirationDuration = 0;
		jlong expirationTime = 0;

		jlong id = jni()->callParamLong("startLocationUpdates", "IJJJIJJ",
		                                   jint(priority_),
		                                   jlong(reqiredInterval_),
		                                   jlong(minimumInterval_),
		                                   maxWaitTime,
		                                   numUpdates,
		                                   expirationDuration,
		                                   expirationTime);

		{
			QMutexLocker lock(&lastLocationSync_);
			regularUpdadesId_ = id;
		}

		qDebug() << __FUNCTION__ << ": updates id =" << id;
	}
}


void QAndroidGmsLocationProvider::stopUpdates()
{
	RequestsColl requests;

	{
		QMutexLocker lock(&lastLocationSync_);

		requests.push_back(regularUpdadesId_);
		requests.insert(requests.end(), requestUpdadesIds_.cbegin(), requestUpdadesIds_.cend());

		regularUpdadesId_ = 0;
		requestUpdadesIds_.clear();
	}

	for (RequestsColl::const_iterator it = requests.cbegin(); it != requests.cend(); ++it)
	{
		stopUpdates(*it);
	}
}


void QAndroidGmsLocationProvider::stopUpdates(jlong requestId)
{
	qDebug() << __FUNCTION__ << "(" << requestId << ")";

	if (isJniReady())
	{
		jni()->callParamVoid("stopLocationUpdates", "J", requestId);
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


void QAndroidGmsLocationProvider::onCheckRequest(jlong requestId)
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


void QAndroidGmsLocationProvider::requestUpdate(int timeout /*= 0*/)
{
	qDebug() << __FUNCTION__;

	if (0 == timeout)
	{
		timeout = std::max(1000, 2 * reqiredInterval_);
	}

	if (isJniReady())
	{
		jlong maxWaitTime = 0;
		jint numUpdates = 1;
		jlong expirationDuration = timeout;
		jlong expirationTime = 0;

		jlong id = jni()->callParamLong("startLocationUpdates", "IJJJIJJ",
		                    jint(priority_),
		                    jlong(reqiredInterval_),
		                    jlong(minimumInterval_),
		                    maxWaitTime,
		                    numUpdates,
		                    expirationDuration,
		                    expirationTime);

		{
			QMutexLocker lock(&lastLocationSync_);
			requestUpdadesIds_.push_back(id);
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
	catch (const std::exception & e)
	{
		qWarning() << "JNI exception in QAndroidGmsLocationProvider::getGmsVersion: " << e.what();
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
	catch (const std::exception & e)
	{
		qWarning() << "JNI exception in QAndroidGmsLocationProvider::isAvailable: " << e.what();
	}

	return false;
}

