/*
  Lightweight access to various Android APIs for Qt

  Author:
  Sergey A. Galin <sergey.galin@gmail.com>

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

#pragma once
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QObject>

namespace QAndroidDesktopUtils {

void preloadJavaClasses();

bool isInternetActive();

// Returns:
// a. -1 on error
// b. See: http://developer.android.com/reference/android/net/ConnectivityManager.html#TYPE_BLUETOOTH
// for possible returned values.
int getNetworkType();

void showApplicationSettings();

// Send a text to any application in the system.
bool sendTo(
	const QString & chooser_caption,
	const QString & text,
	const QString & content_type = QLatin1String("text/plain"));

// Send a text to any application in the system.
// If some application packages are to be removed from possible recipients, pass list of the
// package names via filter_packages. Use * at end of package name to ignore by the prefix.
bool sendTo(
	const QString & chooser_caption,
	const QString & text,
	const QString & content_type,
	const QStringList & filter_packages);

bool sendSMS(const QString & number, const QString & text);

// WARNING: For this function to work properly with e-mail attachments on Android 6+
// or when using force_content_provider, please follow instructions in
// DesktopUtils.java => DesktopUtils.sendEmail().
bool sendEmail(
	const QString & to,
	const QString & subject,
	const QString & body,
	const QString & attach_file = QString::null,
	bool force_content_provider = false,
	const QString & authorities = QLatin1String("ru.dublgis.sharefileprovider"));

bool sendEmail(
	const QString & to,
	const QString & subject,
	const QString & body,
	const QStringList & attachment,
	const QString & authorities = QLatin1String("ru.dublgis.sharefileprovider"));

bool openURL(const QString & url);
bool openFile(const QString & fileName, const QString & mimeType);
bool installApk(const QString & apk);
void uninstallApk(const QString & packagename);

// Opens the number as "tel:/" + number and ACTION_VIEW, which typically opens phone dialer with
// the number entered. Does not require any special permissions. On some devices, it may open
// an IP telephony app.
bool callNumber(const QString & number, const QString & action = QString());

// Does the same as callNumber(number).
bool showPhoneNumber(const QString & number);

// Check if the device has a telephony capable of making voice calls.
bool isVoiceTelephonyAvailable();

// Opens the number with DIAL action, which usually does the same as showPhoneNumber() but may
// have a different effect if non-standard handler for tel: is used. Does not require any special
// permissions. In case of an error it falls back to showPhoneNumber().
bool dialPhoneNumber(const QString & number);

// If the app is running API < 23 OR we have a "android.permission.CALL_PHONE" it opens the number
// via "CALL" action which causes immediate dialing; otherise, it does dialPhoneNumber().
bool callPhoneNumber(const QString & number);

QString getTelephonyDeviceId();
QString getDisplayCountry();
QString getCountry();
QString getAndroidId();
QString getBuildSerial();
QStringList getInstalledAppsList();

QString getDefaultLocaleName();
QStringList getUserLocaleNames();

// This function tries to find out some string which uniquely identifies this device.
// If it's not available on the device it returns an empty string.
QString getUniqueDeviceId();

// API < 23: always true
bool checkSelfPermission(const QString & permission_name);

// API < 23: always false
bool shouldShowRequestPermissionRationale(const QString & permission_name);

// API < 23: does nothing
void requestPermissions(const QStringList & permission_names, int permission_request_code);

void vibrate(int time_ms);

bool isBluetoothLEAvailable();

// Compress files or directories into a zip archive
bool zipFiles(const QStringList & src_pathes, const QString & dst_path);

} // namespace QAndroidDesktopUtils

