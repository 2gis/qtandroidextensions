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

#pragma once
#include <QtCore/QString>
#include <QtCore/QObject>
#include <QtCore/QMap>

/*!
 * Access to Android's Configuration.
 * \see http://developer.android.com/reference/android/content/res/Configuration.html
 *
 * \todo NOTE: This class does not provide access to all fields of Configuration yet!
 */
class QAndroidConfiguration: public QObject
{
	Q_OBJECT
public:
	enum ScreenSize {
		ScreenSizeUndefined = 0,
		ScreenSizeSmall		= 1,
		ScreenSizeNormal	= 2,
		ScreenSizeLarge		= 3,
		ScreenSizeXLarge	= 4
	};
	Q_ENUMS(ScreenSize)

private:
	Q_PROPERTY(ScreenSize screenSize READ screenSize)
	Q_PROPERTY(bool isTablet READ isTablet)

public:
	QAndroidConfiguration(QObject * parent = 0);
	static void preloadJavaClasses();

	static const int
		ANDROID_SCREENLAYOUT_SIZE_MASK = 0x0000000f,
		ANDROID_SCREENLAYOUT_SIZE_UNDEFINED = 0x00000000,
		ANDROID_SCREENLAYOUT_SIZE_SMALL = 0x00000001,
		ANDROID_SCREENLAYOUT_SIZE_NORMAL = 0x00000002,
		ANDROID_SCREENLAYOUT_SIZE_LARGE = 0x00000003,
		ANDROID_SCREENLAYOUT_SIZE_XLARGE = 0x00000004;

	/*!
	 * Convert ScreenSize enum to "small", "normal", "large", "xlarge".
	 * \return Returns screen size name; for undefined or unknown size return "normal".
	 */
	static QString screenSizeName(ScreenSize size);

	QString screenSizeName() const { return screenSizeName(screen_size_); }

	ScreenSize screenSize() const { return screen_size_; }

	/*!
	 * This function returns true for "tablet-sized" screens. (Note that it may
	 * also return true for other big srceens, like monitor, TV and etc.)
	 * Note: an application should check both physical screen size and screen
	 * resolution (e.g. as provided by QAndroidDisplayMetrics) before deciding
	 * to enable tablet UI.
	 */
	bool isTablet() const { return screen_size_ >= ScreenSizeLarge; }

private:
	ScreenSize screen_size_;
};

Q_DECLARE_METATYPE(QAndroidConfiguration::ScreenSize)
