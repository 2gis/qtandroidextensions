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
#include <QAndroidQPAPluginGap.h>



static const char * const c_full_class_name_ = "ru/dublgis/androidgpslocation/LocationManagerProvidersListener";


Q_DECL_EXPORT void JNICALL Java_onProvidersChange(JNIEnv * env, jobject, jlong param)
{
	try
	{
		if (param)
		{
			void * vp = reinterpret_cast<void*>(param);
			QLocationManagerProvidersListener * proxy = reinterpret_cast<QLocationManagerProvidersListener*>(vp);
			proxy->onProvidersChange();
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


QLocationManagerProvidersListener::QLocationManagerProvidersListener(QObject * parent /*= 0*/) :
	QObject(parent)
{
	preloadJavaClasses();

	// Creating Java object
	handler_.reset(new QJniObject(c_full_class_name_, "J",
		jlong(reinterpret_cast<void*>(this))));
}


QLocationManagerProvidersListener::~QLocationManagerProvidersListener()
{
	if (handler_)
	{
		handler_->callVoid("cppDestroyed");
		handler_.reset();
	}
}


QGeoPositionInfoSource::PositioningMethods QLocationManagerProvidersListener::getActiveMethods()
{
	QGeoPositionInfoSource::PositioningMethods res = QGeoPositionInfoSource::NoPositioningMethods;

	if (handler_)
	{
		if (handler_->callBool("IsGpsProviderEnabled"))
		{
			res |= QGeoPositionInfoSource::SatellitePositioningMethods;
		}

		if (handler_->callBool("IsNetworkProviderEnabled"))
		{
			res |= QGeoPositionInfoSource::NonSatellitePositioningMethods;
		}
	}

	return res;
}


bool QLocationManagerProvidersListener::IsActiveProvidersEnabled()
{
	bool ret = false;

	if (handler_)
	{
		ret = handler_->callBool("IsActiveProvidersEnabled");
	}

	qDebug() << __FUNCTION__ << ": ret = " << ret;
	return ret;
}


void QLocationManagerProvidersListener::onProvidersChange()
{
	qDebug() << __FUNCTION__;
	emit providersChange(IsActiveProvidersEnabled());
}


void QLocationManagerProvidersListener::preloadJavaClasses()
{
	static volatile bool preloaded_ = false;

	if (!preloaded_)
	{
		try
		{
			preloaded_ = true;

			QAndroidQPAPluginGap::preloadJavaClasses();
			QAndroidQPAPluginGap::preloadJavaClass(c_full_class_name_);

			qDebug() << "Pre-loading Java classes for QLocationManagerProvidersListener";
			QJniClass ov(c_full_class_name_);
			static const JNINativeMethod methods[] = {
				{"getActivity", "()Landroid/app/Activity;", (void*)QAndroidQPAPluginGap::getActivity},
				{"onProvidersChange", "(J)V", (void*)Java_onProvidersChange},
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
