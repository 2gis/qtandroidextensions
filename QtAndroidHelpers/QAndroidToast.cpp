/*
  Lightweight access to various Android APIs for Qt

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

#include "QAndroidToast.h"
#include <QJniHelpers/QAndroidQPAPluginGap.h>


static const char * const c_full_class_name_ = "ru/dublgis/androidhelpers/ToastHelper";

namespace QAndroidToast {

void showToast(const QString & text, bool length_long)
{
	try
	{
		QJniClass(c_full_class_name_).callStaticParamVoid(
			"showToast",
			"Landroid/content/Context;Ljava/lang/String;I",
			QAndroidQPAPluginGap::Context().jObject(),
			QJniLocalRef(text).jObject(),
			jint((length_long)? ANDROID_TOAST_LENGTH_LONG: ANDROID_TOAST_LENGTH_SHORT));
	}
	catch (const std::exception & e)
	{
		qCritical() << "JNI exception in showToast: " << e.what();
	}
}

void preloadJavaClasses()
{
	static bool s_preloaded = false;
	if (!s_preloaded)
	{
		try
		{
			QAndroidQPAPluginGap::preloadJavaClass(c_full_class_name_);
		}
		catch (const std::exception & e)
		{
			qCritical() << "JNI exception in QAndroidToast::preloadJavaClasses: " << e.what();
		}
		s_preloaded = true;
	}
}

} // namespace QAndroidToast

