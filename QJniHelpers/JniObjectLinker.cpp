/*
    QJniHelpers library for Qt

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

#include "JniObjectLinker.h"
#include <QAndroidQPAPluginGap.h>


JniObjectLinker::JniObjectLinker(void * nativePtr, const char * full_class_name, const JNINativeMethod * methods_list, size_t sizeof_methods_list) : 
	full_class_name_(full_class_name)
	, preloaded_(false)
{
	try
	{
		preloadJavaClasses(methods_list, sizeof_methods_list);
		// Creating Java object
		handler_.reset(new QJniObject(full_class_name, "J", jlong(nativePtr)));
	}
	catch (const std::exception & ex)
	{
		qCritical() << "Failed to preloadJavaClasses: " << ex.what();
	}
}


JniObjectLinker::~JniObjectLinker()
{
	if (handler_)
	{
		try
		{
			handler_->callVoid("cppDestroyed");
		}
		catch (const std::exception & ex)
		{
			qCritical() << "Failed to call cppDestroyed: " << ex.what();
		}
	}
}


QJniObject * JniObjectLinker::handler() const
{
	return handler_.data();
}


void JniObjectLinker::preloadJavaClasses(const JNINativeMethod * methods_list, size_t sizeof_methods_list)
{
	if (!preloaded_)
	{
		preloaded_ = true;

		QAndroidQPAPluginGap::preloadJavaClasses();
		QAndroidQPAPluginGap::preloadJavaClass(full_class_name_.constData());

		QJniClass ov(full_class_name_);
		ov.registerNativeMethods(methods_list, sizeof_methods_list);
	}
}
