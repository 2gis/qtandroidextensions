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
#include <QAndroidQPAPluginGap.h>


Q_DECL_EXPORT void JNICALL Java_ScreenLayoutHandler_globalLayoutChanged(JNIEnv *, jobject, jlong param)
{
	if (param)
	{
		void * vp = reinterpret_cast<void*>(param);
		QAndroidScreenLayoutHandler * proxy = reinterpret_cast<QAndroidScreenLayoutHandler*>(vp);
		if (proxy)
		{
		  proxy->javaGlobalLayoutChanged();
		  return;
		}
	}
	qWarning()<<__FUNCTION__<<"Zero param!";
}


static const char *c_full_class_name_ = "ru/dublgis/androidhelpers/ScreenLayoutHandler";

QAndroidScreenLayoutHandler::QAndroidScreenLayoutHandler(QObject * parent)
	: QObject(parent)
{
	preloadJavaClasses();

	// Creating Java object
	screen_layout_handler_.reset(new QJniObject(c_full_class_name_, "J",
		jlong(reinterpret_cast<void*>(this))));
}

QAndroidScreenLayoutHandler::~QAndroidScreenLayoutHandler()
{
	if (screen_layout_handler_)
	{
		screen_layout_handler_->callVoid("cppDestroyed");
		screen_layout_handler_.reset();
	}
}

void QAndroidScreenLayoutHandler::subscribeToLayoutEvents()
{
	if (screen_layout_handler_)
	{
		screen_layout_handler_->callVoid("subscribeToLayoutEvents");
	}
}

void QAndroidScreenLayoutHandler::unsubscribeFromLayoutEvents()
{
	if (screen_layout_handler_)
	{
		screen_layout_handler_->callVoid("unsubscribeFromLayoutEvents");
	}
}

void QAndroidScreenLayoutHandler::javaGlobalLayoutChanged()
{
	emit globalLayoutChanged();
}

void QAndroidScreenLayoutHandler::preloadJavaClasses()
{
	static volatile bool preloaded_ = false;

	if (!preloaded_)
	{
		preloaded_ = true;

		QAndroidQPAPluginGap::preloadJavaClass(c_full_class_name_);

		QJniClass ov(c_full_class_name_);
		static const JNINativeMethod methods[] = {
			{"getActivity", "()Landroid/app/Activity;", (void*)QAndroidQPAPluginGap::getActivity},
			{"nativeGlobalLayoutChanged", "(J)V", (void*)Java_ScreenLayoutHandler_globalLayoutChanged},
		};
		ov.registerNativeMethods(methods, sizeof(methods));
	}
}
