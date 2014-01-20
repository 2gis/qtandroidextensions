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
#include <qconfig.h>
#include <QScopedPointer>
#include <JniEnvPtr.h>
#include <jcGeneric.h>
#include <JclassPtr.h>
#include "QAndroidQPAPluginGap.h"

namespace QAndroidQPAPluginGap {

jobject JNICALL getActivity(JNIEnv *, jobject)
{
	#if QT_VERSION < 0x050000 && defined(QTOFFSCREENVIEWS_GRYM)
		static const char * const c_class_name = "org/qt/core/QtApplicationBase";
		static const char * const c_method_name = "getActivityStatic";
		// It is OK to return QtActivityBase as it's a descendant of Activity and
		// Java will handle the type casting.
		static const char * const c_result_name = "org/qt/core/QtActivityBase";
	#else // QT_VERSION >= 0x050000
		//! \todo If we make another plugin for Qt 5, this place will need an update!
		static const char * const c_class_name = "org/qtproject/qt5/android/QtNative";
		static const char * const c_method_name = "activity";
		static const char * const c_result_name = "android/app/Activity";
	#endif
	jcGeneric theclass(c_class_name, false);
	if (!theclass.jClass())
	{
		qCritical("QAndroid: Activity retriever class could not be accessed.");
		return 0;
	}
	QScopedPointer<jcGeneric> activity(theclass.CallStaticObject(c_method_name, c_result_name));
	if (!activity)
	{
		qCritical("QAndroid: Failed to get Activity object.");
		return 0;
	}
	if (!activity->jObject())
	{
		qCritical("QAndroid: Java instance of the Activity is 0.");
	}
	// Take jobject away from 'activity' because otherwise it will destroy
	// the global reference and it will become invalid when we return to Java.
	return activity->TakeJobjectOver();
}

} // namespace QAndroidQPAPluginGap
