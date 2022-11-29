/*
  Lightweight access to various Android APIs for Qt

  Author:
  Sergey A. Galin <sergey.galin@gmail.com>

  Distrbuted under The BSD License

  Copyright (c) 2014-2022, DoubleGIS, LLC.
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

#pragma once
#include <QtCore/QObject>
#include <QtCore/QMap>
#include <QJniHelpers/QJniHelpers.h>
#include <memory>


/*!
 * Access to Android's DisplayMetrics. The metrics are read from system API when the
 * object is constructed.
 * \see http://developer.android.com/reference/android/util/DisplayMetrics.html
 * In addition to the values read straight from DisplayMetrics, the class provides
 * some commonly used calculated / detected values, see: theme(), themeDirectoryName(),
 * densityFromDpi(), scaledDensityFromDpi(), realisticDpi().
 */
class QAndroidDisplayMetrics: public QObject
{
	Q_OBJECT
public:
	enum Theme {
		// Themes in order of their density.
		// The integer values are just hints for readability.
		ThemeNone   = 0,
		ThemeLDPI   = 1,
		ThemeMDPI   = 2,
		ThemeTVDPI  = 3,
		ThemeHDPI   = 4,
		Theme280DPI = 5,
		ThemeXHDPI  = 6,
		Theme360DPI = 7,
		Theme400DPI = 8,
		Theme420DPI = 9,
		ThemeXXDPI  = 10,
		Theme560DPI = 11,
		ThemeXXXDPI = 12
	};
	Q_ENUMS(Theme)

private:
	Q_PROPERTY(float density READ density)
	Q_PROPERTY(int densityDpi READ densityDpi)
	Q_PROPERTY(float scaledDensity READ scaledDensity)
	Q_PROPERTY(float densityFromSystemTheme READ densityFromSystemTheme)
	Q_PROPERTY(float scaledDensityFromSystemTheme READ scaledDensityFromSystemTheme)
	Q_PROPERTY(float densityFromHardwareDpiTheme READ densityFromHardwareDpiTheme)
	Q_PROPERTY(float scaledDensityFromHardwareDpiTheme READ scaledDensityFromHardwareDpiTheme)
	Q_PROPERTY(float xdpi READ xdpi)
	Q_PROPERTY(float ydpi READ ydpi)
	Q_PROPERTY(float realisticDpi READ realisticDpi)
	Q_PROPERTY(int widthPixels READ widthPixels)
	Q_PROPERTY(int heightPixels READ heightPixels)
	Q_PROPERTY(int realWidthPixels READ realWidthPixels)
	Q_PROPERTY(int realHeightPixels READ realHeightPixels)
	Q_PROPERTY(Theme theme READ theme)
	Q_PROPERTY(Theme hardwareTheme READ hardwareTheme)
	Q_PROPERTY(QString themeDirectoryName READ themeDirectoryName)
	Q_PROPERTY(QString hardwareThemeDirectoryName READ hardwareThemeDirectoryName)

public:
	// Correctness of the names and constants can be verified here:
	// http://developer.android.com/intl/ru/guide/practices/screens_support.html
	// http://developer.android.com/intl/ru/reference/android/util/DisplayMetrics.html
	static const int
		ANDROID_DENSITY_LOW      = 120,
		ANDROID_DENSITY_DEFAULT  = 160,
		ANDROID_DENSITY_MEDIUM   = 160,
		ANDROID_DENSITY_TV       = 213,
		ANDROID_DENSITY_HIGH     = 240,
		ANDROID_DENSITY_280      = 280,
		ANDROID_DENSITY_XHIGH    = 320,
		ANDROID_DENSITY_360      = 360,
		ANDROID_DENSITY_400      = 400,
		ANDROID_DENSITY_420      = 420,
		ANDROID_DENSITY_XXHIGH   = 480,
		ANDROID_DENSITY_560      = 560,
		ANDROID_DENSITY_XXXHIGH  = 640;

	// Enum to control selection on additional intermediate screen densities.
	// The bigger the integer value, the more possible themes can be matched.
	enum IntermediateDensities
	{
		IntermediateNone        = 0, // Use only major themes (integer and below 1.0 densities).
		IntermediateWithStep0_5 = 1, // Use major themes and X.5 themes.
		IntermediateAll         = 2  // Use all possible themes, including X.33 and X.67 themes.
	};


	static std::unique_ptr<QJniObject> getWindowManager(QJniObject * custom_context = nullptr);
	static std::unique_ptr<QJniObject> getDefaultDisplay(QJniObject * custom_context = nullptr);

	// Get actual refresh rate (will return 0 if it can't be determined).
	static float getRefreshRate(QJniObject * custom_context = nullptr);
	// Get list of refresh rates for seamless switching in current display mode (Android 12+,
	// device / mode must support seamless switching).
	static std::vector<float> getAlternativeRefreshRates(QJniObject & mode);
	static std::vector<float> getAlternativeRefreshRatesForCurrentMode(QJniObject * custom_context = nullptr);
	// Get list of refresh rates supported by the device.
	// The function is deprecated (API 21-22) but on some devices may give more information than
	// newer getSupportedModes().
	static std::vector<float> getSupportedRefreshRates(QJniObject * custom_context = nullptr);

	struct Mode
	{
		int id;
		int physicalWidth;
		int physicalHeight;
		float refreshRate;
		std::vector<float> alternativeRefreshRates;

		Mode(int i, int w, int h, int rate, std::vector<float> && altRates)
			: id(i)
			, physicalWidth(w)
			, physicalHeight(h)
			, refreshRate(rate)
			, alternativeRefreshRates(std::move(altRates))
		{
		}
	};
	static std::vector<Mode> getSupportedModes(QJniObject * custom_context = nullptr);


	static float densityFromTheme(QAndroidDisplayMetrics::Theme theme);
	static Theme themeFromLogicalDensity(
		int density_dpi,
		IntermediateDensities intermediate_densities = IntermediateAll);
	static Theme themeFromHardwareDensity(
		float density_dpi,
		IntermediateDensities intermediate_densities = IntermediateAll);
	static float densityFromDpi(
		float density_dpi,
		IntermediateDensities intermediate_densities = IntermediateAll);

	// custom_context can be set to use non-standard Context, e.g. in Android Auto app it may be
	// a car context to get metrics of the car screen.
	QAndroidDisplayMetrics(
		QObject * parent = 0,
		IntermediateDensities allow_intermediate_densities = IntermediateAll,
		QJniObject * custom_context = nullptr);

	static void preloadJavaClasses();

	// Basic pixel density for this device as reported by the system.
	// This is the number used to scale layout designed for a device with default (medium)
	// resolution (160 DPI) to match the current device.
	// Note: some devices have wrong density value based on wrong physical DPI value.
	// It is safer to use densityFromSystemTheme() or scaledDensityFromSystemTheme(),
	// especially when supporting older Androids.
	float density() const { return density_;  }

	// Logical resolution of the screen (the one that matches density()).
	// It is a value selected by maker of the device from the set of standard resolutions
	// and should be something close to the hardware resolution.
	// This function should return value from one of the ANDROID_DENSITY_... constants
	// and can be used to select set of graphical resources appopriate for the device.
	int densityDpi() const { return densityDpi_; }

	// Size multiplier relative to size optimized for the default (medium) DPI (160),
	// but also takes into account user's enlarged/reduced font setting.
	// See also comments for density().
	float scaledDensity() const { return scaledDensity_; }

	// Device's default theme, based on densityDpi().
	Theme theme() const { return themeFromDensityDpi_; }

	// Device's theme based on the hardware parameters rather than the manufacturer's choice.
	Theme hardwareTheme() const { return themeFromHardwareDpi_; }

	// Standard resource directory name (suffix) used for the theme: "ldpi", "mdpi", and etc.
	static QString themeDirectoryName(Theme theme);

	// Standard resource directory name  for the current device's theme().
	QString themeDirectoryName() const { return themeDirectoryName(theme()); }

	// Standard resource directory name for theme based on hardware parameters rather
	// than manufacturer's choice.
	QString hardwareThemeDirectoryName() const { return themeDirectoryName(hardwareTheme()); }

	// Workaround function: get screen density exactly matching theme().
	// This is more reliable as some older devices have wrong density value, but
	// logical DPI is always correct.
	// On all properly configured devices the value should be the same as density().
	// Old name: densityFromDpi().
	float densityFromSystemTheme() const { return densityFromSystemTheme_; }

	// Get screen density for the theme based on the hardware parameters rather than
	// the the manufacturer's choice.
	float densityFromHardwareDpiTheme() const { return densityFromHardwareDpiTheme_; }

	// Workaround function: get screen densityFromDpi() and apply user setting
	// for UI scaling.
	// Old name: scaledDensityFromDpi().
	float scaledDensityFromSystemTheme() const { return scaledDensityFromSystemTheme_; }

	// Get scaled screen density for the theme based on the hardware parameters rather than
	// manufacturer's choice.
	float scaledDensityFromHardwareDpiTheme() const { return scaledDensityFromHardwareDpiTheme_; }

	// Exact physical resolution of the screen.
	// Warning: it may be set to a wrong value on some devices (due to manufacturer's
	// error or bad way of UI tweaking).
	float xdpi() const { return physicalXDpi_; }
	float ydpi() const { return physicalYDpi_; }

	// Returns the best possible bet on physical screen resolution.
	// This is either average between xdpi and ydpi, or logical DPI if
	// it is too different from the reported physical DPI.
	// This value works good for phones and tablets with wrong physical
	// DPI setting.
	float realisticDpi() const { return realisticPhysicalDpi_; }

	// Pixel size of the screen
	int widthPixels() const { return widthPixels_; }
	int heightPixels() const { return heightPixels_; }

	// Pixel size of the screen without subtracting any window decor or applying any compatibility scale factors
	int realWidthPixels() const { return realWidthPixels_; }
	int realHeightPixels() const { return realHeightPixels_; }

	// Font scale, as configured by user.
	// Please note that this value may changes during run-time, so it is a good
	// idea to check it on each application activation.
	static float fontScale();

	// NB: As other fields, this is a value that has been read once during creation of the object.
	// Starting from Android 11, refresh rate can be changed dynamically.
	float refreshRate() const { return refreshRate_; }

private:
	float density_;
	int densityDpi_;
	float scaledDensity_;
	float densityFromSystemTheme_;
	float scaledDensityFromSystemTheme_;
	float densityFromHardwareDpiTheme_;
	float scaledDensityFromHardwareDpiTheme_;
	float physicalXDpi_;
	float physicalYDpi_;
	float realisticPhysicalDpi_;
	int widthPixels_;
	int heightPixels_;
	int realWidthPixels_;
	int realHeightPixels_;
	Theme themeFromDensityDpi_;
	Theme themeFromHardwareDpi_;
	float refreshRate_;
};

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
	Q_DECLARE_METATYPE(QAndroidDisplayMetrics::Theme)
#endif

