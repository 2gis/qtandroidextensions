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

#include "QAndroidQPAPluginGap.h"

#include <atomic>
#include <mutex>
#include <optional>

#include <QtCore/qconfig.h>
#include <QtCore/QDebug>
#include <QtCore/QScopedPointer>

#if QT_VERSION < 0x050000 && defined(QJNIHELPERS_GRYM)
	#define QPA_QT4GRYM
#elif QT_VERSION >= 0x050000
	// Should work for Qt 6 as well ;)
	#define QPA_QT5
#else
	#error "Unimplemented QPA case"
#endif

#if defined(QPA_QT5)
	#include <QtAndroidExtras/QtAndroidExtras>
#endif

#if defined(Q_OS_ANDROID)

#if defined(QPA_QT4GRYM)
	// Exported from QtAndroidCore
	extern JavaVM * qt_android_get_java_vm();
#elif QT_VERSION >= 0x050000
	#include <QtAndroidExtras/QAndroidJniEnvironment>
#else
	#error "Unimplemented QPA case"
#endif


namespace
{

#if defined(QPA_QT4GRYM)
	const char * const c_activity_getter_class_name = "org/qt/core/QtApplicationBase";
	const char * const c_activity_getter_method_name = "getActivityStatic";
	const char * const c_activity_getter_result_name = "org/qt/core/QtActivityBase";
#elif defined(QPA_QT5)
	const char * const c_activity_getter_class_name = "org/qtproject/qt5/android/QtNative";
	const char * const c_activity_getter_method_name = "activity";
	const char * const c_activity_getter_result_name = "android/app/Activity";
#else
	#error "Unimplemented QPA case"
#endif

} // anonymous namespace



namespace QAndroidQPAPluginGap
{

namespace
{

QMutex s_global_context_mutex { QMutex::Recursive };
std::optional<QJniObject> s_activity;
QJniObject s_custom_context;


void initActivity()
{
	QMutexLocker locker(&s_global_context_mutex);
	if (s_activity)
	{
		return;
	}
	QJniClass activityGetterClass(c_activity_getter_class_name);
	if (!activityGetterClass)
	{
		s_activity = QJniObject {};
		qWarning() << "QtActivity retriever class could not be accessed.";
		return;
	}
	s_activity = activityGetterClass.callStaticObj(
		c_activity_getter_method_name,
		c_activity_getter_result_name);
	if (!(*s_activity))
	{
		qInfo() << "QtActivity retriever didn't return an object.";
	}
}

} // anonymous namespace



Context::Context()
	: QJniObject(getCurrentContextNoThrow(), true)
{
}



Activity::Activity()
	: QJniObject(getActivityNoThrow(), true)
{
}



JavaVM * getJavaVM()
{
	#if defined(QPA_QT4GRYM)
		return qt_android_get_java_vm();
	#elif defined(QPA_QT5)
		return QAndroidJniEnvironment::javaVM();
	#else
		#error "Unimplemented QPA case"
	#endif
}


jobject JNICALL getActivityEx(JNIEnv * env, jobject, bool errorIfNone)
{
	QMutexLocker locker(&s_global_context_mutex);
	if (!s_activity)
	{
		initActivity();
	}
	if (s_activity && *s_activity)
	{
		return QJniEnvPtr(env).env()->NewLocalRef(s_activity->jObject());
	}
	else
	{
		if (errorIfNone)
		{
			throw QJniBaseException("Failed to get QtActivity object.");
		}
		else
		{
			return nullptr;
		}
	}
}


jobject JNICALL getActivity(JNIEnv * env, jobject jo)
{
	return getActivityEx(env, jo, true);
}


jobject JNICALL getActivityNoThrowEx(JNIEnv * env, jobject jo, bool errorIfNone)
{
	try
	{
		return getActivityEx(env, jo, errorIfNone);
	}
	catch (const std::exception & e)
	{
		if (errorIfNone)
		{
			qCritical() << "getActivityNoThrowEx exception:" << e.what();
		}
		else
		{
			qInfo() << "getActivityNoThrowEx exception:" << e.what();
		}
		return nullptr;
	}
}


jobject JNICALL getActivityNoThrow(JNIEnv * env, jobject jo)
{
	return getActivityNoThrowEx(env, jo, true);
}


bool isActivityAvailable()
{
	QMutexLocker locker(&s_global_context_mutex);
	if (!s_activity)
	{
		initActivity();
	}
	return s_activity && *s_activity;
}


void setCustomContext(jobject context)
{
	QMutexLocker locker(&s_global_context_mutex);
	if (context)
	{
		s_custom_context = QJniObject(context, true);
	}
	else
	{
		s_custom_context = {};
	}
}


jobject JNICALL getCustomContext(JNIEnv * env, jobject)
{
	QMutexLocker locker(&s_global_context_mutex);
	return (s_custom_context)
		? QJniEnvPtr(env).env()->NewLocalRef(s_custom_context.jObject())
		: jobject { nullptr };
}


bool customContextSet()
{
	QMutexLocker locker(&s_global_context_mutex);
	return !!s_custom_context;
}


jobject JNICALL getCurrentContext(JNIEnv * env, jobject)
{
	QMutexLocker locker(&s_global_context_mutex);
	if (jobject ret = getCustomContext())
	{
		return ret;
	}
	return getActivity();
}


jobject JNICALL getCurrentContextNoThrow(JNIEnv * env, jobject jo)
{
	try
	{
		return getCurrentContext(env, jo);
	}
	catch (const std::exception & e)
	{
		qCritical() << "getCurrentContext exception:" << e.what();
		return nullptr;
	}
}


void preloadJavaClass(const char * class_name)
{
	// Directly calling to Java_ru_dublgis_qjnihelpers_ClassLoader_nativeJNIPreloadClass seems to be
	// not working properly in some situations (???)
	// Also, it is nicer to have a single function to preload classed and single path to call it.
	// So, let's call it through Java.
	if (!class_name || !(*class_name))
	{
		qWarning() << "preloadJavaClass has been called with empty class name!";
		return;
	}
	try
	{
		QJniEnvPtr jep;
		if (jep.isClassPreloaded(class_name))
		{
			return;
		}

		static const char * const c_class_name = "ru/dublgis/qjnihelpers/ClassLoader";
		static const char * const c_method_name = "callJNIPreloadClass";
		#if defined(QPA_QT4GRYM)
			QJniClass(c_class_name).callStaticVoid(c_method_name, class_name);
		#elif defined(QPA_QT5)
			QAndroidJniObject::callStaticMethod<void>(c_class_name, c_method_name, "(Ljava/lang/String;)V",
				QJniLocalRef(jep, class_name).jObject());
		#endif
	}
	catch (const std::exception & e)
	{
		qWarning() << "Failed to preload class:" << class_name << "Exception:" << e.what();
		throw e;
	}
}


void preloadJavaClasses(const std::initializer_list<const char *> & list)
{
	for (const char * const class_name: list)
	{
		preloadJavaClass(class_name);
	}
}


void preloadJavaClasses()
{
	static std::once_flag call_flag;
	std::call_once(call_flag, [] {
		preloadJavaClasses({
			c_activity_getter_class_name,
			"android/os/Build$VERSION"});
	});
}


int apiLevel()
{
	static std::atomic<int> level_ = -1;
	if (level_ < 0)
	{
		QJniClass version("android/os/Build$VERSION");
		level_ = version.getStaticIntField("SDK_INT");
	}
	return level_;
}

} // namespace QAndroidQPAPluginGap


// JNI entry points. Must be "C" because the function names should not be mangled.
extern "C" {

JNIEXPORT void JNICALL Java_ru_dublgis_qjnihelpers_ClassLoader_nativeJNIPreloadClass(JNIEnv * env, jobject, jstring classname);

/*! This function does the actual pre-loading of a Java class. It can be called either from Java
	via ClassLoader.callJNIPreloadClass() or from C++ main() thread as QAndroidQPAPluginGap.preloadJavaClass(). */
JNIEXPORT void JNICALL Java_ru_dublgis_qjnihelpers_ClassLoader_nativeJNIPreloadClass(JNIEnv * env, jobject, jstring classname)
{
	QJniEnvPtr jep(env);
	const QString qclassname = jep.QStringFromJString(classname);
	if (!jep.preloadClass(qclassname.toLatin1()))
	{
		qCritical() << "Failed to preload Java class:" << qclassname;
	}

	// During the first call of this function, we can also pre-load classes for our own use.
	static std::once_flag call_flag;
	std::call_once(call_flag, [&jep] {
		if (!jep.preloadClass(c_activity_getter_class_name))
		{
			qCritical() << "Failed to preload Java class:" << c_activity_getter_class_name;
		}
	});
}

} // extern "C"

#endif // #if defined(Q_OS_ANDROID)
