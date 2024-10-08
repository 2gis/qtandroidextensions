/*
	Offscreen Android Views library for Qt

	Author:
	  Vyacheslav O. Koscheev <vok1980@gmail.com>

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


#include "QGeoPositionInfoSourceAndroidNetwork.h"

#include <QtCore/qtcoreversion.h>
#include <QtPositioning/QGeoPositionInfo>

#include <QJniHelpers/TJniObjectLinker.h>
#include "PositionInfoConvertor.h"


#if QTCORE_VERSION < 0x050C00
	Q_DECLARE_METATYPE(QGeoPositionInfo)
#endif


static const char * const c_full_class_name_ = "ru/dublgis/androidlocation/NetworkLocationProvider";



Q_DECL_EXPORT void JNICALL Java_GeoPositionInfoSourceAndroidNetwork_onLocation(JNIEnv *, jobject, jlong param, jobject location)
{
	if (0x0 == location)
	{
		return;
	}

	JNI_LINKER_OBJECT(QGeoPositionInfoSourceAndroidNetwork, param, obj)
	QGeoPositionInfo posInfo = positionInfoFromJavaLocation(location);
	obj->onLocationRecieved(posInfo);
}


static const JNINativeMethod methods[] = {
	{"getContext", "()Landroid/content/Context;", reinterpret_cast<void*>(QAndroidQPAPluginGap::getCurrentContextNoThrow)},
	{"onLocationRecieved", "(JLandroid/location/Location;)V", reinterpret_cast<void*>(Java_GeoPositionInfoSourceAndroidNetwork_onLocation)},
};


/*!
	\class QGeoPositionInfoSourceAndroidNetwork
	\inheaderfile QGeoPositionInfoSourceAndroidNetwork.h
	\inmodule QtAndroidLocation
	\ingroup QtAndroidLocationGroup
	\ingroup GeoPositionInfoSources
	\brief Geo position info source for android Network provider
*/


JNI_LINKER_IMPL(QGeoPositionInfoSourceAndroidNetwork, c_full_class_name_, methods)


QGeoPositionInfoSourceAndroidNetwork::QGeoPositionInfoSourceAndroidNetwork(QObject * parent /*= 0*/)
	: QGeoPositionInfoSource(parent)
	, jniLinker_(new JniObjectLinker(this))
	, started_(false)
	, update_interval_msec_(1000)
{
	qRegisterMetaType<QGeoPositionInfo>();
	setPreferredPositioningMethods(QGeoPositionInfoSource::NonSatellitePositioningMethods);
}


QGeoPositionInfoSourceAndroidNetwork::~QGeoPositionInfoSourceAndroidNetwork()
{
}


void QGeoPositionInfoSourceAndroidNetwork::startUpdates()
{
	if (!started_ && isJniReady())
	{
		if (jni()->callParamBoolean("startLocationUpdates", "I", update_interval_msec_))
		{
			started_ = true;
		}
	}
}


void QGeoPositionInfoSourceAndroidNetwork::stopUpdates()
{
	if (isJniReady())
	{
		started_ = false;
		jni()->callVoid("stopLocationUpdates");
	}
}


void QGeoPositionInfoSourceAndroidNetwork::requestUpdate(int timeout /*= 0*/)
{
	Q_UNUSED(timeout);
}


void QGeoPositionInfoSourceAndroidNetwork::setUpdateInterval(int msec)
{
	update_interval_msec_ = msec;
}


QGeoPositionInfo QGeoPositionInfoSourceAndroidNetwork::lastKnownPosition(bool fromSatellitePositioningMethodsOnly /*= false*/) const
{
	QGeoPositionInfo info;

	try
	{
		if (isJniReady())
		{
			QJniObject jlocation(
				jni()->callParamObj(
					"lastKnownPosition",
					"Landroid/location/Location;",
					"Z",
					jboolean(fromSatellitePositioningMethodsOnly)));
			if (jlocation)
			{
				info = positionInfoFromJavaLocation(jlocation.jObject());
			}
		}
	}
	catch (const std::exception & e)
	{
		qWarning() << "Failed to get lastKnownPosition: " << e.what();
	}

	return info;
}


QGeoPositionInfoSource::PositioningMethods QGeoPositionInfoSourceAndroidNetwork::supportedPositioningMethods() const
{
	return QGeoPositionInfoSource::NoPositioningMethods;
}


void QGeoPositionInfoSourceAndroidNetwork::setPreferredPositioningMethods(const PositioningMethods methods)
{
	QGeoPositionInfoSource::setPreferredPositioningMethods(methods);
}


int QGeoPositionInfoSourceAndroidNetwork::minimumUpdateInterval() const
{
	return 0;
}


QGeoPositionInfoSource::Error QGeoPositionInfoSourceAndroidNetwork::error() const
{
	return QGeoPositionInfoSource::NoError;
}


void QGeoPositionInfoSourceAndroidNetwork::onLocationRecieved(QGeoPositionInfo location)
{
	emit positionUpdated(location);
}
