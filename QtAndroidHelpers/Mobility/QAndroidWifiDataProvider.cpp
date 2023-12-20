/*
	Offscreen Android Views library for Qt

	Authors:
	Vyacheslav O. Koscheev <vok1980@gmail.com>
	Ivan Avdeev marflon@gmail.com
	Sergey A. Galin sergey.galin@gmail.com

	Distrbuted under The BSD License

	Copyright (c) 2015-2024, DoubleGIS, LLC.
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
#include <QJniHelpers/QJniHelpers.h>


namespace Mobility {


Q_DECL_EXPORT void JNICALL Java_WifiListener_scanUpdate(JNIEnv *, jobject, jlong native_ptr)
{
	JNI_LINKER_OBJECT(Mobility::QAndroidWifiDataProvider, native_ptr, proxy)
	proxy->scanUpdate();
}


Q_DECL_EXPORT void JNICALL Java_WifiListener_setScanResult(JNIEnv *, jobject, jlong native_ptr, jobject scan_result)
{
	if (0x0 == scan_result)
	{
		return;
	}

	JNI_LINKER_OBJECT(Mobility::QAndroidWifiDataProvider, native_ptr, proxy)
	proxy->setScanResult(scan_result);
}


static const JNINativeMethod methods[] = {
	{"getContext", "()Landroid/content/Context;", reinterpret_cast<void*>(QAndroidQPAPluginGap::getCurrentContextNoThrow)},
	{"scanUpdate", "(J)V", reinterpret_cast<void*>(Java_WifiListener_scanUpdate)},
	{"setScanResult", "(JLandroid/net/wifi/ScanResult;)V", reinterpret_cast<void*>(Java_WifiListener_setScanResult)},
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


bool QAndroidWifiDataProvider::getSignalsData()
{
	if (isJniReady())
	{
		return jni()->callBool("getLastWifiScanResultsTable");
	}

	return false;
}


void QAndroidWifiDataProvider::setScanResult(jobject scan_result)
{
	try
	{
		WifiData wd;
		QJniObject result(scan_result, false);

		wd.StringAsMac(result.getStringField("BSSID"));
		wd.signalStrength =result.getIntField("level");
		
		if (QAndroidQPAPluginGap::apiLevel() < 33)
		{
			wd.name = result.getStringField("SSID");
		}
		else
		{
			wd.name = result.callObj("getWifiSsid", "android/net/wifi/WifiSsid").callString("toString");
		}

		if (QAndroidQPAPluginGap::apiLevel() >= 17)
		{
			wd.timestamp_mks = result.getLongField("timestamp");
			qint64 elapsed_realtime_ms = QJniClass("android/os/SystemClock").callStaticLong("elapsedRealtime");
			wd.since_signal_ms = elapsed_realtime_ms - wd.timestamp_mks / 1000;
		}
		else
		{
			wd.timestamp_mks = 0;
			wd.since_signal_ms = 0;
		}

		data_buf_.push_back(wd);
	}
	catch (const std::exception & e)
	{
		qWarning() << "Exception: " << e.what();
	}
}


bool QAndroidWifiDataProvider::RetrieveData(WifiDataList & data)
{
	data.clear();
	data_buf_.clear();

	if (!getSignalsData())
	{
		return false;
	}

	data.swap(data_buf_);

	return true;
}

}
