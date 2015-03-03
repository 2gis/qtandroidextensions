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

#include <QJniHelpers.h>
#include <QAndroidQPAPluginGap.h>
#include "QAndroidDisplayMetrics.h"

QAndroidDisplayMetrics::QAndroidDisplayMetrics(QObject * parent)
	: QObject(parent)
	, density_(1.0f)
	, densityDpi_(160)
	, scaledDensity_(1.0f)
	, densityFromDpi_(1.0f)
	, scaledDensityFromDpi_(1.0f)
	, xdpi_(160.0f)
	, ydpi_(160.0f)
	, realisticDpi_(160.0f)
	, widthPixels_(240)
	, heightPixels_(240)
	, theme_(ThemeMDPI)
{
	QJniObject metrics("android/util/DisplayMetrics", "");
	{
		QAndroidQPAPluginGap::Context activity;
		QScopedPointer<QJniObject> windowmanager(activity.callObject("getWindowManager", "android/view/WindowManager"));
		QScopedPointer<QJniObject> defaultdisplay(windowmanager->callObject("getDefaultDisplay", "android/view/Display"));
		defaultdisplay->callParamVoid("getMetrics", "Landroid/util/DisplayMetrics;", metrics.jObject());
	}

	density_ = metrics.getFloatField("density");
	densityDpi_ = metrics.getIntField("densityDpi");
	heightPixels_ = metrics.getIntField("heightPixels");
	scaledDensity_ = metrics.getFloatField("scaledDensity");
	xdpi_ = metrics.getFloatField("xdpi");
	ydpi_ = metrics.getFloatField("ydpi");
	widthPixels_ = metrics.getIntField("widthPixels");
	heightPixels_ = metrics.getIntField("heightPixels");

	//
	// Calculating theme
	//
	if (densityDpi_ < (ANDROID_DENSITY_LOW+ANDROID_DENSITY_MEDIUM)/2)
		theme_ = ThemeLDPI;
	else if(densityDpi_ < (ANDROID_DENSITY_MEDIUM+ANDROID_DENSITY_TV)/2)
		theme_ = ThemeMDPI;
	else if(densityDpi_ < (ANDROID_DENSITY_TV+ANDROID_DENSITY_HIGH)/2)
		theme_ = ThemeTVDPI;
	else if(densityDpi_ < (ANDROID_DENSITY_HIGH+ANDROID_DENSITY_XHIGH)/2)
		theme_ = ThemeHDPI;
	else if(densityDpi_ < (ANDROID_DENSITY_XHIGH+ANDROID_DENSITY_400)/2)
		theme_ = ThemeXHDPI;
	else if(densityDpi_ < (ANDROID_DENSITY_400+ANDROID_DENSITY_XXHIGH)/2)
		theme_ = Theme400DPI;
	else if(densityDpi_ < (ANDROID_DENSITY_XXHIGH+ANDROID_DENSITY_XXXHIGH)/2)
		theme_ = ThemeXXDPI;
	else
		theme_ = ThemeXXXDPI;

	//
	// Calculating scaler from the theme
	//
	switch(theme_)
	{
	case ThemeLDPI:		densityFromDpi_ = 0.75f;	break;
	case ThemeMDPI:		densityFromDpi_ = 1.00f;	break;
	case ThemeTVDPI:	densityFromDpi_ = 1.33f;	break;
	case ThemeHDPI:		densityFromDpi_ = 1.50f;	break;
	case ThemeXHDPI:	densityFromDpi_ = 2.00f;	break;
	case Theme400DPI:	densityFromDpi_ = 2.50f;	break; //! \fixme Unverified value!
	case ThemeXXDPI:	densityFromDpi_ = 3.00f;	break;
	case ThemeXXXDPI:	densityFromDpi_ = 4.00f;	break;
	default:
		Q_ASSERT(!"Theme value not listed!");
		densityFromDpi_ = 1.0f;
		break;
	}

	if (density_ > 0.0f)
	{
		scaledDensityFromDpi_ = densityFromDpi_ * (scaledDensity_ / density_);
	}
	else
	{
		scaledDensityFromDpi_ = densityFromDpi_;
	}

	realisticDpi_ = (xdpi_ + ydpi_) / 2.0f;
	if (densityDpi_ > 0.0f)
	{
		float difference = realisticDpi_ / float(densityDpi_);
		if (difference > 1.0f)
			difference = 1.0f / difference;
		if (difference < 0.75f)
		{
			qWarning()<<"Average hardware DPI is reported as"<<realisticDpi_<<"but physical DPI is"
					  <<densityDpi_<<"(too different).";
			realisticDpi_ = float(densityDpi_);
		}
	}

	qDebug()<<"QAndroidDisplayMetrics: density ="<<density()<<"/ densityDpi ="<<densityDpi()
			<<"/ scaledDensity ="<<scaledDensity()
			<<"/ xdpi ="<<xdpi()<<"/ ydpi ="<<ydpi()
			<<"/ realisticDpi ="<<realisticDpi_
			<<"/ widthPixels ="<<widthPixels()<<"/ heightPixels ="<<heightPixels()
			<<"/ Theme ="<<int(theme_)<<themeDirectoryName()
			<<"/ densityFromDpi ="<<densityFromDpi_
			<<"/ scaledDensityFromDpi ="<<scaledDensityFromDpi_;
}

void QAndroidDisplayMetrics::preloadJavaClasses()
{
	QAndroidQPAPluginGap::preloadJavaClasses();
	QAndroidQPAPluginGap::preloadJavaClass("android/util/DisplayMetrics");
}

QString QAndroidDisplayMetrics::themeDirectoryName(Theme theme)
{
	switch(theme)
	{
	case ThemeLDPI:		return QLatin1String("ldpi");
	case ThemeMDPI:		return QLatin1String("mdpi");
	case ThemeTVDPI:	return QLatin1String("tvdpi");
	case ThemeHDPI:		return QLatin1String("hdpi");
	case ThemeXHDPI:	return QLatin1String("xhdpi");
	case Theme400DPI:	return QLatin1String("400dpi");
	case ThemeXXDPI:	return QLatin1String("xxdpi");
	case ThemeXXXDPI:	return QLatin1String("xxxdpi");
	default:
		qWarning()<<"Unknown theme value:"<<theme;
		return QString::null;
	};
}

