/*
	Offscreen Android Views library for Qt

	Authors:
	Vyacheslav O. Koscheev <vok1980@gmail.com>
	Sergey A. Galin <sergey.galin@gmail.com>

	Distrbuted under The BSD License

	Copyright (c) 2017, DoubleGIS, LLC.
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

#include "QAndroidScreenLocker.h"

#include <QJniHelpers/QAndroidQPAPluginGap.h>
#include <QJniHelpers/TJniObjectLinker.h>


static const char * const c_full_class_name_ = "ru/dublgis/androidhelpers/WakeLocker";

static const JNINativeMethod methods[] =
{
	{"getContext", "()Landroid/content/Context;", reinterpret_cast<void*>(QAndroidQPAPluginGap::getCurrentContextNoThrow)},
};


JNI_LINKER_IMPL(QAndroidScreenLocker, c_full_class_name_, methods)


QAndroidScreenLocker & QAndroidScreenLocker::instance()
{
	static QAndroidScreenLocker obj;
	return obj;
}


QAndroidScreenLocker::QAndroidScreenLocker()
	: QLocks::QLockedObject(true)
	, jniLinker_(new JniObjectLinker(this))
{
	if (isJniReady())
	{
		// PowerManager.SCREEN_BRIGHT_WAKE_LOCK | PowerManager.ON_AFTER_RELEASE
		jni()->callVoid("setMode", static_cast<jint>(0x0000000a | 0x20000000));
	}
}


QAndroidScreenLocker::~QAndroidScreenLocker()
{
}


void QAndroidScreenLocker::lock()
{
	if (isJniReady())
	{
		jni()->callBool("Lock");
	}
}


void QAndroidScreenLocker::unlock()
{
	if (isJniReady())
	{
		jni()->callVoid("Unlock");
	}
}


bool QAndroidScreenLocker::isLocked()
{
	if (isJniReady())
	{
		return jni()->callBool("IsLocked");
	}
	return false;
}

