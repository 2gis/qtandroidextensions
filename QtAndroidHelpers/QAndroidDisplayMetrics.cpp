/*
  Lightweight access to various Android APIs for Qt

  Author:
  Sergey A. Galin <sergey.galin@gmail.com>

  Distrbuted under The BSD License

  Copyright (c) 2014-2016, DoubleGIS, LLC.
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


namespace {

struct ThemeListEntry
{
	QAndroidDisplayMetrics::Theme theme;
	float density;
	QAndroidDisplayMetrics::IntermediateDensities availability;
	int starting_ppi;
};

} // anonymous namespace

static const ThemeListEntry all_themes[] =
{
	{ QAndroidDisplayMetrics::ThemeLDPI,   0.75f, QAndroidDisplayMetrics::IntermediateNone,        QAndroidDisplayMetrics::ANDROID_DENSITY_LOW },
	{ QAndroidDisplayMetrics::ThemeMDPI,   1.00f, QAndroidDisplayMetrics::IntermediateNone,        QAndroidDisplayMetrics::ANDROID_DENSITY_MEDIUM },
	{ QAndroidDisplayMetrics::ThemeTVDPI,  1.33f, QAndroidDisplayMetrics::IntermediateAll,         QAndroidDisplayMetrics::ANDROID_DENSITY_TV },
	{ QAndroidDisplayMetrics::ThemeHDPI,   1.50f, QAndroidDisplayMetrics::IntermediateNone,        QAndroidDisplayMetrics::ANDROID_DENSITY_HIGH },
	{ QAndroidDisplayMetrics::Theme280DPI, 1.67f, QAndroidDisplayMetrics::IntermediateAll,         QAndroidDisplayMetrics::ANDROID_DENSITY_280 },
	{ QAndroidDisplayMetrics::ThemeXHDPI,  2.00f, QAndroidDisplayMetrics::IntermediateNone,        QAndroidDisplayMetrics::ANDROID_DENSITY_XHIGH },
	{ QAndroidDisplayMetrics::Theme360DPI, 2.33f, QAndroidDisplayMetrics::IntermediateAll,         QAndroidDisplayMetrics::ANDROID_DENSITY_360 },
	{ QAndroidDisplayMetrics::Theme400DPI, 2.50f, QAndroidDisplayMetrics::IntermediateWithStep0_5, QAndroidDisplayMetrics::ANDROID_DENSITY_400 },
	{ QAndroidDisplayMetrics::Theme420DPI, 2.67f, QAndroidDisplayMetrics::IntermediateAll,         QAndroidDisplayMetrics::ANDROID_DENSITY_420 },
	{ QAndroidDisplayMetrics::ThemeXXDPI,  3.00f, QAndroidDisplayMetrics::IntermediateNone,        QAndroidDisplayMetrics::ANDROID_DENSITY_XXHIGH },
	{ QAndroidDisplayMetrics::Theme560DPI, 3.50f, QAndroidDisplayMetrics::IntermediateWithStep0_5, QAndroidDisplayMetrics::ANDROID_DENSITY_560 },
	{ QAndroidDisplayMetrics::ThemeXXXDPI, 4.00f, QAndroidDisplayMetrics::IntermediateNone,        QAndroidDisplayMetrics::ANDROID_DENSITY_XXXHIGH }
};

static const size_t all_themes_count = sizeof(all_themes) / sizeof(all_themes[0]);


static float densityFromTheme(QAndroidDisplayMetrics::Theme theme)
{
	for (size_t i = 0; i < all_themes_count; ++i)
	{
		if (all_themes[i].theme == theme)
		{
			return all_themes[i].density;
		}
	}
	qWarning() << "Theme code is not listed in the table:" << static_cast<int>(theme);
	return 1.0f;
}


static QAndroidDisplayMetrics::Theme themeFromDensity(int density_dpi, QAndroidDisplayMetrics::IntermediateDensities intermediate_densities)
{
	QAndroidDisplayMetrics::Theme resulting_theme = all_themes[0].theme;
	for (size_t i = 0; i < all_themes_count; ++i)
	{
		// Don't see unallowed themes
		if (all_themes[i].availability > intermediate_densities)
		{
			continue;
		}
		if (density_dpi >= all_themes[i].starting_ppi)
		{
			resulting_theme = all_themes[i].theme;
		}
		else // density_dpi < entry->starting_ppi
		{
			// Since all the next entries have bigger starting_ppi, we don't have to look further.
			break;
		}
	}
	return resulting_theme;
}


QAndroidDisplayMetrics::QAndroidDisplayMetrics(
		QObject * parent
		, QAndroidDisplayMetrics::IntermediateDensities intermediate_densities)
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

	// Calculating theme
	theme_ = themeFromDensity(densityDpi_, intermediate_densities);

	// Calculating scaler from the theme
	densityFromDpi_ = densityFromTheme(theme_);

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
		{
			difference = 1.0f / difference;
		}
		if (difference < 0.75f)
		{
			qWarning() << "Average hardware DPI is reported as" << realisticDpi_ << "but physical DPI is"
				<< densityDpi_ << "(too different).";
			realisticDpi_ = float(densityDpi_);
		}
	}

	qDebug() << "QAndroidDisplayMetrics: density =" << density() << "/ densityDpi =" << densityDpi()
		<< "/ scaledDensity =" << scaledDensity()
		<< "/ xdpi =" << xdpi() << "/ ydpi =" << ydpi()
		<< "/ realisticDpi =" << realisticDpi_
		<< "/ widthPixels =" << widthPixels() << "/ heightPixels =" << heightPixels()
		<< "/ Theme =" << int(theme_) << themeDirectoryName()
		<< "/ densityFromDpi =" << densityFromDpi_
		<< "/ scaledDensityFromDpi =" << scaledDensityFromDpi_;
}

void QAndroidDisplayMetrics::preloadJavaClasses()
{
	QAndroidQPAPluginGap::preloadJavaClasses();
	QAndroidQPAPluginGap::preloadJavaClass("android/util/DisplayMetrics");
	QAndroidQPAPluginGap::preloadJavaClass("android/content/res/Resources");
	QAndroidQPAPluginGap::preloadJavaClass("android/content/res/Configuration");
}

QString QAndroidDisplayMetrics::themeDirectoryName(Theme theme)
{
	switch(theme)
	{
	case ThemeLDPI:   return QLatin1String("ldpi");
	case ThemeMDPI:   return QLatin1String("mdpi");
	case ThemeTVDPI:  return QLatin1String("tvdpi");
	case ThemeHDPI:   return QLatin1String("hdpi");
	case Theme280DPI: return QLatin1String("280dpi");
	case ThemeXHDPI:  return QLatin1String("xhdpi");
	case Theme360DPI: return QLatin1String("360dpi");
	case Theme400DPI: return QLatin1String("400dpi");
	case Theme420DPI: return QLatin1String("420dpi");
	case ThemeXXDPI:  return QLatin1String("xxdpi");
	case Theme560DPI: return QLatin1String("560dpi");
	case ThemeXXXDPI: return QLatin1String("xxxdpi");
	default:
		qWarning() << "Unknown theme value:" << theme;
		return QString::null;
	};
}

float QAndroidDisplayMetrics::fontScale()
{
	QAndroidQPAPluginGap::Context activity;
	QScopedPointer<QJniObject> resources(activity.callObject("getResources", "android/content/res/Resources"));
	QScopedPointer<QJniObject> configuration(resources->callObject("getConfiguration", "android/content/res/Configuration"));
	jfloat font_scale = configuration->getFloatField("fontScale");
	return static_cast<float>(font_scale);
}
