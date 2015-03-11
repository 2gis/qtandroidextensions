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

#include "QAndroidScreenLocker.h"

#include <QAndroidQPAPluginGap.h>


static const char * const c_full_class_name_ = "ru/dublgis/androidhelpers/ScreenLocker";


QAndroidScreenLocker& QAndroidScreenLocker::instance()
{
	static QAndroidScreenLocker obj;
	return obj;
}


QAndroidScreenLocker::QAndroidScreenLocker() 
{
	preloadJavaClasses();

	// Creating Java object
	java_handler_.reset(new QJniObject(c_full_class_name_, "J",
		jlong(reinterpret_cast<void*>(this))));
}


QAndroidScreenLocker::~QAndroidScreenLocker()
{
	if (java_handler_)
	{
		java_handler_->callVoid("cppDestroyed");
		java_handler_.reset();
	}
}


void QAndroidScreenLocker::preloadJavaClasses()
{
	static volatile bool preloaded_ = false;

	if (!preloaded_)
	{
		preloaded_ = true;

		QAndroidQPAPluginGap::preloadJavaClasses();
		QAndroidQPAPluginGap::preloadJavaClass(c_full_class_name_);

		QJniClass ov(c_full_class_name_);
		static const JNINativeMethod methods[] = 
		{
			{"getContext", "()Landroid/content/Context;", (void*)QAndroidQPAPluginGap::getCurrentContext},
		};
	 
		ov.registerNativeMethods(methods, sizeof(methods));
	}
}


void QAndroidScreenLocker::lock()
{
	java_handler_->callBool("Lock");
	Q_ASSERT(isLocked());
}


void QAndroidScreenLocker::unlock()
{
	java_handler_->callVoid("Unlock");
}


bool QAndroidScreenLocker::isLocked()
{
	return java_handler_->callBool("IsLocked");	
}

