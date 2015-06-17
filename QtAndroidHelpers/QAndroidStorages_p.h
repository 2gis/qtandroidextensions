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

#pragma once
#include <jni.h>
#include <QtCore/QSet>
#include <QtCore/QString>
#include <QtCore/QTextStream>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>

// Work with a list of external storages.
// Forvard declaration.
namespace QAndroidStorages {

// Abstract external device.
class DevEntry;
// Device described in '/proc/mounts'.
class MountEntry;
// Device described in '/etc/vold.conf'.
class VoldEntry;

} // namespace QAndroidStorages

QTextStream& operator>> (QTextStream &stream, QAndroidStorages::MountEntry &mount_entry);
QTextStream& operator>> (QTextStream &stream, QAndroidStorages::VoldEntry &vold_entry);

namespace QAndroidStorages {

// Abstract external device.
class DevEntry
{
public:
    DevEntry() {}
    virtual ~DevEntry() {}

    const QString& MountPoint() const { return mount_point; }
    virtual bool IsExternalStorage() const = 0;

protected:
    void MountPoint(const QString &mount_point) { this->mount_point = mount_point; }

private:
    QString mount_point; // The mount point, where the data is to be attached to the filesystem.
};


// Device described in '/proc/mounts'.
class MountEntry : public DevEntry
{
    friend QTextStream& ::operator>> (QTextStream &stream, QAndroidStorages::MountEntry &mount_entry);

public:
    MountEntry() : DevEntry() {}
    virtual ~MountEntry() {}

public: // DevEntry
    virtual bool IsExternalStorage() const;

private:
    QString dev;        // The device name or other means of locating the partition or data source.
    QString fs;         // The filesystem type, or the algorithm used to interpret the filesystem.
    QString options;    // Options, including if the filesystem should be mounted at boot.
    QString dump;       // dump-freq adjusts the archiving schedule for the partition.
    QString pass;       // pass-num Controls the order in which fsck checks the device/partition for errors at boot time.
};


// Device described in '/etc/vold.conf'.
class VoldEntry : public DevEntry
{
    friend QTextStream& ::operator>> (QTextStream &stream, VoldEntry &mount_entry);

public:
    VoldEntry() : DevEntry() {}
    virtual ~VoldEntry() {}

public: // DevEntry
    virtual bool IsExternalStorage() const { return true; }

private:
    QString caption;    // Volume caption.
    QString media_type; // Device type.
};


// DevEntry's loader from a file 'dev_file_name'.
class DevEntryLoader
{
public:
    // The method returns a set of external devices from the file 'dev_file_name', format record is 'Entry'.
    template<class Entry>
    QSet<QString> Load(const QString &dev_file_name)
    {
        QSet<QString> ret;

        QFile dev_file(dev_file_name);
        if (!dev_file.open(QIODevice::ReadOnly))
        {
            // If file 'dev_file_name' can not be read then return empty the set of external devices.
            return ret;
        }

        QTextStream stream(&dev_file);
        while (!stream.atEnd() && QTextStream::Ok == stream.status())
        {
            // Read each devece entry from 'dev_entry'.
            Entry dev_entry;
            stream >> dev_entry;

            if (QTextStream::Ok != stream.status())
            {
                // If format 'Entry' is incorrect then return empty the set of external devices and exit.
                ret.clear();
                break;
            }

            if (dev_entry.MountPoint().isEmpty())
            {
                break;
            }

            if (dev_entry.IsExternalStorage() && QFileInfo(dev_entry.MountPoint()).isWritable())
            {
                ret << dev_entry.MountPoint();
            }
        }
        dev_file.close();

        return ret;
    }
};

} // namespace QAndroidStorages
