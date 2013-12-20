//#include <string>
//#include <map>

#include "stdafx.h"

#include <JNIUtils/debug.h>

#include <JNIUtils/JniEnvPtr.h>

static JavaVM* g_JavaVm = 0;
typedef std::map<std::string,jclass> PreloadedClasses;
static PreloadedClasses g_PreloadedClasses;
//static std::string g_PackageName;

JniEnvPtr::JniEnvPtr()
	: env_(0)
	, was_already_attached_(false)
{
	if(g_JavaVm == 0)
	{
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
			LOG("Error attaching current thread: %d", errsv);
			return;
		}
	}
	else if(errsv != 0)
	{
		LOG("Error getting Java environment: %d", errsv);
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
			LOG("thread detach failed: %d", errsv);
		}
	}
}

JNIEnv* JniEnvPtr::env()
{
	return env_;
}

int JniEnvPtr::PreloadClass(const char* class_name)
{
	LOG("Preloading class \"%s\"", class_name);
	jclass clazz = env_->FindClass(class_name);
	if (env_->ExceptionCheck())
	{
		LOG("...Failed to preload class %s", class_name);
		env_->ExceptionClear();
		return -1;
	}
	jclass gclazz = (jclass)env_->NewGlobalRef(clazz);
	env_->DeleteLocalRef(clazz);
	g_PreloadedClasses.insert(std::pair<std::string,jclass>(std::string(class_name),gclazz));
	LOGV("...Preloaded class \"%s\" as %p", class_name, gclazz);
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
	LOGV("Searching for class \"%s\"", name);
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
