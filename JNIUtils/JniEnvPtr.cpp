/*
  JNIUtils library

  Authors:
  Ivan Avdeev <marflon@gmail.com>
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

#include "stdafx.h"
#include "JniEnvPtr.h"
#include <QAndroidQPAPluginGap.h>

typedef std::map<std::string, jclass> PreloadedClasses;

class JniEnvPtrThreadDetacher
{
public:
	~JniEnvPtrThreadDetacher();
};

static JavaVM * g_JavaVm = 0;
static QMutex g_PreloadedClassesMutex;
static PreloadedClasses g_PreloadedClasses;
static QThreadStorage<JniEnvPtrThreadDetacher*> g_JavaThreadDetacher;

JniEnvPtrThreadDetacher::~JniEnvPtrThreadDetacher()
{
	if (g_JavaVm)
	{
		int errsv = g_JavaVm->DetachCurrentThread();
		if (errsv != JNI_OK)
		{
			qWarning("Thread %d detach failed: %d", (int)gettid(), errsv);
		}
	}
}

#if defined(Q_OS_ANDROID)
	static inline void AutoSetJavaVM()
	{
		if (!g_JavaVm)
		{
			g_JavaVm = QAndroidQPAPluginGap::detectJavaVM();
		}
	}
#endif

JniEnvPtr::JniEnvPtr()
	: env_(0)
{
	#if defined(Q_OS_ANDROID) || defined(ANDROID)
		AutoSetJavaVM();
	#endif
	if (g_JavaVm == 0)
	{
		qWarning("Java VM pointer is not set!");
		return;
	}
	int errsv = g_JavaVm->GetEnv((void**)(&env_), JNI_VERSION_1_6);
	if (errsv == JNI_EDETACHED)
	{
		VERBOSE(qDebug("Current thread %d is not attached, attaching it...", (int)gettid()));
		errsv = g_JavaVm->AttachCurrentThread(&env_, 0);
		if (errsv != 0)
		{
			qWarning("Error attaching current thread %d: %d", (int)gettid(), errsv);
			return;
		}
		if (!g_JavaThreadDetacher.hasLocalData())
		{
			g_JavaThreadDetacher.setLocalData(new JniEnvPtrThreadDetacher());
		}
		VERBOSE(qDebug("Attached current thread %d successfully.", (int)gettid()));
	}
	else if (errsv != JNI_OK)
	{
		qWarning("Error getting Java environment: %d", errsv);
		return;
	}
}

JniEnvPtr::JniEnvPtr(JNIEnv* env)
	: env_(env)
{
	#if defined(Q_OS_ANDROID) || defined(ANDROID)
		AutoSetJavaVM();
	#endif
}

JniEnvPtr::~JniEnvPtr()
{
}

JNIEnv* JniEnvPtr::env() const
{
	return env_;
}

bool JniEnvPtr::PreloadClass(const char* class_name)
{
	qWarning("Preloading class \"%s\"", class_name);
	jclass clazz = env_->FindClass(class_name);
	if (SuppressException())
	{
		qWarning("...Failed to preload class %s (tid %d)", class_name, (int)gettid());
		return false;
	}
	jclass gclazz = (jclass)env_->NewGlobalRef(clazz);
	env_->DeleteLocalRef(clazz);
	QMutexLocker locker(&g_PreloadedClassesMutex);
	g_PreloadedClasses.insert(std::pair<std::string,jclass>(std::string(class_name),gclazz));
	VERBOSE(qDebug("...Preloaded class \"%s\" as %p for tid %d", class_name, gclazz, (int)gettid()));
	return true;
}

int JniEnvPtr::PreloadClasses(const char* const* class_list)
{
	int loaded = 0;
	for(; *class_list != 0; ++class_list)
	{
		if (!PreloadClass(*class_list))
		{
			break;
		}
		++loaded;
	}
	return loaded;
}

bool JniEnvPtr::IsClassPreloaded(const char * class_name)
{
	QMutexLocker locker(&g_PreloadedClassesMutex);
	return g_PreloadedClasses.find(class_name) != g_PreloadedClasses.end();
}

void JniEnvPtr::UnloadClasses()
{
	QMutexLocker locker(&g_PreloadedClassesMutex);
	PreloadedClasses::iterator it;
	for(it = g_PreloadedClasses.begin(); it != g_PreloadedClasses.end(); ++it)
	{
		env_->DeleteGlobalRef((*it).second);
	}
	g_PreloadedClasses.clear();
}

jclass JniEnvPtr::FindClass(const char * name)
{
	// first try for preloaded classes
	{
		QMutexLocker locker(&g_PreloadedClassesMutex);
		VERBOSE(qDebug("Searching for class \"%s\" in tid %d", name, (int)gettid()));
		PreloadedClasses::iterator it = g_PreloadedClasses.find(std::string(name));
		if (it != g_PreloadedClasses.end())
		{
			return (*it).second;
		}
		#if defined(JNIUTILS_VERBOSE_LOG)
			qDebug("No class \"%s\" found in preloaded classes (tid %d), I have %d known classes",
				name, (int)gettid(),  (int)g_PreloadedClasses.size());
			for (it = g_PreloadedClasses.begin(); it != g_PreloadedClasses.end(); ++it)
			{
				qDebug("\"%s\" -> %p", (*it).first.c_str(), (*it).second);
			}
		#endif
	}

	// if it wasn't preloaded, try to load it in JNI (will fail for custom classes in native-created threads)
	VERBOSE(qDebug("Trying to construct the class directly: \"%s\" in tid %d", name, (int)gettid()));
	jclass cls = env_->FindClass(name);
	if (SuppressException())
	{
		qWarning("Failed to find class \"%s\"", name);
		return 0;
	}

	// We must store class ref in a global ref
	jclass ret = (jclass)env_->NewGlobalRef(cls);
	env_->DeleteLocalRef(cls);

	// Add it to a list of preloaded classes for convenience
	QMutexLocker locker(&g_PreloadedClassesMutex);
	g_PreloadedClasses.insert(std::pair<std::string,jclass>(std::string(name),ret));

	VERBOSE(qDebug("Successfuly found Java class: \"%s\" in tid %d", name, (int)gettid()));

	return ret;
}

JavaVM * JniEnvPtr::getJavaVM() const
{
	return g_JavaVm;
}

void JniEnvPtr::SetJavaVM(JavaVM* vm)
{
	// qDebug()<<"SetJavaVM"<<vm;
	g_JavaVm = vm;
}

void JniEnvPtr::SetJavaVM(JNIEnv* env)
{
	env->GetJavaVM(&g_JavaVm);
}

jstring JniEnvPtr::JStringFromQString(const QString& str)
{
	jstring ret = env_->NewString(str.utf16(), str.length());
	if (SuppressException())
	{
		qWarning("Failed to convert QString to jstring.");
		return 0;
	}
	return ret;
}

QString JniEnvPtr::QStringFromJString(jstring str)
{
	if (str == NULL)
	{
		return QString();
	}

	int length = env_->GetStringLength(str);
	if (length == 0)
	{
		return QString();
	}

	const jchar* str_ptr = env_->GetStringChars(str, NULL);
	if (str_ptr == NULL)
	{
		return QString();
	}

	QString ret = QString((const QChar *)str_ptr, length);
	env_->ReleaseStringChars(str, str_ptr);
	return ret;
}

bool JniEnvPtr::SuppressException(bool describe)
{
	if (env_->ExceptionCheck())
	{
		if (describe)
		{
			env_->ExceptionDescribe();
		}
		env_->ExceptionClear();
		return true;
	}
	else
	{
		return false;
	}
}
