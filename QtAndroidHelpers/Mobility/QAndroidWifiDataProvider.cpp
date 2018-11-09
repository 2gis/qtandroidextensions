/*
	Offscreen Android Views library for Qt

	Authors:
	Vyacheslav O. Koscheev <vok1980@gmail.com>
	Ivan Avdeev marflon@gmail.com
	Sergey A. Galin sergey.galin@gmail.com

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

#include "QAndroidWifiDataProvider.h"
#include <QJniHelpers/QAndroidQPAPluginGap.h>
#include <QJniHelpers/TJniObjectLinker.h>


namespace Mobility {


Q_DECL_EXPORT void JNICALL Java_WifiListener_scanUpdate(JNIEnv *, jobject, jlong native_ptr)
{
	JNI_LINKER_OBJECT(Mobility::QAndroidWifiDataProvider, native_ptr, proxy)
	proxy->scanUpdate();
}


static const JNINativeMethod methods[] = {
	{"getContext", "()Landroid/content/Context;", reinterpret_cast<void*>(QAndroidQPAPluginGap::getCurrentContextNoThrow)},
	{"scanUpdate", "(J)V", reinterpret_cast<void*>(Java_WifiListener_scanUpdate)},
};


JNI_LINKER_IMPL(QAndroidWifiDataProvider, "ru/dublgis/androidhelpers/mobility/WifiListener", methods)


QAndroidWifiDataProvider::QAndroidWifiDataProvider(QObject * parent /*= 0*/)
	: QObject(parent)
	, jniLinker_(new JniObjectLinker(this))
	, started_(false)
{
}


QAndroidWifiDataProvider::~QAndroidWifiDataProvider()
{
}


void QAndroidWifiDataProvider::start()
{
	if (started_)
	{
		return;
	}

	if (isJniReady())
	{
		started_ = jni()->callBool("start");
	}
}


void QAndroidWifiDataProvider::stop()
{
	if (isJniReady())
	{
		jni()->callVoid("stop");
	}

	started_ = false;
}


void QAndroidWifiDataProvider::scanUpdate()
{
	emit signalsUpdated();
}


QString QAndroidWifiDataProvider::getSignalsData()
{
	if (isJniReady())
	{
		return jni()->callString("getLastWifiScanResultsTable");
	}

	return QString::null;
}


bool QAndroidWifiDataProvider::RetrieveData(WifiDataList & data)
{
	data.clear();
	QString table = getSignalsData();

	if(table.isEmpty())
	{
		return false;
	}

	QStringList tablelist = table.split(QChar('\n'), QString::SkipEmptyParts);
	
	if(!tablelist.size())
	{
		return false;
	}
	
	WifiData wd;
	
	for(int i=0; i<tablelist.size(); i++)
	{
		QStringList entry = tablelist.at(i).split(QChar('\t'), QString::KeepEmptyParts);

		if (entry.size() < 4)
		{
			qWarning() << "WiFi table entry is too short!";
			return false;
		}

		wd.StringAsMac(entry[0]);
		wd.signalStrength = entry.at(1).toInt();
		wd.name = entry[2];
		wd.timestamp_mks = entry.at(3).toInt();

		data.push_back(wd);
	}
	
	return true;
}

}
