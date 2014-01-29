/*
  QJniHelpers library

  Authors:
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
#include <QDebug>
#include <QScopedPointer>
#include <QJniHelpers.h>

#if QT_VERSION < 0x050000 && defined(QJNIHELPERS_GRYM)
	#define QPA_QT4GRYM
#elif QT_VERSION >= 0x050000
	#define QPA_QT5
#else
	#error "Unimplemented QPA case"
#endif

#if defined(QPA_QT5)
	#include <QtAndroidExtras>
#endif

#include "QAndroidQPAPluginGap.h"

#if defined(Q_OS_ANDROID)

#if defined(QPA_QT4GRYM)
	extern JavaVM * qt_android_get_java_vm();
#elif QT_VERSION >= 0x050000
	#include <QAndroidJniEnvironment>
#else
	#error "Unimplemented QPA case"
#endif

namespace QAndroidQPAPluginGap {

JavaVM * detectJavaVM()
{
	#if defined(QPA_QT4GRYM)
		return qt_android_get_java_vm();
	#elif defined(QPA_QT5)
		return QAndroidJniEnvironment::javaVM();
	#else
		#error "Unimplemented QPA case"
	#endif
}

jobject JNICALL getActivity(JNIEnv *, jobject)
{
	#if defined(QPA_QT4GRYM)
		static const char * const c_class_name = "org/qt/core/QtApplicationBase";
		static const char * const c_method_name = "getActivityStatic";
		// It is OK to return QtActivityBase as it's a descendant of Activity and
		// Java will handle the type casting.
		static const char * const c_result_name = "org/qt/core/QtActivityBase";
	#elif defined(QPA_QT5)
		//! \todo If we make another plugin for Qt 5, this place will need an update!
		static const char * const c_class_name = "org/qtproject/qt5/android/QtNative";
		static const char * const c_method_name = "activity";
		static const char * const c_result_name = "android/app/Activity";
	#else
		#error "Unimplemented QPA case"
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
		qCritical("QAndroid: Failed to get create Activity object.");
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

void preloadJavaClass(const char * class_name)
{
	qDebug()<<__PRETTY_FUNCTION__<<class_name;
	JniEnvPtr jep;
	if (jep.IsClassPreloaded(class_name))
	{
		qDebug()<<"Class already pre-loaded:"<<class_name;
		return;
	}
	qDebug()<<"Pre-loading:"<<class_name;

	static const char * const c_class_name = "ru/dublgis/qjnihelpers/ClassLoader";
	static const char * const c_method_name = "callJNIPreloadClass";
	#if defined(QPA_QT4GRYM)
		jcGeneric(c_class_name, false).CallStaticVoid(c_method_name, class_name);
	#elif defined(QPA_QT5)
		jstring jclassname = jep.JStringFromQString(class_name);
		QAndroidJniObject::callStaticMethod<void>(c_class_name, c_method_name, "(Ljava/lang/String;)V", jclassname);
		jep.env()->DeleteLocalRef(jclassname);
	#endif
}

} // namespace QAndroidQPAPluginGap

// JNI entry points. Must be "C" because the function names should not be mangled.
extern "C" {

JNIEXPORT void JNICALL Java_ru_dublgis_qjnihelpers_ClassLoader_nativeJNIPreloadClass(JNIEnv * env, jobject, jstring classname)
{
	JniEnvPtr jep(env);
	QString qclassname = jep.QStringFromJString(classname);
	jep.PreloadClass(qclassname.toLatin1());
}

} // extern "C"

#endif // #if defined(Q_OS_ANDROID)
