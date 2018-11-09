/*
  Lightweight access to various Android APIs for Qt

  Author:
  Sergey A. Galin <sergey.galin@gmail.com>

  Distrbuted under The BSD License

  Copyright (c) 2014, DoubleGIS, LLC.
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

#include "QAndroidConfiguration.h"
#include <QJniHelpers/QJniHelpers.h>
#include <QJniHelpers/QAndroidQPAPluginGap.h>


QAndroidConfiguration::QAndroidConfiguration(QObject * parent)
	: QObject(parent)
	, screen_size_(ScreenSizeUndefined)
{
	QAndroidQPAPluginGap::Context activity;
	QScopedPointer<QJniObject> resources(activity.callObject("getResources", "android/content/res/Resources"));
	QScopedPointer<QJniObject> configuration(resources->callObject("getConfiguration", "android/content/res/Configuration"));

	int screenLayout = configuration->getIntField("screenLayout") & ANDROID_SCREENLAYOUT_SIZE_MASK;

	switch(screenLayout)
	{
	case ANDROID_SCREENLAYOUT_SIZE_SMALL:
		screen_size_ = ScreenSizeSmall;
		break;
	case ANDROID_SCREENLAYOUT_SIZE_NORMAL:
		screen_size_ = ScreenSizeNormal;
		break;
	case ANDROID_SCREENLAYOUT_SIZE_LARGE:
		screen_size_ = ScreenSizeLarge;
		break;
	case ANDROID_SCREENLAYOUT_SIZE_XLARGE:
		screen_size_ = ScreenSizeXLarge;
		break;
	case ANDROID_SCREENLAYOUT_SIZE_UNDEFINED:
		screen_size_ = ScreenSizeUndefined;
		break;
	default:
		screen_size_ = ScreenSizeXLarge;
	}

	//! \todo Add reading of more fields here.

	qDebug()<<"QAndroidConfiguration: screen layout is"<<screenLayout<<"=>"<<screen_size_<<"("<<screenSizeName()<<")"
			<<", a tablet size:"<<isTablet();
}

void QAndroidConfiguration::preloadJavaClasses()
{
	QAndroidQPAPluginGap::preloadJavaClasses();
	// QAndroidQPAPluginGap::preloadJavaClass("android/util/DisplayMetrics");
}

QString QAndroidConfiguration::screenSizeName(ScreenSize size)
{
	switch(size)
	{
	case ScreenSizeSmall:	return QLatin1String("small");
	case ScreenSizeNormal:	return QLatin1String("normal");
	case ScreenSizeLarge:	return QLatin1String("large");
	case ScreenSizeXLarge:	return QLatin1String("xlarge");
	default:
		qDebug()<<"Not screen size name for size"<<int(size);
		return QLatin1String("normal");
	}
}

