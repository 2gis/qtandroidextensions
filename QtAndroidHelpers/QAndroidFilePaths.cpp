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

#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>
#include <QJniHelpers.h>
#include <QAndroidQPAPluginGap.h>
#include "QAndroidFilePaths.h"

static QMutex paths_mutex_(QMutex::Recursive);

const QLatin1String
	QAndroidFilePaths::ANDROID_DIRECTORY_MUSIC("Music"),
	QAndroidFilePaths::ANDROID_DIRECTORY_PODCASTS("Podcasts"),
	QAndroidFilePaths::ANDROID_DIRECTORY_RINGTONES("Ringtones"),
	QAndroidFilePaths::ANDROID_DIRECTORY_ALARMS("Alarms"),
	QAndroidFilePaths::ANDROID_DIRECTORY_NOTIFICATIONS("Notifications"),
	QAndroidFilePaths::ANDROID_DIRECTORY_PICTURES("Pictures"),
	QAndroidFilePaths::ANDROID_DIRECTORY_MOVIES("Movies"),
	QAndroidFilePaths::ANDROID_DIRECTORY_DOWNLOADS("Download"),
	QAndroidFilePaths::ANDROID_DIRECTORY_DCIM("DCIM"),
	QAndroidFilePaths::ANDROID_DIRECTORY_DOCUMENTS("Documents");

const QString & QAndroidFilePaths::ApplicationFilesDirectory()
{
	// Activity.getApplication().getFilesDir().getPath().
	QMutexLocker locker(&paths_mutex_);
	static QString path;
	if (path.isEmpty())
	{
		QJniObject activity(QAndroidQPAPluginGap::getActivity(), true);
		QScopedPointer<QJniObject> application(activity.callObject("getApplication", "android/app/Application"));
		QScopedPointer<QJniObject> filesdir(application->callObject("getFilesDir", "java/io/File"));
		path = filesdir->callString("getPath");
	}
	return path;
}

const QString & QAndroidFilePaths::ExternalFilesDirectory(const QString & type)
{
	// Activity.getExternalFilesDir().getPath().
	QMutexLocker locker(&paths_mutex_);
	static QString path;
	if (path.isEmpty())
	{
		QJniObject activity(QAndroidQPAPluginGap::getActivity(), true);
		QScopedPointer<QJniObject> externalfilesdir(activity.callParamObject(
				"getExternalFilesDir",
				"java/io/File",
				"Ljava/lang/String;",
				(type.isEmpty())? jstring(0): QJniLocalRef(type).operator jstring()));
		path = externalfilesdir->callString("getPath");
	}
	return path;
}

const QString & QAndroidFilePaths::ExternalStorageDirectory()
{
	// Environment.getExternalStorageDirectory().getPath().
	QMutexLocker locker(&paths_mutex_);
	static QString path;
	if (path.isEmpty())
	{
		QJniClass environment("android/os/Environment");
		QScopedPointer<QJniObject> externalstoragedir(environment.callStaticObject("getExternalStorageDirectory", "java/io/File"));
		path = externalstoragedir->callString("getPath");
	}
	return path;
}

const QString & QAndroidFilePaths::DownloadCacheDirectory()
{
	// Environment.getDownloadCacheDirectory().getPath().
	QMutexLocker locker(&paths_mutex_);
	static QString path;
	if (path.isEmpty())
	{
		QJniClass environment("android/os/Environment");
		QScopedPointer<QJniObject> externalstoragedir(environment.callStaticObject("getDownloadCacheDirectory", "java/io/File"));
		path = externalstoragedir->callString("getPath");
	}
	return path;
}

void QAndroidFilePaths::preloadJavaClasses()
{
	QAndroidQPAPluginGap::preloadJavaClass("android/os/Environment");

	/*qDebug()<<"QAndroidFilePaths"<<"APPDIR:"<<ApplicationFilesDirectory();
	qDebug()<<"QAndroidFilePaths"<<"EXTFDIR:"<<ExternalFilesDirectory();
	qDebug()<<"QAndroidFilePaths"<<"EXTSTORAGE:"<<ExternalStorageDirectory();
	qDebug()<<"QAndroidFilePaths"<<"DLCACHE:"<<DownloadCacheDirectory();*/
}


