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
#include <string>
#include <stdarg.h>
#include <QtCore/qdebug.h>
#include "JniEnvPtr.h"
#include "jcGeneric.h"

const char * jcGeneric::method_not_found_exception::what() const throw()
{
	return "Java method not found";
}

const char * jcGeneric::field_not_found_exception::what() const throw()
{
	return "Java field not found";
}

// Be sure to delete the ref via env->DeleteLocalRef(jstring)
static inline jstring QStringToJstring(JNIEnv *env, const QString& str)
{
	jstring ret = env->NewString(str.utf16(), str.length());
	if (env->ExceptionCheck())
		env->ExceptionClear();
	return ret;
}

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
	VERBOSE(qDebug()<<"Grym"<<__FILE__<<__LINE__);
	JniEnvPtr jep;
	VERBOSE(qDebug()<<"Grym"<<__FILE__<<__LINE__);
	init(jep.env(), full_class_name, create);
	VERBOSE(qDebug()<<"Grym"<<__FILE__<<__LINE__);
}

jcGeneric::jcGeneric()
	: instance_(0)
	, class_(0)
{
}

jcGeneric::~jcGeneric()
{
	VERBOSE(qDebug("jcGeneric::~jcGeneric() %p",this));
	JniEnvPtr jep;
	if (instance_ != NULL)
	{
		jep.env()->DeleteGlobalRef(instance_);
	}
	if (class_ != NULL)
	{
		jep.env()->DeleteGlobalRef(class_);
	}
}

jobject jcGeneric::TakeJobjectOver()
{
	jobject ret = instance_;
	instance_ = 0;
	return ret;
}

void jcGeneric::init(JNIEnv* env, jobject instance)
{
	VERBOSE(qDebug("jcGeneric::init(JNIEnv* env, jobject instance) %p", this));
	// FIXME: exceptions
	instance_ = env->NewGlobalRef(instance);
	
	// here we get a warning if instance is an object that was passed from Java to native directly (i.e. in a JNI call)
	// "" JNI WARNING: DeleteLocalRef(0x44902210) failed to find entry (valid=1) ""
	//env->DeleteLocalRef(instance);

	// class is expected to be a valid ref during the whole lifetime of the instance_ object
	jclass clazz = env->GetObjectClass(instance_);
	class_ = static_cast<jclass>(env->NewGlobalRef(clazz));
	// VERBOSE(qDebug()<<QString("DeleteLocalRef: 0x%1 >>>>").arg((unsigned long)clazz, 0, 16)<<__LINE__);
	env->DeleteLocalRef(clazz);
	// VERBOSE(qDebug()<<QString("DeleteLocalRef: 0x%1 <<<<").arg((unsigned long)clazz, 0, 16)<<__LINE__);
}

void jcGeneric::init(JNIEnv* env, jclass class_to_instantiate, bool create)
{
	VERBOSE(qDebug("jcGeneric::init(JNIEnv* env, jclass class_to_instantiate, %d) %p", (int)create, this));

	if( create )
	{
		// get constructor method id. FIXME: exceptions
		jmethodID mid_init = env->GetMethodID(class_to_instantiate, "<init>", "()V");
		if (!mid_init)
		{
			qWarning("%s: method not found.", __FUNCTION__);
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
		class_ = static_cast<jclass>(env->NewGlobalRef(class_to_instantiate));
		instance_ = NULL;
	}
}

void jcGeneric::init(JNIEnv* env, const char * full_class_name, bool create)
{
	VERBOSE(qDebug("jcGeneric::init(JNIEnv* env, const char* full_class_name) %p: %s, create: %d", this, full_class_name, (int)create));
	// FIXME: exceptions
	JniEnvPtr jep(env);
	jclass cls = jep.FindClass(full_class_name); // this is a preloaded global ref, we don't need to delete it as a local ref
	if(cls == NULL)
	{
		qWarning("Could not find class %s", full_class_name);
		return; // FIXME: throw
	}
	VERBOSE(qDebug("Class \"%s\" is loaded @ %p", full_class_name, cls));

	if (create)
	{
		init(env, cls, true);
	}
	else
	{
		class_ = static_cast<jclass>(env->NewGlobalRef(cls));
		instance_ = NULL;
	}
	/* See comment for: jclass cls = jep.FindClass(full_class_name);
	VERBOSE(qDebug()<<QString("DeleteLocalRef: 0x%1 >>>>").arg((unsigned long)cls, 0, 16)<<__LINE__);
	env->DeleteLocalRef(cls);
	VERBOSE(qDebug()<<QString("DeleteLocalRef: 0x%1 <<<").arg((unsigned long)cls, 0, 16)<<__LINE__); */
}

void jcGeneric::CallVoid(const char* method_name)
{
	VERBOSE(qDebug("void jcGeneric::CallVoid(const char* method_name) %p \"%s\"",this,method_name));
	JniEnvPtr jep;
	JNIEnv* env = jep.env();
	jmethodID mid = env->GetMethodID(class_, method_name, "()V");
	if (!mid)
	{
		qWarning("%s: method not found.", __FUNCTION__);
		throw method_not_found_exception();
	}
	env->CallVoidMethod(instance_, mid);
}

bool jcGeneric::CallBool(const char* method_name)
{
	VERBOSE(qDebug("bool jcGeneric::CallBool(const char* method_name) %p \"%s\"",this,method_name));
	JniEnvPtr jep;
	JNIEnv* env = jep.env();
	jmethodID mid = env->GetMethodID(class_, method_name, "()Z");
	if (!mid)
	{
		qWarning("%s: method not found.", __FUNCTION__);
		throw method_not_found_exception();
	}
	return (JNI_TRUE==env->CallBooleanMethod(instance_, mid));
}

bool jcGeneric::CallBool(const char* method_name, bool param)
{
	VERBOSE(qDebug("bool jcGeneric::CallBool(const char* method_name, bool param) %p \"%s\"",this,method_name));
	JniEnvPtr jep;
	JNIEnv* env = jep.env();
	jmethodID mid = env->GetMethodID(class_, method_name, "(Z)Z");
	if (!mid)
	{
		qWarning("%s: method not found.", __FUNCTION__);
		throw method_not_found_exception();
	}
	return (JNI_TRUE==env->CallBooleanMethod(instance_, mid, jboolean(param)));
}

int jcGeneric::CallInt(const char* method_name)
{
	VERBOSE(qDebug("int jcGeneric::CallInt(const char* method_name) %p \"%s\"",this,method_name));
	JniEnvPtr jep;
	JNIEnv* env = jep.env();
	jmethodID mid = env->GetMethodID(class_, method_name, "()I");
	if (!mid)
	{
		qWarning("%s: method not found.", __FUNCTION__);
		throw method_not_found_exception();
	}
	return (int)env->CallIntMethod(instance_, mid);
}

long long jcGeneric::CallLong(const char* method_name)
{
	VERBOSE(qDebug("int jcGeneric::CallLong(const char* method_name) %p \"%s\"",this,method_name));
	JniEnvPtr jep;
	JNIEnv* env = jep.env();
	jmethodID mid = env->GetMethodID(class_, method_name, "()J");
	if (!mid)
	{
		qWarning("%s: method not found.", __FUNCTION__);
		throw method_not_found_exception();
	}
	return (long long)env->CallLongMethod(instance_, mid);
}

float jcGeneric::CallFloat(const char* method_name)
{
	VERBOSE(qDebug("float jcGeneric::CallFloat(const char* method_name) %p \"%s\"",this,method_name));
	JniEnvPtr jep;
	JNIEnv* env = jep.env();
	jmethodID mid = env->GetMethodID(class_, method_name, "()F");
	if (!mid)
	{
		qWarning("%s: method not found.", __FUNCTION__);
		throw method_not_found_exception();
	}
	return (float)env->CallFloatMethod(instance_, mid);
}

float jcGeneric::CallFloat(const char* method_name, int param)
{
	VERBOSE(qDebug("float jcGeneric::CallFloat(const char* method_name) %p \"%s\" (%d)",this,method_name, param));
	JniEnvPtr jep;
	JNIEnv* env = jep.env();
	jmethodID mid = env->GetMethodID(class_, method_name, "(I)F");
	if (!mid)
	{
		qWarning("%s: method not found.", __FUNCTION__);
		throw method_not_found_exception();
	}
	return (float)env->CallFloatMethod(instance_, mid, jint(param));
}

double jcGeneric::CallDouble(const char* method_name)
{
	VERBOSE(qDebug("float jcGeneric::CallDouble(const char* method_name) %p \"%s\"",this,method_name));
	JniEnvPtr jep;
	JNIEnv* env = jep.env();
	jmethodID mid = env->GetMethodID(class_, method_name, "()D");
	if (!mid)
	{
		qWarning("%s: method not found.", __FUNCTION__);
		throw method_not_found_exception();
	}
	return (double)env->CallDoubleMethod(instance_, mid);
}

jcGeneric* jcGeneric::CallObject(const char* method_name, const char* objname)
{
	std::string signature = "()L";
	signature += objname;
	signature += ";";

	VERBOSE(qDebug("jcGeneric::CallObject: \"%s\", \"%s\"", method_name, signature.c_str()));
	JniEnvPtr jep;
	JNIEnv* env = jep.env();
	jmethodID mid = env->GetMethodID(class_, method_name, signature.c_str());
	if (!mid)
	{
		qWarning("%s: method not found.", __FUNCTION__);
		throw method_not_found_exception();
	}
	jobject object = env->CallObjectMethod(instance_,mid);
	jcGeneric* result = new jcGeneric(object, true);
	return result;
}

void jcGeneric::CallStaticVoid(const char* method_name)
{
	VERBOSE(qDebug("void jcGeneric::CallStaticVoid(const char* method_name) %p \"%s\"", this, method_name));
	JniEnvPtr jep;
	JNIEnv* env = jep.env();
	jmethodID mid = env->GetStaticMethodID(class_, method_name, "()V");
	if (!mid)
	{
		qWarning("%s: method not found.", __FUNCTION__);
		throw method_not_found_exception();
	}
	env->CallStaticVoidMethod(class_, mid);
}

#ifdef QT_CORE_LIB
QString jcGeneric::CallString(const char *method_name)
{
	VERBOSE(qDebug("QString jcGeneric::CallString(const char* method_name) %p \"%s\"",this,method_name));
	JniEnvPtr jep;
	JNIEnv* env = jep.env();
	jmethodID mid = env->GetMethodID(class_, method_name, "()Ljava/lang/String;");
	if (!mid)
	{
		qWarning("%s: method not found.", __FUNCTION__);
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
	VERBOSE(qDebug("QString jcGeneric::CallStaticString(const char* method_name) %p \"%s\"",this,method_name));
	JniEnvPtr jep;
	JNIEnv* env = jep.env();
	jmethodID mid = env->GetStaticMethodID(class_, method_name, "()Ljava/lang/String;");
	if (!mid)
	{
		qWarning("%s: method not found.", __FUNCTION__);
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
		qWarning("%s: method not found.", __FUNCTION__);
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
	VERBOSE(qDebug("jcGeneric::CallStaticObject(\"%s\",\"%s\")", method_name, objname));
	std::string signature = "()L";
	signature += objname;
	signature += ";";

	VERBOSE(qDebug("jcGeneric::CallStaticObject signature: %s", signature.c_str()));
	JniEnvPtr jep;
	JNIEnv* env = jep.env();

	VERBOSE(qDebug("env->GetStaticMethodID"));
	jmethodID mid = env->GetStaticMethodID(class_, method_name, signature.c_str());
	if (!mid)
	{
		qWarning("%s: method not found.", __FUNCTION__);
		throw method_not_found_exception();
	}

	VERBOSE(qDebug("new jcGeneric(env->CallStaticObjectMethod(class_,mid), true);"));
	jobject jret = env->CallStaticObjectMethod(class_, mid);
	if (jret == 0)
	{
		return NULL;
	}
	return new jcGeneric(jret, true);
}

int jcGeneric::GetInt(const char* field_name)
{
	VERBOSE(qDebug("int jcGeneric::GetInt(const char* fieldd_name) %p \"%s\"",this,field_name));
	JniEnvPtr jep;
	JNIEnv* env = jep.env();
	jfieldID fid = env->GetFieldID(class_, field_name, "I");
	if (!fid)
	{
		qWarning("%s: method not found.", __FUNCTION__);
		throw field_not_found_exception();
	}
	return (int)env->GetIntField(instance_, fid);
}

// TODO: add something like what they do in jni.h:
//	#define CALL_TYPE_METHOD(_jtype, _jname)
//	_jtype Call##_jname##Method(jobject obj, jmethodID methodID, ...)
void jcGeneric::CallParamVoid(const char* method_name, const char* param_signature, ...)
{
	VERBOSE(qDebug("void jcGeneric(%p)::CallParamVoid(\"%s\", \"%s\", ...)", this, method_name, param_signature));

	va_list args;
	JniEnvPtr jep;
	JNIEnv* env = jep.env();

	std::string signature = "(";
	signature += param_signature;
	signature += ")V";
	jmethodID mid = env->GetMethodID(class_, method_name, signature.c_str());
	if (!mid)
	{
		qWarning("%s: method not found.", __FUNCTION__);
		throw method_not_found_exception();
	}

	va_start(args, param_signature);
	env->CallVoidMethodV(instance_, mid, args);
	va_end(args);

	if (env->ExceptionCheck())
	{
		qWarning("void jcGeneric(%p)::CallParamVoid(\"%s\", \"%s\", ...): exception occured", this, method_name, param_signature);
		env->ExceptionDescribe();
		env->ExceptionClear();
		return;
	}
}

void jcGeneric::CallVoid(const char * method_name, jint x)
{
	CallParamVoid(method_name, "I", x);
}

void jcGeneric::CallVoid(const char * method_name, jlong x)
{
	CallParamVoid(method_name, "J", x);
}

#ifdef QT_CORE_LIB

void jcGeneric::CallVoid(const char * method_name, const QString & string)
{
	JniEnvPtr jep;
	JNIEnv * env = jep.env();
	jstring js = QStringToJstring(env, string);
	CallParamVoid(method_name, "Ljava/lang/String;", js);
	env->DeleteLocalRef(js);
}

void jcGeneric::CallVoid(const char * method_name, const QString & string1, const QString & string2)
{
	JniEnvPtr jep;
	JNIEnv * env = jep.env();
	jstring js1 = QStringToJstring(env, string1);
	jstring js2 = QStringToJstring(env, string2);
	CallParamVoid(method_name, "Ljava/lang/String;Ljava/lang/String;", js1, js2);
	env->DeleteLocalRef(js1);
	env->DeleteLocalRef(js2);
}

void jcGeneric::CallVoid(const char * method_name, const QString & string1, const QString & string2, const QString & string3)
{
	JniEnvPtr jep;
	JNIEnv * env = jep.env();
	jstring js1 = QStringToJstring(env, string1);
	jstring js2 = QStringToJstring(env, string2);
	jstring js3 = QStringToJstring(env, string3);
	CallParamVoid(method_name, "Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;", js1, js2, js3);
	env->DeleteLocalRef(js1);
	env->DeleteLocalRef(js2);
	env->DeleteLocalRef(js3);
}

void jcGeneric::CallVoid(const char * method_name, const QString & string1, const QString & string2, const QString & string3, const QString & string4)
{
	JniEnvPtr jep;
	JNIEnv * env = jep.env();
	jstring js1 = QStringToJstring(env, string1);
	jstring js2 = QStringToJstring(env, string2);
	jstring js3 = QStringToJstring(env, string3);
	jstring js4 = QStringToJstring(env, string4);
	CallParamVoid(method_name, "Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;", js1, js2, js3, js4);
	env->DeleteLocalRef(js1);
	env->DeleteLocalRef(js2);
	env->DeleteLocalRef(js3);
	env->DeleteLocalRef(js4);
}

void jcGeneric::CallVoid(const char * method_name, const QString & string1, const QString & string2, const QString & string3, const QString & string4, const QString & string5)
{
	JniEnvPtr jep;
	JNIEnv * env = jep.env();
	jstring js1 = QStringToJstring(env, string1);
	jstring js2 = QStringToJstring(env, string2);
	jstring js3 = QStringToJstring(env, string3);
	jstring js4 = QStringToJstring(env, string4);
	jstring js5 = QStringToJstring(env, string5);
	CallParamVoid(method_name, "Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;", js1, js2, js3, js4, js5);
	env->DeleteLocalRef(js1);
	env->DeleteLocalRef(js2);
	env->DeleteLocalRef(js3);
	env->DeleteLocalRef(js4);
	env->DeleteLocalRef(js5);
}

#endif

bool jcGeneric::RegisterNativeMethod(const char* name, const char* signature, void* ptr)
{
	JNINativeMethod jnm = {name, signature, ptr};
	return RegisterNativeMethods(&jnm, sizeof(jnm));
}

bool jcGeneric::RegisterNativeMethods(const JNINativeMethod * methods_list, size_t sizeof_methods_list)
{
	JniEnvPtr jep;
	jint result = jep.env()->RegisterNatives(class_, methods_list, sizeof_methods_list/sizeof(JNINativeMethod));
	return result == 0;
}
