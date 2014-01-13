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

static JavaVM * g_JavaVm = 0;
typedef std::map<std::string,jclass> PreloadedClasses;
static PreloadedClasses g_PreloadedClasses;

JniEnvPtr::JniEnvPtr()
	: env_(0)
	, was_already_attached_(false)
{
	if(g_JavaVm == 0)
	{
		qWarning("Java VM pointer is not set!");
		return;
	}
	was_already_attached_ = true;
	int errsv = g_JavaVm->GetEnv((void**)(&env_), JNI_VERSION_1_4);
	if(errsv == JNI_EDETACHED)
	{
		was_already_attached_ = false;
		errsv = g_JavaVm->AttachCurrentThread(&env_, 0);
		if( errsv != 0 )
		{
			qWarning("Error attaching current thread: %d", errsv);
			return;
		}
	}
	else if(errsv != 0)
	{
		qWarning("Error getting Java environment: %d", errsv);
		return;
	}
}

JniEnvPtr::JniEnvPtr(JNIEnv* env)
	: env_(env)
	, was_already_attached_(true)
{
}

JniEnvPtr::~JniEnvPtr()
{
	if(!was_already_attached_)
	{
		int errsv = g_JavaVm->DetachCurrentThread();
		if(errsv != 0 )
		{
			qWarning("thread detach failed: %d", errsv);
		}
	}
}

JNIEnv* JniEnvPtr::env()
{
	return env_;
}

int JniEnvPtr::PreloadClass(const char* class_name)
{
	qWarning("Preloading class \"%s\"", class_name);
	jclass clazz = env_->FindClass(class_name);
	if (env_->ExceptionCheck())
	{
		qWarning("...Failed to preload class %s", class_name);
		env_->ExceptionClear();
		return -1;
	}
	jclass gclazz = (jclass)env_->NewGlobalRef(clazz);
	env_->DeleteLocalRef(clazz);
	g_PreloadedClasses.insert(std::pair<std::string,jclass>(std::string(class_name),gclazz));
	VERBOSE(qDebug("...Preloaded class \"%s\" as %p", class_name, gclazz));
	return 0;
}

int JniEnvPtr::PreloadClasses(const char* const* class_list)
{
	int loaded = 0;
    for(; *class_list != 0; ++class_list, ++loaded )
	{
        if(PreloadClass(*class_list) != 0)
		{
			break;
		}
	}
	return loaded;
}

void JniEnvPtr::UnloadClasses()
{
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
	VERBOSE(qDebug("Searching for class \"%s\"", name));
    PreloadedClasses::iterator it = g_PreloadedClasses.find(std::string(name));
    if (it != g_PreloadedClasses.end())
	{
        return (*it).second;
	}
#if defined(DEBUG_VERBOSE)
    LOGV("no class \"%s\" found in preloaded classes, known classes are:", name);
    for(it = g_PreloadedClasses.begin(); it != g_PreloadedClasses.end(); ++it)
	{
        LOGV("\"%s\" -> %p", (*it).first.c_str(), (*it).second);
	}
#endif //DEBUG_VERBOSE

	// if it wasn't preloaded, try to load it in JNI (will fail for custom classes in native-created threads)
	jclass cls = env_->FindClass(name);
	if (env_->ExceptionCheck())
	{
		env_->ExceptionClear();
		return 0;
	}

	// add it to a list of preloaded classes for convenience
	jclass ret = (jclass)env_->NewGlobalRef(cls);
	env_->DeleteLocalRef(cls);
	g_PreloadedClasses.insert(std::pair<std::string,jclass>(std::string(name),ret));

	return ret;
}

/*JavaVM* JniEnvPtr::JavaVM()
{
	return g_JavaVm;
}*/

void JniEnvPtr::SetJavaVM(JavaVM* vm)
{
	g_JavaVm = vm;
}

void JniEnvPtr::SetJavaVM(JNIEnv* env)
{
	env->GetJavaVM(&g_JavaVm);
}

/*
std::string JniEnvPtr::PackageName()
{
	if(g_PackageName.empty())
	{
		JniEnvPtr jep;
		jcGeneric
	}
	return g_PackageName;
}*/

#ifdef QT_CORE_LIB
jstring JniEnvPtr::JStringFromQString(const QString& str)
{
	jstring ret = env_->NewString(str.utf16(), str.length());
	if (env_->ExceptionCheck())
		env_->ExceptionClear();
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

#endif //QT_CORE_LIB
