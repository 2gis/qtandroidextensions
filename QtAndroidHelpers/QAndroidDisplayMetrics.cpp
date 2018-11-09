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

#include <QJniHelpers/QJniHelpers.h>
#include <QJniHelpers/QAndroidQPAPluginGap.h>
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


// Find a theme that matches logical density.
// Normally, density_dpi should be EXACT MATCH to one of the known themes.
// However, if the application doesn't want to use some intermediate densities
// and the device offers one, it will return the next "more round" theme.
static QAndroidDisplayMetrics::Theme themeFromLogicalDensity(
	int density_dpi
	, QAndroidDisplayMetrics::IntermediateDensities intermediate_densities)
{
	QAndroidDisplayMetrics::Theme resulting_theme = all_themes[0].theme;
	for (size_t i = 1; i < all_themes_count; ++i)
	{
		// Skip unwanted themes
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


// Returns ratio of a/b or b/a, the one which is <= 1.0.
// The bigger the ratio, the closer (relatively) the numbers are.
// a, b, should be > 1e-6.
static float proximityRatio(float a, float b)
{
	return (a <= 1e-6f || b <= 1e-6f)
		? 0.0f
		: ((a > b) ? b / a : a / b);
}


// Find a theme that matches hardware density (thus ignoring manufacturer's choice
// of the theme for the device). The matching is done by choosing the theme that
// has the closest DPI to the hardware value.
static QAndroidDisplayMetrics::Theme themeFromHardwareDensity(
	float density_dpi
	, QAndroidDisplayMetrics::IntermediateDensities intermediate_densities)
{
	QAndroidDisplayMetrics::Theme resulting_theme = all_themes[0].theme;
	float bestK = proximityRatio(density_dpi, static_cast<float>(all_themes[0].starting_ppi));
	for (size_t i = 1; i < all_themes_count; ++i)
	{
		// Skip unwanted themes
		if (all_themes[i].availability > intermediate_densities)
		{
			continue;
		}
		const float k = proximityRatio(density_dpi, static_cast<float>(all_themes[i].starting_ppi));
		if (k > bestK)
		{
			bestK = k;
			resulting_theme = all_themes[i].theme;
		}
	}
	return resulting_theme;
}


// Find out which actual hardware DPI we have. Some Android devices report wrong hardware
// DPI values so we have to use some heuristics.
static float guessRealisticHardwareDpi(float xdpi, float ydpi, float logicalDpi)
{
	// Let's start with average hardware resoltion.
	float realisticDpi = (xdpi + ydpi) / 2.0f;
	if (logicalDpi > 0.0f)
	{
		// Max difference between standard adjacent logical DPI values is 1.5 or 0.66.
		// If hardware DPI differs from logical DPI more than 0.66 times we assume
		// that the hardware value is set wrong and fall back to the logical value.
		if (proximityRatio(realisticDpi, logicalDpi) < 0.66f)
		{
			qWarning() << "Average hardware DPI is reported as" << realisticDpi
				<< "but logical DPI is" << logicalDpi
				<< "(too different). Falling back to the logical value.";
			realisticDpi = logicalDpi;
		}
	}
	return realisticDpi;
}


QAndroidDisplayMetrics::QAndroidDisplayMetrics(
		QObject * parent
		, QAndroidDisplayMetrics::IntermediateDensities allow_intermediate_densities)
	: QObject(parent)
	, density_(1.0f)
	, densityDpi_(160)
	, scaledDensity_(1.0f)
	, densityFromSystemTheme_(1.0f)
	, scaledDensityFromSystemTheme_(1.0f)
	, densityFromHardwareDpiTheme_(1.0f)
	, scaledDensityFromHardwareDpiTheme_(1.0f)
	, physicalXDpi_(160.0f)
	, physicalYDpi_(160.0f)
	, realisticPhysicalDpi_(160.0f)
	, widthPixels_(240)
	, heightPixels_(240)
	, themeFromDensityDpi_(ThemeMDPI)
	, themeFromHardwareDpi_(ThemeMDPI)
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
	physicalXDpi_ = metrics.getFloatField("xdpi");
	physicalYDpi_ = metrics.getFloatField("ydpi");
	widthPixels_ = metrics.getIntField("widthPixels");
	heightPixels_ = metrics.getIntField("heightPixels");

	realisticPhysicalDpi_ = guessRealisticHardwareDpi(
		physicalXDpi_
		, physicalYDpi_
		, static_cast<float>(densityDpi_));

	themeFromDensityDpi_ = themeFromLogicalDensity(densityDpi_, allow_intermediate_densities);
	themeFromHardwareDpi_ = themeFromHardwareDensity(realisticPhysicalDpi_, allow_intermediate_densities);

	densityFromSystemTheme_ = densityFromTheme(themeFromDensityDpi_);
	densityFromHardwareDpiTheme_ = densityFromTheme(themeFromHardwareDpi_);

	const float scale = (density_ > 0.0f) ? (scaledDensity_ / density_) : 1.0f;

	scaledDensityFromSystemTheme_ = densityFromSystemTheme_ * scale;
	scaledDensityFromHardwareDpiTheme_ = densityFromHardwareDpiTheme_ * scale;

	qDebug()
		<< "QAndroidDisplayMetrics: DP =" << density()
		<< "/ logical DPI =" << densityDpi()
		<< "/ scaled DP =" << scaledDensity()
		<< "/ DPI X =" << xdpi() << "/ Y =" << ydpi()
		<< "/ realistic DPI =" << realisticPhysicalDpi_
		<< "/ W =" << widthPixels() << "/ H =" << heightPixels()

		<< "/ SYSTEM THEME:" << static_cast<int>(themeFromDensityDpi_) << themeDirectoryName()
		<< "/ density =" << densityFromSystemTheme_
		<< "/ scaled =" << scaledDensityFromSystemTheme_

		<< "/ HARDWARE THEME:" << static_cast<int>(themeFromHardwareDpi_) << themeDirectoryName(themeFromHardwareDpi_)
		<< "/ density =" << densityFromHardwareDpiTheme_
		<< "/ scale =" << scaledDensityFromHardwareDpiTheme_;
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
