/*
  Lightweight access to various Android APIs for Qt

  Author:
  Sergey A. Galin <sergey.galin@gmail.com>

  Distrbuted under The BSD License

  Copyright (c) 2014-2020, DoubleGIS, LLC.
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


float QAndroidDisplayMetrics::densityFromTheme(QAndroidDisplayMetrics::Theme theme)
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
QAndroidDisplayMetrics::Theme QAndroidDisplayMetrics::themeFromLogicalDensity(
	int density_dpi,
	QAndroidDisplayMetrics::IntermediateDensities intermediate_densities)
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
QAndroidDisplayMetrics::Theme QAndroidDisplayMetrics::themeFromHardwareDensity(
	float density_dpi,
	QAndroidDisplayMetrics::IntermediateDensities intermediate_densities)
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


float QAndroidDisplayMetrics::densityFromDpi(
	float density_dpi,
	IntermediateDensities intermediate_densities)
{
	return densityFromTheme(themeFromHardwareDensity(density_dpi, intermediate_densities));
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


std::unique_ptr<QJniObject> QAndroidDisplayMetrics::getWindowManager(QJniObject * custom_context)
{
	std::unique_ptr<QJniObject> windowmanager;
	try
	{
		if (!QAndroidQPAPluginGap::customContextSet() && (!custom_context || !*custom_context))
		{
			//  Works only in Activity, gets its local window manager.
			windowmanager = std::unique_ptr<QJniObject>(QAndroidQPAPluginGap::Context().callObject(
				"getWindowManager",
				"android/view/WindowManager"));
		}
		else
		{
			// Works in any Context. For Service gets system window manager.
			QJniObject * context = nullptr;
			QScopedPointer<QJniObject> default_context;
			if (custom_context && *custom_context)
			{
				context = custom_context;
			}
			else
			{
				default_context.reset(new QAndroidQPAPluginGap::Context());
				context = default_context.data();
			}
			windowmanager = std::unique_ptr<QJniObject>(context->callParamObject(
				"getSystemService",
				"Ljava/lang/Object;",
				"Ljava/lang/String;",
				QJniLocalRef(QStringLiteral("window")).jObject()));
			if (!windowmanager || !windowmanager->jObject())
			{
				qCritical() << "Could not get WindowManager";
				return {};
			}
		}
	}
	catch (const std::exception & e)
	{
		qCritical() << "JNI exception in getWindowManager:" << e.what();
	}
	return std::move(windowmanager);
}


std::unique_ptr<QJniObject> QAndroidDisplayMetrics::getDefaultDisplay(QJniObject * custom_context)
{
	std::unique_ptr<QJniObject> window_manager = getWindowManager(custom_context);
	if (!window_manager)
	{
		return {};
	}
	try
	{
		std::unique_ptr<QJniObject> result(window_manager->callObject(
			"getDefaultDisplay",
			"android/view/Display"));
		if (!result || !result->jObject())
		{
			qCritical() << "Could not get default Display";
			return {};
		}
		return std::move(result);
	}
	catch (const std::exception & e)
	{
		qCritical() << "JNI exception in getDefaultDisplay:" << e.what();
		return {};
	}
}


QAndroidDisplayMetrics::QAndroidDisplayMetrics(
		QObject * parent,
		QAndroidDisplayMetrics::IntermediateDensities allow_intermediate_densities,
		QJniObject * custom_context)
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
	, realWidthPixels_(widthPixels_)
	, realHeightPixels_(heightPixels_)
	, themeFromDensityDpi_(ThemeMDPI)
	, themeFromHardwareDpi_(ThemeMDPI)
	, refreshRate_(0.0f)
{
	if (std::unique_ptr<QJniObject> display = getDefaultDisplay(custom_context))
	{
		QJniObject metrics("android/util/DisplayMetrics", "");
		QJniObject point("android/graphics/Point", "");
		display->callParamVoid("getMetrics", "Landroid/util/DisplayMetrics;", metrics.jObject());

		density_ = metrics.getFloatField("density");
		densityDpi_ = metrics.getIntField("densityDpi");
		scaledDensity_ = metrics.getFloatField("scaledDensity");
		physicalXDpi_ = metrics.getFloatField("xdpi");
		physicalYDpi_ = metrics.getFloatField("ydpi");
		widthPixels_ = metrics.getIntField("widthPixels");
		heightPixels_ = metrics.getIntField("heightPixels");

		display->callParamVoid("getRealSize", "Landroid/graphics/Point;", point.jObject());
		realWidthPixels_ = point.getIntField("x");
		realHeightPixels_ = point.getIntField("y");

		refreshRate_ = display->callFloat("getRefreshRate");
	}

	realisticPhysicalDpi_ = guessRealisticHardwareDpi(
		physicalXDpi_,
		physicalYDpi_,
		static_cast<float>(densityDpi_));

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
		return QString();
	};
}


float QAndroidDisplayMetrics::fontScale()
{
	jfloat font_scale = 1.0f;
	QAndroidQPAPluginGap::Context context;
	QScopedPointer<QJniObject> resources(context.callObject("getResources", "android/content/res/Resources"));
	if (resources)
	{
		QScopedPointer<QJniObject> configuration(resources->callObject("getConfiguration", "android/content/res/Configuration"));
		if (configuration)
		{
			font_scale = configuration->getFloatField("fontScale");
		}
		else
		{
			qCritical() << "Resources configuration is unavailable, use no font scale";
		}
	}
	else
	{
		qCritical() << "Application's resources are unavailable, use no font scale";
	}
	return static_cast<float>(font_scale);
}


float QAndroidDisplayMetrics::getRefreshRate(QJniObject * custom_context)
{
	if (QAndroidQPAPluginGap::apiLevel() >= 23) // Android >= 6
	{
		try
		{
			if (auto display = getDefaultDisplay(custom_context))
			{
				return static_cast<float>(display->callFloat("getRefreshRate"));
			}
		}
		catch (const std::exception & e)
		{
			qCritical() << "JNI exception reading refresh rate: " << e.what();
		}
	}
	return 0.0f;
}


std::vector<float> QAndroidDisplayMetrics::getAlternativeRefreshRates(QJniObject & mode)
{
	std::vector<float> result;
	if (QAndroidQPAPluginGap::apiLevel() < 31 || !mode) // Android < 12 or null mode
	{
		return result;
	}
	try
	{
		if (auto array =std::unique_ptr<QJniObject>(
			mode.callObject("getAlternativeRefreshRates", "[F")))
		{
			for (const auto r: QJniEnvPtr().convert(static_cast<jfloatArray>(array->jObject())))
			{
				result.push_back(static_cast<float>(r));
			}
		}
	}
	catch (const std::exception & e)
	{
		qCritical() << "JNI exception reading alternative refresh rates: " << e.what();
	}
	return result;
}


std::vector<float>
QAndroidDisplayMetrics::getAlternativeRefreshRatesForCurrentMode(QJniObject * custom_context)
{
	std::vector<float> result;
	if (QAndroidQPAPluginGap::apiLevel() < 31) // Android < 12
	{
		return result;
	}
	try
	{
		if (auto display = getDefaultDisplay(custom_context))
		{
			if (!*display)
			{
				qWarning() << "Null Display";
				return result;
			}
			if (auto mode = std::unique_ptr<QJniObject>(
				display->callObject("getMode", "android/view/Display$Mode")))
			{
				result = getAlternativeRefreshRates(*mode);
			}
		}
	}
	catch (const std::exception & e)
	{
		qCritical() << "JNI exception reading alternative refresh rates: " << e.what();
	}
	return result;
}


std::vector<float> QAndroidDisplayMetrics::getSupportedRefreshRates(QJniObject * custom_context)
{
	std::vector<float> result;
	if (QAndroidQPAPluginGap::apiLevel() < 21) // Android < 5.0
	{
		return result;
	}
	try
	{
		if (auto display = getDefaultDisplay(custom_context))
		{
			if (!*display)
			{
				qWarning() << "Null Display";
				return result;
			}
			if (auto array = std::unique_ptr<QJniObject>(
				display->callObject("getSupportedRefreshRates", "[F")))
			{
				for (const auto r:
					 QJniEnvPtr().convert(static_cast<jfloatArray>(array->jObject())))
				{
					result.push_back(static_cast<float>(r));
				}
			}
		}
	}
	catch (const std::exception & e)
	{
		qCritical() << "JNI exception reading supported refresh rates: " << e.what();
	}
	return result;
}


std::vector<QAndroidDisplayMetrics::Mode>
QAndroidDisplayMetrics::getSupportedModes(QJniObject * custom_context)
{
	std::vector<Mode> result;
	if (QAndroidQPAPluginGap::apiLevel() < 23) // Android < 6.0
	{
		return result;
	}
	try
	{
		if (auto display = getDefaultDisplay(custom_context))
		{
			if (!*display)
			{
				qWarning() << "Null Display";
				return result;
			}
			if (auto array = std::unique_ptr<QJniObject>(
				display->callObject("getSupportedModes", "[Landroid/view/Display$Mode;")))
			{
				auto modes = QJniEnvPtr().convert(static_cast<jobjectArray>(array->jObject()));
				for (auto & m: modes)
				{
					result.emplace_back(
						static_cast<int>(m.callInt("getModeId")),
						static_cast<float>(m.callInt("getPhysicalWidth")),
						static_cast<float>(m.callInt("getPhysicalHeight")),
						static_cast<float>(m.callFloat("getRefreshRate")),
						getAlternativeRefreshRates(m));
				}
			}
		}
	}
	catch (const std::exception & e)
	{
		qCritical() << "JNI exception reading supported modes: " << e.what();
	}
	return result;
}


std::optional<int> QAndroidDisplayMetrics::getCurrentModeId(QJniObject * custom_context)
{
	std::optional<int> result;
	if (QAndroidQPAPluginGap::apiLevel() < 23) // Android < 6
	{
		return result;
	}
	try
	{
		if (auto display = getDefaultDisplay(custom_context))
		{
			if (!*display)
			{
				qWarning() << "Null Display";
				return result;
			}
			if (auto mode = std::unique_ptr<QJniObject>(
				display->callObject("getMode", "android/view/Display$Mode")))
			{
				if (*mode)
				{
					result = mode->callInt("getModeId");
				}
			}
		}
	}
	catch (const std::exception & e)
	{
		qCritical() << "JNI exception reading current mode id: " << e.what();
	}
	return result;
}

