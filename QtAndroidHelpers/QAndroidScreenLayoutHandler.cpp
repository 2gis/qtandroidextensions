/*
  Offscreen Android Views library for Qt

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

#include "QAndroidScreenLayoutHandler.h"
#include <QJniHelpers/QAndroidQPAPluginGap.h>
#include <QJniHelpers/TJniObjectLinker.h>


Q_DECL_EXPORT void JNICALL Java_ScreenLayoutHandler_globalLayoutChanged(JNIEnv *, jobject, jlong param)
{
	JNI_LINKER_OBJECT(QAndroidScreenLayoutHandler, param, obj)
	obj->javaGlobalLayoutChanged();
}


Q_DECL_EXPORT void JNICALL Java_ScreenLayoutHandler_scrollChanged(JNIEnv *, jobject, jlong param)
{
	JNI_LINKER_OBJECT(QAndroidScreenLayoutHandler, param, obj)
	obj->javaScrollChanged();
}

Q_DECL_EXPORT void JNICALL Java_ScreenLayoutHandler_keyboardHeightChanged(JNIEnv *, jobject, jlong param, jint height)
{
	JNI_LINKER_OBJECT(QAndroidScreenLayoutHandler, param, obj)
	obj->javaKeyboardHeightChanged(height);
}

static const char * const c_full_class_name_ = "ru/dublgis/androidhelpers/ScreenLayoutHandler";

static const JNINativeMethod methods[] = {
	{"getActivity", "()Landroid/app/Activity;", reinterpret_cast<void*>(QAndroidQPAPluginGap::getActivityNoThrow)},
	{"nativeGlobalLayoutChanged", "(J)V", reinterpret_cast<void*>(Java_ScreenLayoutHandler_globalLayoutChanged)},
	{"nativeScrollChanged", "(J)V", reinterpret_cast<void*>(Java_ScreenLayoutHandler_scrollChanged)},
	{"nativeKeyboardHeightChanged", "(JI)V", reinterpret_cast<void*>(Java_ScreenLayoutHandler_keyboardHeightChanged)},
};


JNI_LINKER_IMPL(QAndroidScreenLayoutHandler, c_full_class_name_, methods)


QAndroidScreenLayoutHandler::QAndroidScreenLayoutHandler(QObject * parent)
	: QObject(parent)
	, jniLinker_(new JniObjectLinker(this))
{
}


QAndroidScreenLayoutHandler::~QAndroidScreenLayoutHandler()
{
}


void QAndroidScreenLayoutHandler::subscribeToLayoutEvents()
{
	if (isJniReady())
	{
		jni()->callVoid("subscribeToLayoutEvents");
	}
}


void QAndroidScreenLayoutHandler::unsubscribeFromLayoutEvents()
{
	if (isJniReady())
	{
		jni()->callVoid("unsubscribeFromLayoutEvents");
	}
}


void QAndroidScreenLayoutHandler::javaGlobalLayoutChanged()
{
	emit globalLayoutChanged();
}


void QAndroidScreenLayoutHandler::javaScrollChanged()
{
	emit scrollChanged();
}


void QAndroidScreenLayoutHandler::javaKeyboardHeightChanged(int height)
{
	emit keyboardHeightChanged(height);
}
