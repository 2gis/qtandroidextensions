#include "stdafx.h"
#include <string>
#include <stdarg.h>
#include <QtCore/qdebug.h>
#include <JNIUtils/debug.h>
#include <JNIUtils/JniEnvPtr.h>
#include <JNIUtils/jcGeneric.h>

// Влючение более старого кода работы со ссылками.
// См. GRMPPC-5775.
//! \todo Если в этом направлении всё будет ОК, то в 2014 можно вычистить.
// #define OLD_GOOD_CODE

const char * jcGeneric::method_not_found_exception::what() const throw()
{
	return "Java method not found";
}

const char * jcGeneric::field_not_found_exception::what() const throw()
{
	return "Java field not found";
}

// TODO: severely optimize (delete local refs, etc)

jcGeneric::jcGeneric(jobject instance, bool take_ownership)
	: instance_(0)
	, class_(0)
{
	JniEnvPtr jep;
	// Создаёт глобальную ссылку
	init(jep.env(), instance);
	if (take_ownership)
	{
		jep.env()->DeleteLocalRef(instance);
	}
}

jcGeneric::jcGeneric(jclass clazz, bool create)
	: instance_(0)
	, class_(0)
{
	JniEnvPtr jep;
	init(jep.env(), clazz, create);
}

jcGeneric::jcGeneric(const char* full_class_name, bool create)
	: instance_(0)
	, class_(0)
{
	JniEnvPtr jep;
	init(jep.env(), full_class_name, create);
}

jcGeneric::jcGeneric()
	: instance_(0)
	, class_(0)
{
}

jcGeneric::~jcGeneric()
{
	LOGV("jcGeneric::~jcGeneric() %p",this);
	JniEnvPtr jep;
	if (instance_ != NULL)
	{
		jep.env()->DeleteGlobalRef(instance_);
	}
#ifndef OLD_GOOD_CODE
	// OLD CODE: no this block
	if (class_ != NULL)
	{
		jep.env()->DeleteGlobalRef(class_);
	}
#endif
}

void jcGeneric::init(JNIEnv* env, jobject instance)
{
	LOGV("jcGeneric::init(JNIEnv* env, jobject instance) %p",this);
	// FIXME: exceptions
	instance_ = env->NewGlobalRef(instance);
	
	// here we get a warning if instance is an object that was passed from Java to native directly (i.e. in a JNI call)
	// "" JNI WARNING: DeleteLocalRef(0x44902210) failed to find entry (valid=1) ""
	//env->DeleteLocalRef(instance);

	// class is expected to be a valid ref during the whole lifetime of the instance_ object
#ifdef OLD_GOOD_CODE
	// OLD CODE:
	class_ = env->GetObjectClass(instance_);
#else
	jclass clazz = env->GetObjectClass(instance_);
	class_ = static_cast<jclass>(env->NewGlobalRef(clazz));
	// qDebug()<<QString("DeleteLocalRef: 0x%1 >>>>").arg((unsigned long)clazz, 0, 16)<<__LINE__;
	env->DeleteLocalRef(clazz);
	// qDebug()<<QString("DeleteLocalRef: 0x%1 <<<<").arg((unsigned long)clazz, 0, 16)<<__LINE__;
#endif
}

void jcGeneric::init(JNIEnv* env, jclass class_to_instantiate, bool create)
{
	LOGV("jcGeneric::init(JNIEnv* env, jclass class_to_instantiate, %d) %p", (int)create, this);

	if( create )
	{
		// get constructor method id. FIXME: exceptions
		jmethodID mid_init = env->GetMethodID(class_to_instantiate, "<init>", "()V");
		if (!mid_init)
		{
			LOG("%s: method not found.", __FUNCTION__);
			throw method_not_found_exception();
		}

		// create class instance; FIXME: exceptions
		jobject obj = env->NewObject(class_to_instantiate, mid_init);

		// it is dangerous to go alone, use this
		init(env, obj);
		env->DeleteLocalRef(obj);
	}
	else
	{
#ifdef OLD_GOOD_CODE
		// OLD CODE:
		class_ = class_to_instantiate;
#else
		class_ = static_cast<jclass>(env->NewGlobalRef(class_to_instantiate));
#endif
		instance_ = NULL;
	}
}

void jcGeneric::init(JNIEnv* env, const char * full_class_name, bool create)
{
	LOGV("jcGeneric::init(JNIEnv* env, const char* full_class_name) %p: %s, create: %d", this, full_class_name, (int)create);
	// FIXME: exceptions
	JniEnvPtr jep(env);
	jclass cls = jep.FindClass(full_class_name); // this is a preloaded global ref, we don't need to delete it as a local ref
	if(cls == NULL)
	{
		LOG("Could not find class %s", full_class_name);
		return; // FIXME: throw
	}
	LOGV("Class \"%s\" is loaded @ %p", full_class_name, cls);

	if (create)
	{
		init(env, cls, true);
	}
	else
	{
#ifdef OLD_GOOD_CODE
		// OLD CODE:
		class_ = cls;
#else
		class_ = static_cast<jclass>(env->NewGlobalRef(cls));
#endif
		instance_ = NULL;
	}
#ifndef OLD_GOOD_CODE
	// OLD CODE: it was not here
	/* См. комментарий к jclass cls = jep.FindClass(full_class_name);
	qDebug()<<QString("DeleteLocalRef: 0x%1 >>>>").arg((unsigned long)cls, 0, 16)<<__LINE__;
	env->DeleteLocalRef(cls);
	qDebug()<<QString("DeleteLocalRef: 0x%1 <<<").arg((unsigned long)cls, 0, 16)<<__LINE__; */
#endif
}

void jcGeneric::CallVoid(const char* method_name)
{
	LOGV("void jcGeneric::CallVoid(const char* method_name) %p \"%s\"",this,method_name);
	JniEnvPtr jep;
	JNIEnv* env = jep.env();
	jmethodID mid = env->GetMethodID(class_, method_name, "()V");
	if (!mid)
	{
		LOG("%s: method not found.", __FUNCTION__);
		throw method_not_found_exception();
	}
	env->CallVoidMethod(instance_, mid);
}

bool jcGeneric::CallBool(const char* method_name)
{
	LOGV("bool jcGeneric::CallBool(const char* method_name) %p \"%s\"",this,method_name);
	JniEnvPtr jep;
	JNIEnv* env = jep.env();
	jmethodID mid = env->GetMethodID(class_, method_name, "()Z");
	if (!mid)
	{
		LOG("%s: method not found.", __FUNCTION__);
		throw method_not_found_exception();
	}
	return (JNI_TRUE==env->CallBooleanMethod(instance_, mid));
}

int jcGeneric::CallInt(const char* method_name)
{
	LOGV("int jcGeneric::CallInt(const char* method_name) %p \"%s\"",this,method_name);
	JniEnvPtr jep;
	JNIEnv* env = jep.env();
	jmethodID mid = env->GetMethodID(class_, method_name, "()I");
	if (!mid)
	{
		LOG("%s: method not found.", __FUNCTION__);
		throw method_not_found_exception();
	}
	return (int)env->CallIntMethod(instance_, mid);
}

long long jcGeneric::CallLong(const char* method_name)
{
	LOGV("int jcGeneric::CallLong(const char* method_name) %p \"%s\"",this,method_name);
	JniEnvPtr jep;
	JNIEnv* env = jep.env();
	jmethodID mid = env->GetMethodID(class_, method_name, "()J");
	if (!mid)
	{
		LOG("%s: method not found.", __FUNCTION__);
		throw method_not_found_exception();
	}
	return (long long)env->CallLongMethod(instance_, mid);
}

float jcGeneric::CallFloat(const char* method_name)
{
	LOGV("float jcGeneric::CallFloat(const char* method_name) %p \"%s\"",this,method_name);
	JniEnvPtr jep;
	JNIEnv* env = jep.env();
	jmethodID mid = env->GetMethodID(class_, method_name, "()F");
	if (!mid)
	{
		LOG("%s: method not found.", __FUNCTION__);
		throw method_not_found_exception();
	}
	return (float)env->CallFloatMethod(instance_, mid);
}

double jcGeneric::CallDouble(const char* method_name)
{
	LOGV("float jcGeneric::CallDouble(const char* method_name) %p \"%s\"",this,method_name);
	JniEnvPtr jep;
	JNIEnv* env = jep.env();
	jmethodID mid = env->GetMethodID(class_, method_name, "()D");
	if (!mid)
	{
		LOG("%s: method not found.", __FUNCTION__);
		throw method_not_found_exception();
	}
	return (double)env->CallDoubleMethod(instance_, mid);
}

jcGeneric* jcGeneric::CallObject(const char* method_name, const char* objname)
{
	std::string signature = "()L";
	signature += objname;
	signature += ";";

	LOGV("jcGeneric::CallObject: \"%s\", \"%s\"", method_name, signature.c_str());
	JniEnvPtr jep;
	JNIEnv* env = jep.env();
	jmethodID mid = env->GetMethodID(class_, method_name, signature.c_str());
	if (!mid)
	{
		LOG("%s: method not found.", __FUNCTION__);
		throw method_not_found_exception();
	}
	jobject object = env->CallObjectMethod(instance_,mid);
	jcGeneric* result = new jcGeneric(object, true);
	return result;
}

void jcGeneric::CallStaticVoid(const char* method_name)
{
	LOGV("void jcGeneric::CallStaticVoid(const char* method_name) %p \"%s\"", this, method_name);
	JniEnvPtr jep;
	JNIEnv* env = jep.env();
	jmethodID mid = env->GetStaticMethodID(class_, method_name, "()V");
	if (!mid)
	{
		LOG("%s: method not found.", __FUNCTION__);
		throw method_not_found_exception();
	}
	env->CallStaticVoidMethod(class_, mid);
}

#ifdef QT_CORE_LIB
QString jcGeneric::CallString(const char *method_name)
{
	LOGV("QString jcGeneric::CallString(const char* method_name) %p \"%s\"",this,method_name);
	JniEnvPtr jep;
	JNIEnv* env = jep.env();
	jmethodID mid = env->GetMethodID(class_, method_name, "()Ljava/lang/String;");
	if (!mid)
	{
		LOG("%s: method not found.", __FUNCTION__);
		throw method_not_found_exception();
	}
	jstring jret = (jstring)env->CallObjectMethod(instance_, mid);
	if (jret == 0)
	{
		return QString();
	}
	QString ret = jep.QStringFromJString(jret);
	env->DeleteLocalRef(jret);
	return ret;
}

QString jcGeneric::CallStaticString(const char *method_name)
{
	LOGV("QString jcGeneric::CallStaticString(const char* method_name) %p \"%s\"",this,method_name);
	JniEnvPtr jep;
	JNIEnv* env = jep.env();
	jmethodID mid = env->GetStaticMethodID(class_, method_name, "()Ljava/lang/String;");
	if (!mid)
	{
		LOG("%s: method not found.", __FUNCTION__);
		throw method_not_found_exception();
	}
	jstring jret = (jstring)env->CallStaticObjectMethod(class_, mid);
	if (jret == 0)
	{
		return QString();
	}
	QString ret = jep.QStringFromJString(jret);
	env->DeleteLocalRef(jret);
	return ret;
}

QString jcGeneric::GetString(const char *field_name)
{
	JniEnvPtr jep;
	JNIEnv* env = jep.env();
	jfieldID fid = env->GetFieldID(class_, field_name, "Ljava/lang/String;");
	if (!fid)
	{
		LOG("%s: method not found.", __FUNCTION__);
		throw field_not_found_exception();
	}
	jstring jret = (jstring)env->GetObjectField(instance_, fid);
	if (jret == 0)
	{
		return QString();
	}
	QString ret = jep.QStringFromJString(jret);
	env->DeleteLocalRef(jret);
	return ret;
}
#endif // QT_CORE_LIB

jcGeneric* jcGeneric::CallStaticObject(const char *method_name, const char *objname)
{
	LOGV("jcGeneric::CallStaticObject(\"%s\",\"%s\")", method_name, objname);
	std::string signature = "()L";
	signature += objname;
	signature += ";";

	LOGV("jcGeneric::CallStaticObject signature: %s", signature.c_str());
	JniEnvPtr jep;
	JNIEnv* env = jep.env();

	LOGV("env->GetStaticMethodID");
	jmethodID mid = env->GetStaticMethodID(class_, method_name, signature.c_str());
	if (!mid)
	{
		LOG("%s: method not found.", __FUNCTION__);
		throw method_not_found_exception();
	}

	LOGV("new jcGeneric(env->CallStaticObjectMethod(class_,mid), true);");
	jobject jret = env->CallStaticObjectMethod(class_, mid);
	if (jret == 0)
	{
		return NULL;
	}
	return new jcGeneric(jret, true);
}

int jcGeneric::GetInt(const char* field_name)
{
	LOGV("int jcGeneric::GetInt(const char* fieldd_name) %p \"%s\"",this,field_name);
	JniEnvPtr jep;
	JNIEnv* env = jep.env();
	jfieldID fid = env->GetFieldID(class_, field_name, "I");
	if (!fid)
	{
		LOG("%s: method not found.", __FUNCTION__);
		throw field_not_found_exception();
	}
	return (int)env->GetIntField(instance_, fid);
}

// TODO: add something like what they do in jni.h:
//	#define CALL_TYPE_METHOD(_jtype, _jname)                                    \
//	_jtype Call##_jname##Method(jobject obj, jmethodID methodID, ...)
//	...
void jcGeneric::CallParamVoid(const char* method_name, const char* param_signature, ...)
{
	LOGV("void jcGeneric(%p)::CallParamVoid(\"%s\", \"%s\", ...)", this, method_name, param_signature);

	va_list args;
	JniEnvPtr jep;
	JNIEnv* env = jep.env();

	std::string signature = "(";
	signature += param_signature;
	signature += ")V";
	jmethodID mid = env->GetMethodID(class_, method_name, signature.c_str());
	if (!mid)
	{
		LOG("%s: method not found.", __FUNCTION__);
		throw method_not_found_exception();
	}

	va_start(args, param_signature);
	env->CallVoidMethodV(instance_, mid, args);
	va_end(args);

	if (env->ExceptionCheck())
	{
		LOG("void jcGeneric(%p)::CallParamVoid(\"%s\", \"%s\", ...): exception occured", this, method_name, param_signature);
		env->ExceptionDescribe();
		env->ExceptionClear();
		return;
	}
}

void jcGeneric::RegisterNativeMethod(const char* name, const char* signature, void* ptr)
{
	JniEnvPtr jep;
	JNINativeMethod jnm = {name, signature, ptr};
	jep.env()->RegisterNatives(class_, &jnm, 1);
}

void jcGeneric::RegisterNativeMethods(JNINativeMethod* methods_list, size_t sizeof_methods_list)
{
	JniEnvPtr jep;
	jep.env()->RegisterNatives(class_, methods_list, sizeof_methods_list/sizeof(JNINativeMethod));
}
