/*
	Offscreen Android Views library for Qt

	Author:
	Vyacheslav O. Koscheev <vok1980@gmail.com>

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

#include "QAndroidWiFiLocker.h"

#include <QJniHelpers/QAndroidQPAPluginGap.h>
#include <QJniHelpers/TJniObjectLinker.h>


static const char * const c_full_class_name_ = "ru/dublgis/androidhelpers/WifiLocker";


static const JNINativeMethod methods[] =
{
	{"getContext", "()Landroid/content/Context;", reinterpret_cast<void*>(QAndroidQPAPluginGap::getCurrentContextNoThrow)},
};


JNI_LINKER_IMPL(QAndroidWiFiLocker, c_full_class_name_, methods)


QAndroidWiFiLocker & QAndroidWiFiLocker::instance()
{
	static QAndroidWiFiLocker obj;
	return obj;
}


QAndroidWiFiLocker::QAndroidWiFiLocker()
	: QLocks::QLockedObject(false)
	, jniLinker_(new JniObjectLinker(this))
{
}


QAndroidWiFiLocker::~QAndroidWiFiLocker()
{
}


void QAndroidWiFiLocker::lock()
{
	if (isJniReady())
	{
		jni()->callBool("Lock");
	}
}


void QAndroidWiFiLocker::unlock()
{
	if (isJniReady())
	{
		jni()->callVoid("Unlock");
	}
}


bool QAndroidWiFiLocker::isLocked()
{
	if (isJniReady())
	{
		return jni()->callBool("IsLocked");
	}

	return false;
}


bool QAndroidWiFiLocker::isWifiEnabled()
{
	if (isJniReady())
	{
		return jni()->callBool("IsWifiEnabled");
	}

	return false;
}

