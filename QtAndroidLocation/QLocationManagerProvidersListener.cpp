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

#include "QLocationManagerProvidersListener.h"
#include <QJniHelpers/QAndroidQPAPluginGap.h>
#include <QJniHelpers/TJniObjectLinker.h>



static const char * const c_full_class_name_ = "ru/dublgis/androidlocation/LocationManagerProvidersListener";


Q_DECL_EXPORT void JNICALL Java_onProvidersChange(JNIEnv *, jobject, jlong param)
{
	JNI_LINKER_OBJECT(QLocationManagerProvidersListener, param, obj)
	obj->onProvidersChange();
}


static const JNINativeMethod methods[] = {
	{"getActivity", "()Landroid/app/Activity;", reinterpret_cast<void*>(QAndroidQPAPluginGap::getActivityNoThrow)},
	{"onProvidersChange", "(J)V", reinterpret_cast<void*>(Java_onProvidersChange)},
};


/*!
	\class QLocationManagerProvidersListener
	\inheaderfile QLocationManagerProvidersListener.h
	\inmodule QtAndroidLocation
	\ingroup QtAndroidLocationGroup
	\brief Network status listener
*/


JNI_LINKER_IMPL(QLocationManagerProvidersListener, c_full_class_name_, methods)


QLocationManagerProvidersListener::QLocationManagerProvidersListener(QObject * parent /*= 0*/) :
	QObject(parent)
	, jniLinker_(new JniObjectLinker(this))
{
}


QLocationManagerProvidersListener::~QLocationManagerProvidersListener()
{
}


QGeoPositionInfoSource::PositioningMethods QLocationManagerProvidersListener::getAvailableMethods()
{
	QGeoPositionInfoSource::PositioningMethods res = QGeoPositionInfoSource::NoPositioningMethods;

	if (isJniReady())
	{
		if (jni()->callBool("isGpsProviderAvailable"))
		{
			res |= QGeoPositionInfoSource::SatellitePositioningMethods;
		}

		if (jni()->callBool("isNetworkProviderAvailable"))
		{
			res |= QGeoPositionInfoSource::NonSatellitePositioningMethods;
		}
	}

	return res;
}


QGeoPositionInfoSource::PositioningMethods QLocationManagerProvidersListener::getActiveMethods()
{
	QGeoPositionInfoSource::PositioningMethods res = QGeoPositionInfoSource::NoPositioningMethods;

	if (isJniReady())
	{
		if (jni()->callBool("isGpsProviderEnabled"))
		{
			res |= QGeoPositionInfoSource::SatellitePositioningMethods;
		}

		if (jni()->callBool("isNetworkProviderEnabled"))
		{
			res |= QGeoPositionInfoSource::NonSatellitePositioningMethods;
		}
	}

	return res;
}


bool QLocationManagerProvidersListener::isActiveProvidersEnabled()
{
	bool ret = false;

	if (isJniReady())
	{
		ret = jni()->callBool("isActiveProvidersEnabled");
	}

	qDebug() << __FUNCTION__ << ": ret = " << ret;
	return ret;
}


void QLocationManagerProvidersListener::onProvidersChange()
{
	qDebug() << __FUNCTION__;
	emit providersChange(isActiveProvidersEnabled());
}
