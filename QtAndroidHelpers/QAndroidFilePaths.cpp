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
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QJniHelpers/QJniHelpers.h>
#include <QJniHelpers/QAndroidQPAPluginGap.h>
#include "QAndroidStorages.h"
#include "QAndroidFilePaths.h"


static QMutex paths_mutex_(QMutex::Recursive);
static const char * const c_directorieshelper_class_ = "ru/dublgis/androidhelpers/DirectoriesHelper";

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
		QAndroidQPAPluginGap::Context context;
		QScopedPointer<QJniObject> app_context(context.callObject("getApplicationContext", "android/content/Context"));
		QScopedPointer<QJniObject> filesdir(app_context->callObject("getFilesDir", "java/io/File"));
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
		QAndroidQPAPluginGap::Context activity;
		QScopedPointer<QJniObject> externalfilesdir(activity.callParamObject(
			"getExternalFilesDir",
			"java/io/File",
			"Ljava/lang/String;",
			(type.isEmpty())? jstring(0): QJniLocalRef(type).operator jstring()));
		path = externalfilesdir->callString("getPath");
	}
	return path;
}


const QStringList & QAndroidFilePaths::ExternalFilesDirectories(const QString & type)
{
	QMutexLocker locker(&paths_mutex_);
	static QStringList dirs;
	if (dirs.isEmpty())
	{
		if (QAndroidQPAPluginGap::apiLevel() < 19)
		{
			if (!type.isEmpty())
			{
				dirs.push_back(ExternalFilesDirectory(type));
			}
			else
			{
				// Using hack code to find external storage directories
				QString std_card_path = ExternalFilesDirectory(QString::null);
				dirs.push_back(std_card_path);
				QString package_name = QAndroidQPAPluginGap::Context().callString("getPackageName");
				const QStringList & storages = QAndroidStorages::externalStorages();
				for (int i = 0; i < storages.size(); ++i)
				{
					if (std_card_path.startsWith(storages.at(i) + QChar('/')))
					{
						continue;
					}
					QString full_path = storages.at(i) + QLatin1String("/Android/data/") + package_name + QLatin1String("/files");
					QDir().mkpath(full_path);
					if (!QFile::exists(full_path))
					{
						qWarning() << "[ExternalFilesDirectories] Failed to create directory:" << full_path;
						continue;
					}
					if (!QFileInfo(full_path).isWritable())
					{
						qWarning() << "[ExternalFilesDirectories] Directory exists but not writable:" << full_path;
						continue;
					}
					qDebug() << "[ExternalFilesDirectories] Adding external files path:" << full_path;
					dirs.push_back(full_path);
				}
			}
		}
		else
		{
			QString paths = QJniClass(c_directorieshelper_class_).callStaticParamString(
				"getExternalFilesDirs",
				"Landroid/content/Context;Ljava/lang/String;",
				QAndroidQPAPluginGap::Context().jObject(),
				(type.isEmpty())? jobject(0): QJniLocalRef(type).jObject());
			dirs = paths.split(QLatin1Char('\n'), QString::SkipEmptyParts);
			if (dirs.isEmpty())
			{
				qWarning() << "Failed to get dirs from getExternalFilesDirs(), falling back to getExternalFilesDir().";
				dirs.push_back(ExternalFilesDirectory(type));
			}
		}
	}
	return dirs;
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


bool QAndroidFilePaths::IsExternalStorageEmulated()
{
	QMutexLocker locker(&paths_mutex_);
	static bool s_is_emulated = false;
	static bool s_read = false;
	try
	{
		if (!s_read)
		{
			s_read = true;
			if (QAndroidQPAPluginGap::apiLevel() >= 11)
			{
				s_is_emulated = QJniClass("android/os/Environment").callStaticBoolean("isExternalStorageEmulated");
			}
		}
	}
	catch (const std::exception & e)
	{
		qCritical() << "IsExternalStorageEmulated exception:" << e.what();
	}
	return s_is_emulated;
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


const QString & QAndroidFilePaths::ExternalCacheDirectory()
{
	// Activity.getExternalCacheDir().getPath().
	QMutexLocker locker(&paths_mutex_);
	static QString path;
	if (path.isEmpty())
	{
		QAndroidQPAPluginGap::Context activity;
		QScopedPointer<QJniObject> externalfilesdir(activity.callObject(
			"getExternalCacheDir",
			"java/io/File"));
		path = externalfilesdir->callString("getPath");
	}
	return path;
}


const QString & QAndroidFilePaths::CacheDirectory()
{
	// Activity.getExternalCacheDir().getPath().
	QMutexLocker locker(&paths_mutex_);
	static QString path;
	if (path.isEmpty())
	{
		QAndroidQPAPluginGap::Context activity;
		QScopedPointer<QJniObject> externalfilesdir(activity.callObject(
			"getCacheDir",
			"java/io/File"));
		path = externalfilesdir->callString("getPath");
	}
	return path;
}


QAndroidFilePaths::StatFs QAndroidFilePaths::GetStatFs(const QString & path)
{
	QAndroidFilePaths::StatFs result;
	if (path.isEmpty())
	{
		qWarning() << "GetStatFs() called with empty path.";
		return result;
	}
	try
	{
		QJniObject stat("android/os/StatFs", "Ljava/lang/String;", QJniLocalRef(path).jObject());
		quint64 avail_blocks = 0, total_blocks = 0, free_blocks = 0, block_size = 0;
		if (QAndroidQPAPluginGap::apiLevel() >= 18)
		{
			avail_blocks = static_cast<quint64>(stat.callLong("getAvailableBlocksLong"));
			total_blocks = static_cast<quint64>(stat.callLong("getBlockCountLong"));
			free_blocks = static_cast<quint64>(stat.callLong("getFreeBlocksLong"));
			block_size = static_cast<quint64>(stat.callLong("getBlockSizeLong"));
		}
		else
		{
			avail_blocks = static_cast<quint64>(stat.callInt("getAvailableBlocks"));
			total_blocks = static_cast<quint64>(stat.callInt("getBlockCount"));
			free_blocks = static_cast<quint64>(stat.callInt("getFreeBlocks"));
			block_size = static_cast<quint64>(stat.callInt("getBlockSize"));
		}
		result.total_bytes = block_size * total_blocks;
		result.available_bytes = block_size * avail_blocks;
		result.free_bytes = block_size * free_blocks;
		result.block_size = block_size;
	}
	catch (std::exception & e)
	{
		qWarning() << "Failed to stat FS:" << path << "Exception:" << e.what();
	}
	return result;
}


void QAndroidFilePaths::preloadJavaClasses()
{
	static bool s_preloaded = false;
	if (!s_preloaded)
	{
		s_preloaded = true;
		QAndroidQPAPluginGap::preloadJavaClasses();
		QAndroidQPAPluginGap::preloadJavaClass("android/os/Environment");
		QAndroidQPAPluginGap::preloadJavaClass("android/os/StatFs");
		QAndroidQPAPluginGap::preloadJavaClass("android/content/Context");
		QAndroidQPAPluginGap::preloadJavaClass(c_directorieshelper_class_);
	}
}


