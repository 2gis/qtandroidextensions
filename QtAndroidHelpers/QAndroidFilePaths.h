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

namespace QAndroidFilePaths
{
	/*!
	 * Activity.getApplication().getFilesDir().getPath().
	 * Typically the path looks like: "/data/data/package.name/files".
	 */
	const QString & ApplicationFilesDirectory();

	// Constants for ExternalFilesDirectory().
	extern const QLatin1String
		ANDROID_DIRECTORY_MUSIC,
		ANDROID_DIRECTORY_PODCASTS,
		ANDROID_DIRECTORY_RINGTONES,
		ANDROID_DIRECTORY_ALARMS,
		ANDROID_DIRECTORY_NOTIFICATIONS,
		ANDROID_DIRECTORY_PICTURES,
		ANDROID_DIRECTORY_MOVIES,
		ANDROID_DIRECTORY_DOWNLOADS,
		ANDROID_DIRECTORY_DCIM,
		ANDROID_DIRECTORY_DOCUMENTS;

	/*!
	 * Context.getExternalFilesDir().getPath().
	 * \param type - empty or null string, or one of ANDROID_DIRECTORY_... constants.
	 * For type == null, the path typically looks like: "/storage/emulated/0/Android/data/package.name/files".
	 */
	const QString & ExternalFilesDirectory(const QString & type = QString::null);

	/*!
	 * Context.getExternalFilesDirs()
	 * \param type - empty or null string, or one of ANDROID_DIRECTORY_... constants.
	 */
	const QStringList & ExternalFilesDirectories(const QString & type = QString::null);

	/*!
	 * Environment.getExternalStorageDirectory(type).getPath().
	 * Typically the path looks like: "/storage/emulated/0".
	 */
	const QString & ExternalStorageDirectory();

	/*!
	 * Environment.isExternalStorageEmulated()
	 */
	bool IsExternalStorageEmulated();

	/*!
	 * Environment.getDownloadCacheDirectory().getPath().
	 * Typically the path looks like: "/cache".
	 */
	const QString & DownloadCacheDirectory();

	/*!
	 * Context.getCacheDir().getPath().
	 */
	const QString & CacheDirectory();

	/*!
	 * Context.getExternalCacheDir().getPath().
	 */
	const QString & ExternalCacheDirectory();

	struct StatFs
	{
		quint64 total_bytes;
		quint64 available_bytes;
		quint64 free_bytes;
		quint64 block_size;

		StatFs():
			total_bytes(0),
			available_bytes(0),
			free_bytes(0),
			block_size(0)
		{
		}
	};

	StatFs GetStatFs(const QString & path);

	void preloadJavaClasses();

} // namespace QAndroidFilePaths


