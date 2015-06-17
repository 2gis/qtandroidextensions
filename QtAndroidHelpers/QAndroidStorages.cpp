/*
  Lightweight access to various Android APIs for Qt

  Authors:
  Alexander A. Saytgalin <a.saytgalin@2gis.com>
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

#include <android/log.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <QtCore/qdebug.h>
#include <QtCore/QStringList>
#include "QAndroidFilePaths.h"
#include "QAndroidStorages_p.h"
#include "QAndroidStorages.h"

namespace QAndroidStorages {

bool MountEntry::IsExternalStorage() const
{
    const char * const dev_prefix = "/dev/";
    return 0 == dev.indexOf(dev_prefix);
}

} // namespace QAndroidStorages

QTextStream& operator>> (QTextStream &stream, QAndroidStorages::MountEntry &mount_entry)
{
    stream >> mount_entry.dev;

    QString mount_point;
    stream >> mount_point;
    mount_entry.MountPoint(mount_point);

    stream >> mount_entry.fs;
    stream >> mount_entry.options;
    stream >> mount_entry.dump;
    stream >> mount_entry.pass;

    return stream;
}

QTextStream& operator>> (QTextStream &stream, QAndroidStorages::VoldEntry &vold_entry)
{
    const char * const media_type_name = "media_type";
    const char * const mount_point_name = "mount_point";
    const char comment = '#';
    const char * const open_br = "{";
    const char * const close_br = "}";

    // Read caption
    {
        QString line;
        // Skip empty lines and comments.
        do {
            line = stream.readLine().trimmed();
        } while(!stream.atEnd() && (line.isEmpty() || comment == line[0]));

        if (stream.atEnd() || QTextStream::Ok != stream.status())
        {
            return stream;
        }

        QTextStream stream_line(&line);
        stream_line >> vold_entry.caption;
        QString open;
        stream_line >> open;
        if (open_br != open.trimmed())
        {
            stream.setStatus(QTextStream::ReadCorruptData);
            return stream;
        }
    }

    // Read data
    for(;;)
    {
        QString line;
        // Skip empty lines and comments.
        do {
            line = stream.readLine().trimmed();
        } while(!stream.atEnd() && (line.isEmpty() || comment == line[0]));

        if (QTextStream::Ok != stream.status())
        {
            return stream;
        }

        if (close_br == line)
        {
            return stream;
        }

        QTextStream stream_line(&line);
        QString var;
        QString value;

        stream_line >> var >> value;

        var = var.trimmed();
        value = value.trimmed();

        if (media_type_name == var)
        {
            vold_entry.media_type = value;
        }
        else if (mount_point_name == var)
        {
            vold_entry.MountPoint(value);
        }
    }

    return stream;
}

namespace QAndroidStorages {

// Function gives a list of external storages.
const QStringList & externalStorages()
{
    static bool need_update(true);
    static QStringList ret;

    // The file'/etc/vold.conf' ('/etc/vold.fstab') doesn't change. Need to read once.
    if (!need_update)
    {
        return ret;
    }

    // Will try to read mount tables from one of these files:
    const char * const vold_conf_file_name = "/etc/vold.conf";
    const char * const vold_fstab_file_name = "/etc/vold.fstab";
    const char * const mount_file_name = "/proc/mounts";

    QAndroidStorages::DevEntryLoader loader;
    // Load devices from the file '/etc/vold.conf'. File format is 'VoldEntry'.
    QSet<QString> devs = loader.Load<QAndroidStorages::VoldEntry>(vold_conf_file_name);

    if (devs.isEmpty())
    {
        // Some devices don't contain the file '/etc/vold.conf' (it containts the file '/etc/vold.fstab'?).
        devs = loader.Load<QAndroidStorages::VoldEntry>(vold_fstab_file_name);
    }

    if (!devs.isEmpty())
    {
        // The file'/etc/vold.conf' ('/etc/vold.fstab') doesn't change.
        need_update = false;
    }
    else
    {
        // Load devices from the universal file '/proc/mounts'. File format is 'MountEntry'.
        devs = loader.Load<QAndroidStorages::MountEntry>(mount_file_name);
    }

    // Add the primary storage directory (if necessary).
    const QString & external_storage = QAndroidFilePaths::ExternalStorageDirectory();
    devs << external_storage; // (NB: devs is a set, so this won't cause a duplicate)

    // Blacklist
    const char * const blacklist[] = {
        "/", "/acct", "/cache", "/config", "/data", "/dev", "/etc", "/init", "/mnt",
        "/pds", "/proc", "/root", "/sbin", "/sys", "/system", "/vendor", 0
    };
    // Std paths for external SD card which are not listed in fstab on some stupid devices.
    // Note that the first SD card does not need to be listed as it is always returned
    // by qt_android_get_external_storage_path().
    const char * const standard_paths[] = {
        "/mnt/sdcard1", "/mnt/extSdCard", "/sdcard/external_sd", // Samsung
        "/storage/sdcard1", // Samsung (?), Lenovo
        "/sdcard2", "/sdcard/removable_sdcard", // Lenovo
        "/mnt/extsd", // TeXet
        "/mnt/external1", "/mnt/external2", // Xoom
        "/storage/external_SD",
        0
    };

    // Apply blacklist
    for (const char * const * p = blacklist; *p; ++p)
        devs.remove(*p);
    // Apply stupid std path list
    for (const char * const * p = standard_paths; *p; ++p)
        devs.insert(*p); // Non-existent paths and symlinks will be removed later

    // Resolve symlinks, because otherwise there can be dublicates,
    // like /sdcard and /mnt/sdcard one of which is a symlink to the other.
    // Also check the mount points for existence.
    QMap<QString, QString> symlinks;
    QSet<QString> nonexistent;
    for (QSet<QString>::iterator it = devs.begin(); it != devs.end(); ++it) {
        if (!QFile::exists(*it)) { // Path does not exists
            nonexistent.insert(*it);
            continue;
        }
        char buf[PATH_MAX+1] = {0}; // Output for readlink()
        // (Returns -1 on error)
        ssize_t sz = readlink(it->toUtf8(), buf, sizeof(buf));
        if (sz > 0) { // This is a symlink then
            QString real_file = QString::fromUtf8(buf);
            symlinks.insert(*it, real_file);
            qDebug()<<"Storage symlink:"<<(*it)<<"=>"<<real_file;
        } // else it's not a symlink, nothing to do here, move along
    }
    // Replace symlinks with real paths.
    // NB: for sdcard, this shouldn't replace the one returned by qt_android_get_external_storage_path()
    // because Android typically reports real path there (not the symlink).
    for (QMap<QString, QString>::iterator it = symlinks.begin(); it != symlinks.end(); ++it) {
        devs.remove(it.key());
        devs.insert(it.value());
    }
    // Remove non-existent paths
    for (QSet<QString>::iterator it = nonexistent.begin(); it != nonexistent.end(); ++it)
    {
        devs.remove(*it);
    }

    ret = devs.toList();

    qDebug()<<"Found storages:"<<ret.join(", ");

    return ret;
}

} // namespace QAndroidStorages