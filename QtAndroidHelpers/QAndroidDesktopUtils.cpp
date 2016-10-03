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

#include <QAndroidQPAPluginGap.h>
#include "QAndroidDesktopUtils.h"

namespace QAndroidDesktopUtils {

static const char * const c_full_class_name_ = "ru/dublgis/androidhelpers/DesktopUtils";

void preloadJavaClasses()
{
	static bool s_preloaded = false;
	if (!s_preloaded)
	{
		QAndroidQPAPluginGap::preloadJavaClass(c_full_class_name_);
		s_preloaded = true;
	}

}

bool isInternetActive()
{
	QJniClass du(c_full_class_name_);
	QAndroidQPAPluginGap::Context activity;
	jint result = du.callStaticParamInt("isInternetActive", "Landroid/content/Context;", activity.jObject());
	if (result == 0)
	{
		return false;
	}
	else if (result > 0)
	{
		return true;
	}
	else
	{
		throw QJniBaseException("isInternetActive exception");
	}
}

int getNetworkType()
{
	QJniClass du(c_full_class_name_);
	QAndroidQPAPluginGap::Context activity;
	return static_cast<int>(du.callStaticParamInt("getNetworkType", "Landroid/content/Context;", activity.jObject()));
}

bool sendTo(const QString & chooser_caption, const QString & text, const QString & content_type)
{
	QJniClass du(c_full_class_name_);
	QAndroidQPAPluginGap::Context activity;
	return du.callStaticParamBoolean("sendTo", "Landroid/content/Context;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;",
		activity.jObject(),
		QJniLocalRef(chooser_caption).jObject(),
		QJniLocalRef(text).jObject(),
		QJniLocalRef(content_type).jObject());
}

bool sendSMS(const QString & number, const QString & text)
{
	QJniClass du(c_full_class_name_);
	QAndroidQPAPluginGap::Context activity;
	return du.callStaticParamBoolean("sendSMS", "Landroid/content/Context;Ljava/lang/String;Ljava/lang/String;",
		activity.jObject(),
		QJniLocalRef(number).jObject(),
		QJniLocalRef(text).jObject());
}

bool sendEmail(
	const QString & to,
	const QString & subject,
	const QString & body,
	const QString & attach_file,
	bool force_content_provider,
	const QString & authorities)
{
	QJniClass du(c_full_class_name_);
	QAndroidQPAPluginGap::Context activity;
	return du.callStaticParamBoolean(
		"sendEmail",
		"Landroid/content/Context;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;ZLjava/lang/String;",
		activity.jObject(),
		QJniLocalRef(to).jObject(),
		QJniLocalRef(subject).jObject(),
		QJniLocalRef(body).jObject(),
		QJniLocalRef(attach_file).jObject(),
		static_cast<jboolean>(force_content_provider),
		QJniLocalRef(authorities).jObject());
}

bool sendEmail(
	const QString & to,
	const QString & subject,
	const QString & body,
	const QStringList & attachment,
	const QString & authorities)
{
	QJniEnvPtr jep;

	QJniLocalRef attachment_array{jep.env()->NewObjectArray(attachment.size(), QJniClass{"java/lang/String"}.jClass(), nullptr)};

	for (QStringList::size_type i = 0; i < attachment.size(); ++i)
	{
		jep.env()->SetObjectArrayElement(static_cast<jobjectArray>(attachment_array.jObject()), i, QJniLocalRef{attachment[i]}.jObject());
	}

	QJniClass du(c_full_class_name_);
	QAndroidQPAPluginGap::Context activity;
	return du.callStaticParamBoolean(
		"sendEmail",
		"Landroid/content/Context;"
		"Ljava/lang/String;" // to
		"Ljava/lang/String;" // subject
		"Ljava/lang/String;" // body
		"[Ljava/lang/String;" // attachment
		"Ljava/lang/String;", // authorities
		activity.jObject(),
		QJniLocalRef(to).jObject(),
		QJniLocalRef(subject).jObject(),
		QJniLocalRef(body).jObject(),
		static_cast<jobjectArray>(attachment_array.jObject()),
		QJniLocalRef(authorities).jObject());
}

bool openURL(const QString & url)
{
	QJniClass du(c_full_class_name_);
	QAndroidQPAPluginGap::Context activity;
	return du.callStaticParamBoolean("openURL", "Landroid/content/Context;Ljava/lang/String;",
		activity.jObject(),
		QJniLocalRef(url).jObject());
}

bool openFile(const QString & fileName, const QString & mimeType)
{
	QJniClass du(c_full_class_name_);
	QAndroidQPAPluginGap::Context activity;
	return du.callStaticParamBoolean("openFile", "Landroid/content/Context;Ljava/lang/String;Ljava/lang/String;",
		activity.jObject(),
		QJniLocalRef(fileName).jObject(),
		QJniLocalRef(mimeType).jObject());
}

bool installApk(const QString & apk)
{
	QJniClass du(c_full_class_name_);
	QAndroidQPAPluginGap::Context activity;
	return du.callStaticParamBoolean("installApk", "Landroid/content/Context;Ljava/lang/String;",
		activity.jObject(),
		QJniLocalRef(apk).jObject());
}

void uninstallApk(const QString & packagename)
{
	QJniClass du(c_full_class_name_);
	QAndroidQPAPluginGap::Context activity;
	du.callStaticParamVoid("uninstallApk", "Landroid/content/Context;Ljava/lang/String;",
		activity.jObject(),
		QJniLocalRef(packagename).jObject());
}

bool callNumber(const QString & number)
{
	QJniClass du(c_full_class_name_);
	QAndroidQPAPluginGap::Context activity;
	return du.callStaticParamBoolean("callNumber", "Landroid/content/Context;Ljava/lang/String;",
		activity.jObject(),
		QJniLocalRef(number).jObject());
}

QString getTelephonyDeviceId()
{
	QJniClass du(c_full_class_name_);
	QAndroidQPAPluginGap::Context activity;
	return du.callStaticParamString("getTelephonyDeviceId", "Landroid/content/Context;", activity.jObject());
}

QString getDisplayCountry()
{
	QJniClass du(c_full_class_name_);
	QAndroidQPAPluginGap::Context activity;
	return du.callStaticParamString("getDisplayCountry", "Landroid/content/Context;", activity.jObject());
}

QString getCountry()
{
	QJniClass du(c_full_class_name_);
	QAndroidQPAPluginGap::Context activity;
	return du.callStaticParamString("getCountry", "Landroid/content/Context;", activity.jObject());
}

QString getAndroidId()
{
	QJniClass du(c_full_class_name_);
	QAndroidQPAPluginGap::Context activity;
	return du.callStaticParamString("getAndroidId", "Landroid/content/Context;", activity.jObject());
}

QString getBuildSerial()
{
	QJniClass du(c_full_class_name_);
	QAndroidQPAPluginGap::Context activity;
	return du.callStaticString("getBuildSerial");
}

QStringList getInstalledAppsList()
{
	QJniClass du(c_full_class_name_);
	QAndroidQPAPluginGap::Context activity;
	QString list = du.callStaticParamString("getInstalledAppsList", "Landroid/content/Context;", activity.jObject());
	return list.split(QChar('\n'), QString::SkipEmptyParts);
}

QString getUniqueDeviceId()
{
	static bool device_id_initialized = false;
	static QString device_id;

	if (!device_id_initialized)
	{
		device_id_initialized = true;

		// Best option is to use telephony hardware ID (IMEI).
		QString telephony_id = getTelephonyDeviceId();
		if (!telephony_id.isEmpty())
		{
			device_id = QLatin1String("Tel.Id:") + telephony_id;
		}
		else
		{
			// Device serial number (Android 2.3+), should be available on all
			// devices without telephony id.
			QString build_serial = getBuildSerial();
			if (!build_serial.isEmpty())
			{
				device_id = QLatin1String("Build:") + build_serial;
			}
			else
			{
				// The last option: use ANDROID_ID which is generated when device
				// is connected to Google account. Reset on device reset.
				// Broken on some Android 2.2 devices.
				QString android_id = getAndroidId();
				if (!android_id.isEmpty())
				{
					device_id = QLatin1String("Android Id:") + android_id;
				}
				else
				{
					device_id = QString::null;
				}
			}
		}
	}
	return device_id;
}

QString getDefaultLocaleName()
{
	QJniClass du(c_full_class_name_);
	if (du.jClass())
	{
		return du.callStaticString("getDefaultLocaleName");
	}
	else
	{
		qCritical() << "Null class:" << c_full_class_name_;
		return QLatin1String("C");
	}
}

bool checkSelfPermission(const QString & permission_name)
{
	if (QAndroidQPAPluginGap::apiLevel() < 23)
	{
		return true;
	}
	else
	{
		jint result = QAndroidQPAPluginGap::Context().callParamInt(
			"checkSelfPermission"
			, "Ljava/lang/String;"
			, QJniLocalRef(permission_name).jObject());
		return result == 0; // PERMISSION_GRANTED
	}
}

bool shouldShowRequestPermissionRationale(const QString & permission_name)
{
	if (QAndroidQPAPluginGap::apiLevel() < 23)
	{
		return false;
	}
	else
	{
		return QAndroidQPAPluginGap::Context().callParamBoolean(
			"shouldShowRequestPermissionRationale"
			, "Ljava/lang/String;"
			, QJniLocalRef(permission_name).jObject());
	}
}

void requestPermissions(const QStringList & permission_names, int permission_request_code)
{
	if (QAndroidQPAPluginGap::apiLevel() >= 23 && !permission_names.isEmpty())
	{
		QJniEnvPtr jep;

		QJniLocalRef permission_array(
			jep.env()->NewObjectArray(
				  permission_names.count() // size
				  , QJniClass("java/lang/String").jClass() // type
				  , 0)); // initial object

		for (int i = 0; i < permission_names.count(); ++i)
		{
			jep.env()->SetObjectArrayElement(
				static_cast<jobjectArray>(permission_array.jObject())
				, i
				, QJniLocalRef(permission_names.at(i)).jObject());
		}
		QAndroidQPAPluginGap::Context().callParamVoid(
			"requestPermissions"
			, "[Ljava/lang/String;I"
			, static_cast<jobjectArray>(permission_array.jObject())
			, static_cast<jint>(permission_request_code));
	}
}


} // namespace QAndroidDesktopUtils


