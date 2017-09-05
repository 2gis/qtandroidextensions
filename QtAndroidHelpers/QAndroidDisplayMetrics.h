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

#pragma once
#include <QtCore/QString>
#include <QtCore/QObject>
#include <QtCore/QMap>

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
	Q_PROPERTY(float densityFromDpi READ densityFromDpi)
	Q_PROPERTY(float scaledDensityFromDpi READ scaledDensityFromDpi)
	Q_PROPERTY(float xdpi READ xdpi)
	Q_PROPERTY(float ydpi READ ydpi)
	Q_PROPERTY(float realisticDpi READ realisticDpi)
	Q_PROPERTY(int widthPixels READ widthPixels)
	Q_PROPERTY(int heightPixels READ heightPixels)
	Q_PROPERTY(Theme theme READ theme)
	Q_PROPERTY(QString themeDirectoryName READ themeDirectoryName)

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

	enum IntermediateDensities
	{
		// Themes availablity.
		// The integer values are just hints for readability.
		IntermediateNone        = 0, // Use only major themes (integer and below 1.0 densities).
		IntermediateWithStep0_5 = 1, // Use major themes and X.5 themes.
		IntermediateAll         = 2  // Use all possible themes, including X.33 and X.67 themes.
	};

	QAndroidDisplayMetrics(QObject * parent = 0, IntermediateDensities intermediate_densities = IntermediateAll);
	static void preloadJavaClasses();

	/*!
	 * Size multiplier relative to size optimized for the default (medium) DPI (160).
	 * As for 2014/03 the value seems to be varying from 0.75 to 3.
	 * Note: some devices have wrong density value based on wrong physical DPI value.
	 * It is safer to use densityFromDpi() or scaledDensityFromDpi().
	 */
	float density() const { return density_;  }

	/*!
	 * "Logical" resoultion of the screen.
	 * This function should return value from one of the ANDROID_DENSITY_... constants
	 * and can be used to select graphical resources.
	 */
	int densityDpi() const { return densityDpi_; }

	/*!
	 * Size multiplier relative to size optimized for the default (medium) DPI (160),
	 * but also takes into account user's enlarged/reduced font setting.
	 * See also comments for density().
	 */
	float scaledDensity() const { return scaledDensity_; }

	Theme theme() const { return theme_; }

	static QString themeDirectoryName(Theme theme);

	QString themeDirectoryName() const { return themeDirectoryName(theme()); }

	/*!
	 * Same as density(), but the value is looked up from a table by densityDpi().
	 * This is more reliable because some devices have wrong density value, but
	 * logical DPI is always correct.
	 */
	float densityFromDpi() const { return densityFromDpi_; }

	/*!
	 * Same as density(), but the value is looked up from a table by densityDpi().
	 * This is more reliable because some devices have wrong density value, but
	 * logical DPI is always correct.
	 */
	float scaledDensityFromDpi() const { return scaledDensityFromDpi_; }

	/*!
	 * Exact physical resolution of the screen.
	 * Maybe set to wrong value on some devices.
	 */
	float xdpi() const { return xdpi_; }

	/*!
	 * Exact physical resolution of the screen.
	 * Maybe set to wrong value on some devices.
	 */
	float ydpi() const { return ydpi_; }

	/*!
	 * Returns the most probable value of physical screen DPI.
	 * This is either average between xdpi and ydpi, or logical DPI if
	 * it is too different from the reported physical DPI.
	 * This value works good for phones and tablets with wrong physical
	 * DPI setting.
	 */
	float realisticDpi() const { return realisticDpi_; }

	//! Pixel size of the screen
	int widthPixels() const { return widthPixels_; }

	//! Pixel size of the screen
	int heightPixels() const { return heightPixels_; }

	// Font scale, as configured by user.
	// Please note that this value may changes during run-time, so it is a good
	// idea to check it on each application activation.
	static float fontScale();

private:
	float density_;
	int densityDpi_;
	float scaledDensity_;
	float densityFromDpi_;
	float scaledDensityFromDpi_;
	float xdpi_;
	float ydpi_;
	float realisticDpi_;
	int widthPixels_;
	int heightPixels_;
	Theme theme_;
};

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
	Q_DECLARE_METATYPE(QAndroidDisplayMetrics::Theme)
#endif

