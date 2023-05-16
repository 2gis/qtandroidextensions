/*
  Offscreen Android Views library for Qt

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

#include <QJniHelpers/QJniHelpers.h>
#include "QAndroidDisplayMetrics.h"
#include "QAndroidScreenOrientation.h"
#include <QJniHelpers/TJniObjectLinker.h>


namespace QAndroidScreenOrientation {


static const char * const c_locker_helper_class = "ru/dublgis/androidhelpers/AndroidScreenOrientationHelper$LockerOrientationInfo";

static const JNINativeMethod methods[] = {
	{"getContext", "()Landroid/content/Context;", reinterpret_cast<void*>(QAndroidQPAPluginGap::getCurrentContextNoThrow)},
};

JNI_LINKER_IMPL(QAndroidScreenOrientationHelper, "ru/dublgis/androidhelpers/AndroidScreenOrientationHelper", methods)

int getRequestedOrientation()
{
	try
	{
		QAndroidQPAPluginGap::Context activity;
		jint result = activity.callInt("getRequestedOrientation");
		// qDebug()<<"QAndroidScreenOrientation::getRequestedOrientation"<<result;
		return int(result);
	}
	catch (const std::exception & e)
	{
		qWarning() << "JNI exception in getRequestedOrientation:" << e.what();
		return ANDROID_ACTIVITYINFO_SCREEN_ORIENTATION_UNSPECIFIED;
	}
}


int getSurfaceRotation()
{
	int rotation = ANDROID_SURFACE_ROTATION_UNDEFINED;
	try
	{
		static QMutex s_mutex;
		static QJniObject s_display;
		QMutexLocker locker(&s_mutex);
		if (!s_display)
		{
			QJniObject wm(QAndroidQPAPluginGap::Context().callObj(
				"getWindowManager"
				, "android/view/WindowManager"));
			if (!wm)
			{
				throw QJniBaseException("No WindowManager.");
			}
			s_display = wm.callObj("getDefaultDisplay", "android/view/Display");
			if (!s_display)
			{
				throw QJniBaseException("No default Display.");
			}
		}
		rotation = s_display.callInt("getRotation");
	}
	catch (const std::exception & e)
	{
		qWarning() << "Exception in QAndroidScreenOrientation => getSurfaceRotation:" << e.what();
	}
	return rotation;
}


int getCurrentFixedOrientation()
{
	try
	{
		QAndroidDisplayMetrics dm;
		try
		{
			int rotation = getSurfaceRotation();

			int orientation = ANDROID_ACTIVITYINFO_SCREEN_ORIENTATION_UNSPECIFIED;
			if (((rotation == ANDROID_SURFACE_ROTATION_0 || rotation == ANDROID_SURFACE_ROTATION_180)
					&& dm.heightPixels() > dm.widthPixels()) || // Not rotated or turned over & portrait
				((rotation == ANDROID_SURFACE_ROTATION_90 || rotation == ANDROID_SURFACE_ROTATION_270)
					&& dm.widthPixels() > dm.heightPixels())) // Rotated 90x & landscape
			{
				// Device's natural orientation is portrait:
				switch (rotation)
				{
				case ANDROID_SURFACE_ROTATION_0:
					qDebug()<<"QAndroidScreenOrientation: default=portrait, rotation=0 => portrait";
					orientation = ANDROID_ACTIVITYINFO_SCREEN_ORIENTATION_PORTRAIT;
					break;
				case ANDROID_SURFACE_ROTATION_90:
					qDebug()<<"QAndroidScreenOrientation: default=portrait, rotation=90 => landscape";
					orientation = ANDROID_ACTIVITYINFO_SCREEN_ORIENTATION_LANDSCAPE;
					break;
				case ANDROID_SURFACE_ROTATION_180:
					qDebug()<<"QAndroidScreenOrientation: default=portrait, rotation=180 => reverse portrait";
					orientation = ANDROID_ACTIVITYINFO_SCREEN_ORIENTATION_REVERSE_PORTRAIT;
					break;
				case ANDROID_SURFACE_ROTATION_270:
					qDebug()<<"QAndroidScreenOrientation: default=portrait, rotation=270 => reverse landscape";
					orientation = ANDROID_ACTIVITYINFO_SCREEN_ORIENTATION_REVERSE_LANDSCAPE;
					break;
				default:
					qDebug()<<"QAndroidScreenOrientation: Unknown screen orientation (1):"<<rotation;
					break;
				}
			}
			else
			{
				// Device's natural orientation is landscape (or if the device is square):
				switch (rotation)
				{
				case ANDROID_SURFACE_ROTATION_0:
					qDebug()<<"QAndroidScreenOrientation default=landscape, rotation=0 => landscape";
					orientation = ANDROID_ACTIVITYINFO_SCREEN_ORIENTATION_LANDSCAPE;
					break;
				case ANDROID_SURFACE_ROTATION_90:
					qDebug()<<"QAndroidScreenOrientation default=landscape, rotation=90 => reverse portrait";
					orientation = ANDROID_ACTIVITYINFO_SCREEN_ORIENTATION_REVERSE_PORTRAIT;
					break;
				case ANDROID_SURFACE_ROTATION_180:
					qDebug()<<"QAndroidScreenOrientation default=landscape, rotation=180 => reverse landscape";
					orientation = ANDROID_ACTIVITYINFO_SCREEN_ORIENTATION_REVERSE_LANDSCAPE;
					break;
				case ANDROID_SURFACE_ROTATION_270:
					qDebug()<<"QAndroidScreenOrientation default=landscape, rotation=270 => portrait";
					orientation = ANDROID_ACTIVITYINFO_SCREEN_ORIENTATION_PORTRAIT;
					break;
				default:
					qDebug()<<"QAndroidScreenOrientation Unknown screen orientation (2):"<<rotation;
					break;
				}
			}
			return orientation;
		}
		catch (const std::exception & e)
		{
			qWarning()<<"QAndroidScreenOrientation exception (2):"<<e.what();
		}
		// Totally fallback.
		if (dm.heightPixels() <= dm.widthPixels())
		{
			return ANDROID_ACTIVITYINFO_SCREEN_ORIENTATION_PORTRAIT;
		}
		else
		{
			return ANDROID_ACTIVITYINFO_SCREEN_ORIENTATION_LANDSCAPE;
		}
	}
	catch (const std::exception & e)
	{
		qWarning()<<"QAndroidScreenOrientation exception (1):"<<e.what();
	}
	return ANDROID_ACTIVITYINFO_SCREEN_ORIENTATION_UNSPECIFIED;
}


QAndroidScreenOrientationHelper::QAndroidScreenOrientationHelper()
	: jniLinker_(new JniObjectLinker(this))
{
}

QAndroidScreenOrientationHelper::~QAndroidScreenOrientationHelper()
{
}

LockerInfoPtr QAndroidScreenOrientationHelper::lockRequestedOrientation(int orientation)
{
	try
	{
		if (isJniReady())
		{
			return LockerInfoPtr::create(jni()->callParamObj(
				"lockOrientation",
				c_locker_helper_class,
				"I",
				orientation));
		}
	}
	catch (const std::exception & e)
	{
		qCritical() << "JNI exception in " << __FUNCTION__ << ": " << e.what();
	}
	return {};
}

void QAndroidScreenOrientationHelper::releaseLocker(LockerInfoPtr lockerInfo)
{
	try
	{
		if (isJniReady())
		{
			return jni()->callParamVoid(
				"releaseLocker",
				"Lru/dublgis/androidhelpers/AndroidScreenOrientationHelper$LockerOrientationInfo;",
				lockerInfo->jObject());
		}
	}
	catch (const std::exception & e)
	{
		qCritical() << "JNI exception in " << __FUNCTION__ << ": " << e.what();
	}
}


} // namespace QAndroidScreenOrientation
