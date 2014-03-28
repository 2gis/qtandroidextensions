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

#include <QtCore/QString>
#include <QtCore/QObject>

/*!
 * Access to Android's DisplayMetrics. The metrics are read from system API when the
 * object is constructed.
 * \see http://developer.android.com/reference/android/util/DisplayMetrics.html
 */
class QAndroidDisplayMetrics: public QObject
{
	Q_OBJECT
	Q_PROPERTY(float density READ density)
	Q_PROPERTY(int densityDpi READ densityDpi)
	Q_PROPERTY(int heightPixels READ heightPixels)
	Q_PROPERTY(float scaledDensity READ scaledDensity)
	Q_PROPERTY(int widthPixels READ widthPixels)
	Q_PROPERTY(float xdpi READ xdpi)
	Q_PROPERTY(float ydpi READ ydpi)
public:
	static const int
		ANDROID_DENSITY_LOW		= 120,
		ANDROID_DENSITY_DEFAULT = 160,
		ANDROID_DENSITY_MEDIUM  = 160,
		ANDROID_DENSITY_TV		= 213,
		ANDROID_DENSITY_HIGH	= 240,
		ANDROID_DENSITY_XHIGH	= 320,
		ANDROID_DENSITY_400		= 400,
		ANDROID_DENSITY_XXHIGH	= 480,
		ANDROID_DENSITY_XXXHIGH	= 640;

	QAndroidDisplayMetrics(QObject * parent = 0);
	static void preloadJavaClasses();

	/*!
	 * Size multiplier relative to size optimized for the default (medium) DPI (160).
	 * As for 2014/03 the value seems to be varying from 0.75 to 3.
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
	 */
	float scaledDensity() const { return scaledDensity_; }

	//! Exact physical resolution of the screen.
	float xdpi() const { return xdpi_; }

	//! Exact physical resolution of the screen.
	float ydpi() const { return ydpi_; }

	//! Pixel size of the screen
	int widthPixels() const { return widthPixels_; }

	//! Pixel size of the screen
	int heightPixels() const { return heightPixels_; }

private:
	float density_;
	int densityDpi_;
	float scaledDensity_;
	float xdpi_;
	float ydpi_;
	int widthPixels_;
	int heightPixels_;

};
