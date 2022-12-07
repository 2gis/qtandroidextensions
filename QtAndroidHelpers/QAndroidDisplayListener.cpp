/*
  Lightweight access to various Android APIs for Qt

  Author:
  Sergey A. Galin <sergey.galin@gmail.com>

  Distrbuted under The BSD License

  Copyright (c) 2022, DoubleGIS, LLC.
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

#include "QAndroidDisplayListener.h"
#include <QJniHelpers/QAndroidQPAPluginGap.h>


namespace {

const char * const c_listener_class_name = "ru/dublgis/androidhelpers/DisplayListener";

} // anonymous namespace


QAndroidDisplayListener::QAndroidDisplayListener(QObject * parent)
	: QObject(parent)
{
	try
	{
		preloadJavaClasses();
		mJavaListener = QJniObject(c_listener_class_name);
		if (mJavaListener)
		{
			mJavaListener.callParamVoid(
				"register",
				"Landroid/content/Context;J",
				QAndroidQPAPluginGap::Context().jObject(),
				reinterpret_cast<jlong>(this));
		}
		else
		{
			qCritical() << "Failed to create DisplayListener instance!";
		}
	}
	catch (const std::exception & e)
	{
		qCritical() << "JNI exception in QAndroidDisplayListener constructor: " << e.what();
	}
}


QAndroidDisplayListener::~QAndroidDisplayListener()
{
	try
	{
		if (mJavaListener)
		{
			mJavaListener.callVoid("unregister");
		}
	}
	catch (const std::exception & e)
	{
		qCritical() << "JNI exception in QAndroidDisplayListener destructor: " << e.what();
	}
}


void QAndroidDisplayListener::preloadJavaClasses()
{
	static bool s_preloaded = false;
	if (!s_preloaded)
	{
		try
		{
			QAndroidQPAPluginGap::preloadJavaClass(c_listener_class_name);
			QJniClass clz(c_listener_class_name);
			static const JNINativeMethod methods[] = {
				{"nativeDisplayAdded", "(JI)V", reinterpret_cast<void*>(QAndroidDisplayListener::javaDisplayAdded)},
				{"nativeDisplayRemoved", "(JI)V", reinterpret_cast<void*>(QAndroidDisplayListener::javaDisplayRemoved)},
				{"nativeDisplayChanged", "(JI)V", reinterpret_cast<void*>(QAndroidDisplayListener::javaDisplayChanged)},
			};
			clz.registerNativeMethods(methods, sizeof(methods));
			s_preloaded = true;
		}
		catch (const std::exception & e)
		{
			qCritical() << "JNI exception in QAndroidDisplayListener::preloadJavaClasses: " << e.what();
		}
	}
}


void QAndroidDisplayListener::javaDisplayAdded(JNIEnv *, jobject, jlong ptr, jint id)
{
	if (auto self = reinterpret_cast<QAndroidDisplayListener*>(ptr))
	{
		QMetaObject::invokeMethod(self, [self, id] { emit self->displayAdded(static_cast<int>(id)); });
	}
}


void QAndroidDisplayListener::javaDisplayRemoved(JNIEnv *, jobject, jlong ptr, jint id)
{
	if (auto self = reinterpret_cast<QAndroidDisplayListener*>(ptr))
	{
		QMetaObject::invokeMethod(self, [self, id] { emit self->displayRemoved(static_cast<int>(id)); });
	}
}


void QAndroidDisplayListener::javaDisplayChanged(JNIEnv *, jobject, jlong ptr, jint id)
{
	if (auto self = reinterpret_cast<QAndroidDisplayListener*>(ptr))
	{
		QMetaObject::invokeMethod(self, [self, id] { emit self->displayChanged(static_cast<int>(id)); });
	}
}


