/*
	Offscreen Android Views library for Qt

	Author:
	  Vyacheslav O. Koscheev <vok1980@gmail.com>

	Distrbuted under The BSD License

	Copyright (c) 2017, DoubleGIS, LLC.
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


#include "QGeoPositionInfoSourceAndroidPassive.h"

#include <QtPositioning/QGeoPositionInfo>

#include <QJniHelpers/TJniObjectLinker.h>
#include "PositionInfoConvertor.h"


Q_DECLARE_METATYPE(QGeoPositionInfo)


static const char * const c_full_class_name_ = "ru/dublgis/androidlocation/PassiveLocationProvider";



Q_DECL_EXPORT void JNICALL Java_GeoPositionInfoSourceAndroidPassive_onLocation(JNIEnv *, jobject, jlong param, jobject location)
{
	if (0x0 == location)
	{
		return;
	}

	JNI_LINKER_OBJECT(QGeoPositionInfoSourceAndroidPassive, param, obj)
	QGeoPositionInfo posInfo = positionInfoFromJavaLocation(location);
	obj->onLocationRecieved(posInfo);
}


static const JNINativeMethod methods[] = {
	{"getActivity", "()Landroid/app/Activity;", reinterpret_cast<void*>(QAndroidQPAPluginGap::getActivityNoThrow)},
	{"onLocationRecieved", "(JLandroid/location/Location;)V", reinterpret_cast<void*>(Java_GeoPositionInfoSourceAndroidPassive_onLocation)},
};


/*!
	\class QGeoPositionInfoSourceAndroidPassive
	\inheaderfile QGeoPositionInfoSourceAndroidPassive.h
	\inmodule QtAndroidLocation
	\ingroup QtAndroidLocationGroup
	\ingroup GeoPositionInfoSources
	\brief Geo position info source for android passive provider
*/


JNI_LINKER_IMPL(QGeoPositionInfoSourceAndroidPassive, c_full_class_name_, methods)


QGeoPositionInfoSourceAndroidPassive::QGeoPositionInfoSourceAndroidPassive(QObject * parent /*= 0*/)
	: QGeoPositionInfoSource(parent)
	, jniLinker_(new JniObjectLinker(this))
	, started_(false)
	, update_interval_msec_(5 * 60 * 1000)
{
	qRegisterMetaType<QGeoPositionInfo>();
	setPreferredPositioningMethods(QGeoPositionInfoSource::NonSatellitePositioningMethods);
}


QGeoPositionInfoSourceAndroidPassive::~QGeoPositionInfoSourceAndroidPassive()
{
}


void QGeoPositionInfoSourceAndroidPassive::startUpdates()
{
	if (!started_ && isJniReady())
	{
		if (jni()->callParamBoolean("startLocationUpdates", "I", update_interval_msec_))
		{
			started_ = true;
		}
	}
}


void QGeoPositionInfoSourceAndroidPassive::stopUpdates()
{
	if (isJniReady())
	{
		started_ = false;
		jni()->callVoid("stopLocationUpdates");
	}
}


void QGeoPositionInfoSourceAndroidPassive::requestUpdate(int timeout /*= 0*/)
{
	Q_UNUSED(timeout);
}


void QGeoPositionInfoSourceAndroidPassive::setUpdateInterval(int msec)
{
	update_interval_msec_ = msec;
}


QGeoPositionInfo QGeoPositionInfoSourceAndroidPassive::lastKnownPosition(bool fromSatellitePositioningMethodsOnly /*= false*/) const
{
	QGeoPositionInfo info;

	try
	{
		if (isJniReady())
		{
			QScopedPointer<QJniObject> jlocation(
				jni()->callStaticParamObject(
					"lastKnownPosition",
					"Landroid/location/Location;",
					"Z",
					jboolean(fromSatellitePositioningMethodsOnly)));

			if (jlocation && jlocation->jObject())
			{
				info = positionInfoFromJavaLocation(jlocation->jObject());
			}
		}
	}
	catch (const std::exception & e)
	{
		qWarning() << "Failed to get lastKnownPosition: " << e.what();
	}

	return info;
}


QGeoPositionInfoSource::PositioningMethods QGeoPositionInfoSourceAndroidPassive::supportedPositioningMethods() const
{
	return QGeoPositionInfoSource::NoPositioningMethods;
}


void QGeoPositionInfoSourceAndroidPassive::setPreferredPositioningMethods(const PositioningMethods methods)
{
	QGeoPositionInfoSource::setPreferredPositioningMethods(methods);
}


int QGeoPositionInfoSourceAndroidPassive::minimumUpdateInterval() const
{
	return 0;
}


QGeoPositionInfoSource::Error QGeoPositionInfoSourceAndroidPassive::error() const
{
	return QGeoPositionInfoSource::NoError;
}


void QGeoPositionInfoSourceAndroidPassive::onLocationRecieved(QGeoPositionInfo location)
{
	emit positionUpdated(location);
}
