/*
  QJniHelpers library

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

#include <QJniHelpers.h>
#include <QAndroidQPAPluginGap.h>

#if defined(QJNIHELPERS_VERBOSE_LOG)
	#define VERBOSE(x) x
#else
	#define VERBOSE(x)
#endif

typedef std::map<std::string, jclass> PreloadedClasses;

class QJniEnvPtrThreadDetacher
{
public:
	~QJniEnvPtrThreadDetacher();
};

static JavaVM * g_JavaVm = 0;
static QMutex g_PreloadedClassesMutex;
static PreloadedClasses g_PreloadedClasses;
static QThreadStorage<QJniEnvPtrThreadDetacher*> g_JavaThreadDetacher;

QJniEnvPtrThreadDetacher::~QJniEnvPtrThreadDetacher()
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

QJniEnvPtr::QJniEnvPtr()
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
			g_JavaThreadDetacher.setLocalData(new QJniEnvPtrThreadDetacher());
		}
		VERBOSE(qDebug("Attached current thread %d successfully.", (int)gettid()));
	}
	else if (errsv != JNI_OK)
	{
		qWarning("Error getting Java environment: %d", errsv);
		return;
	}
}

QJniEnvPtr::QJniEnvPtr(JNIEnv* env)
	: env_(env)
{
	#if defined(Q_OS_ANDROID) || defined(ANDROID)
		AutoSetJavaVM();
	#endif
}

QJniEnvPtr::~QJniEnvPtr()
{
}

JNIEnv* QJniEnvPtr::env() const
{
	return env_;
}

bool QJniEnvPtr::preloadClass(const char* class_name)
{
	qWarning("Preloading class \"%s\"", class_name);
	jclass clazz = env_->FindClass(class_name);
	if (clearException())
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

int QJniEnvPtr::preloadClasses(const char* const* class_list)
{
	int loaded = 0;
	for(; *class_list != 0; ++class_list)
	{
		if (!preloadClass(*class_list))
		{
			break;
		}
		++loaded;
	}
	return loaded;
}

bool QJniEnvPtr::isClassPreloaded(const char * class_name)
{
	QMutexLocker locker(&g_PreloadedClassesMutex);
	return g_PreloadedClasses.find(class_name) != g_PreloadedClasses.end();
}

void QJniEnvPtr::unloadAllClasses()
{
	QMutexLocker locker(&g_PreloadedClassesMutex);
	PreloadedClasses::iterator it;
	for(it = g_PreloadedClasses.begin(); it != g_PreloadedClasses.end(); ++it)
	{
		env_->DeleteGlobalRef((*it).second);
	}
	g_PreloadedClasses.clear();
}

jclass QJniEnvPtr::findClass(const char * name)
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
		#if defined(QJNIHELPERS_VERBOSE_LOG)
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
	if (clearException())
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

JavaVM * QJniEnvPtr::getJavaVM() const
{
	return g_JavaVm;
}

void QJniEnvPtr::setJavaVM(JavaVM* vm)
{
	// qDebug()<<"SetJavaVM"<<vm;
	g_JavaVm = vm;
}

void QJniEnvPtr::setJavaVM(JNIEnv* env)
{
	env->GetJavaVM(&g_JavaVm);
}

jstring QJniEnvPtr::JStringFromQString(const QString& str)
{
	jstring ret = env_->NewString(str.utf16(), str.length());
	if (clearException())
	{
		qWarning("Failed to convert QString to jstring.");
		return 0;
	}
	return ret;
}

QString QJniEnvPtr::QStringFromJString(jstring str)
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

bool QJniEnvPtr::clearException(bool describe)
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








const char * QJniObject::MethodNotFoundException::what() const throw()
{
	return "Java method not found";
}

const char * QJniObject::FieldNotFoundException::what() const throw()
{
	return "Java field not found";
}

QJniObject::QJniObject(jobject instance, bool take_ownership)
	: instance_(0)
	, class_(0)
{
	QJniEnvPtr jep;
	// Создаёт глобальную ссылку
	init(jep.env(), instance);
	if (take_ownership)
	{
		jep.env()->DeleteLocalRef(instance);
	}
}

QJniObject::QJniObject(jclass clazz, bool create)
	: instance_(0)
	, class_(0)
{
	QJniEnvPtr jep;
	init(jep.env(), clazz, create);
}

QJniObject::QJniObject(const char* full_class_name, bool create)
	: instance_(0)
	, class_(0)
{
	VERBOSE(qDebug()<<"Grym"<<__FILE__<<__LINE__);
	QJniEnvPtr jep;
	VERBOSE(qDebug()<<"Grym"<<__FILE__<<__LINE__);
	init(jep.env(), full_class_name, create);
	VERBOSE(qDebug()<<"Grym"<<__FILE__<<__LINE__);
}

QJniObject::QJniObject()
	: instance_(0)
	, class_(0)
{
}

QJniObject::~QJniObject()
{
	VERBOSE(qDebug("QJniObject::~QJniObject() %p",this));
	QJniEnvPtr jep;
	if (instance_ != NULL)
	{
		jep.env()->DeleteGlobalRef(instance_);
	}
	if (class_ != NULL)
	{
		jep.env()->DeleteGlobalRef(class_);
	}
}

jobject QJniObject::takeJobjectOver()
{
	jobject ret = instance_;
	instance_ = 0;
	return ret;
}

void QJniObject::init(JNIEnv* env, jobject instance)
{
	VERBOSE(qDebug("QJniObject::init(JNIEnv* env, jobject instance) %p", this));
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

void QJniObject::init(JNIEnv* env, jclass class_to_instantiate, bool create)
{
	VERBOSE(qDebug("QJniObject::init(JNIEnv* env, jclass class_to_instantiate, %d) %p", (int)create, this));

	if( create )
	{
		// get constructor method id. FIXME: exceptions
		jmethodID mid_init = env->GetMethodID(class_to_instantiate, "<init>", "()V");
		if (!mid_init)
		{
			qWarning("%s: method not found.", __FUNCTION__);
			throw MethodNotFoundException();
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

void QJniObject::init(JNIEnv* env, const char * full_class_name, bool create)
{
	VERBOSE(qDebug("QJniObject::init(JNIEnv* env, const char* full_class_name) %p: %s, create: %d", this, full_class_name, (int)create));
	// FIXME: exceptions
	QJniEnvPtr jep(env);
	jclass cls = jep.findClass(full_class_name); // this is a preloaded global ref, we don't need to delete it as a local ref
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

void QJniObject::callVoid(const char* method_name)
{
	VERBOSE(qDebug("void QJniObject::CallVoid(const char* method_name) %p \"%s\"",this,method_name));
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();
	jmethodID mid = env->GetMethodID(class_, method_name, "()V");
	if (!mid)
	{
		qWarning("%s: method not found.", __FUNCTION__);
		throw MethodNotFoundException();
	}
	env->CallVoidMethod(instance_, mid);
}

bool QJniObject::callBool(const char* method_name)
{
	VERBOSE(qDebug("bool QJniObject::CallBool(const char* method_name) %p \"%s\"",this,method_name));
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();
	jmethodID mid = env->GetMethodID(class_, method_name, "()Z");
	if (!mid)
	{
		qWarning("%s: method not found.", __FUNCTION__);
		throw MethodNotFoundException();
	}
	return (JNI_TRUE==env->CallBooleanMethod(instance_, mid));
}

bool QJniObject::callBool(const char* method_name, bool param)
{
	VERBOSE(qDebug("bool QJniObject::CallBool(const char* method_name, bool param) %p \"%s\"",this,method_name));
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();
	jmethodID mid = env->GetMethodID(class_, method_name, "(Z)Z");
	if (!mid)
	{
		qWarning("%s: method not found.", __FUNCTION__);
		throw MethodNotFoundException();
	}
	return (JNI_TRUE==env->CallBooleanMethod(instance_, mid, jboolean(param)));
}

int QJniObject::callInt(const char* method_name)
{
	VERBOSE(qDebug("int QJniObject::CallInt(const char* method_name) %p \"%s\"",this,method_name));
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();
	jmethodID mid = env->GetMethodID(class_, method_name, "()I");
	if (!mid)
	{
		qWarning("%s: method not found.", __FUNCTION__);
		throw MethodNotFoundException();
	}
	return (int)env->CallIntMethod(instance_, mid);
}

long long QJniObject::callLong(const char* method_name)
{
	VERBOSE(qDebug("int QJniObject::CallLong(const char* method_name) %p \"%s\"",this,method_name));
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();
	jmethodID mid = env->GetMethodID(class_, method_name, "()J");
	if (!mid)
	{
		qWarning("%s: method not found.", __FUNCTION__);
		throw MethodNotFoundException();
	}
	return (long long)env->CallLongMethod(instance_, mid);
}

float QJniObject::callFloat(const char* method_name)
{
	VERBOSE(qDebug("float QJniObject::CallFloat(const char* method_name) %p \"%s\"",this,method_name));
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();
	jmethodID mid = env->GetMethodID(class_, method_name, "()F");
	if (!mid)
	{
		qWarning("%s: method not found.", __FUNCTION__);
		throw MethodNotFoundException();
	}
	return (float)env->CallFloatMethod(instance_, mid);
}

float QJniObject::callFloat(const char* method_name, int param)
{
	VERBOSE(qDebug("float QJniObject::CallFloat(const char* method_name) %p \"%s\" (%d)",this,method_name, param));
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();
	jmethodID mid = env->GetMethodID(class_, method_name, "(I)F");
	if (!mid)
	{
		qWarning("%s: method not found.", __FUNCTION__);
		throw MethodNotFoundException();
	}
	return (float)env->CallFloatMethod(instance_, mid, jint(param));
}

double QJniObject::callDouble(const char* method_name)
{
	VERBOSE(qDebug("float QJniObject::CallDouble(const char* method_name) %p \"%s\"",this,method_name));
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();
	jmethodID mid = env->GetMethodID(class_, method_name, "()D");
	if (!mid)
	{
		qWarning("%s: method not found.", __FUNCTION__);
		throw MethodNotFoundException();
	}
	return (double)env->CallDoubleMethod(instance_, mid);
}

QJniObject* QJniObject::callObject(const char* method_name, const char* objname)
{
	std::string signature = "()L";
	signature += objname;
	signature += ";";

	VERBOSE(qDebug("QJniObject::CallObject: \"%s\", \"%s\"", method_name, signature.c_str()));
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();
	jmethodID mid = env->GetMethodID(class_, method_name, signature.c_str());
	if (!mid)
	{
		qWarning("%s: method not found.", __FUNCTION__);
		throw MethodNotFoundException();
	}
	jobject object = env->CallObjectMethod(instance_,mid);
	QJniObject* result = new QJniObject(object, true);
	return result;
}

void QJniObject::callStaticVoid(const char* method_name)
{
	VERBOSE(qDebug("void QJniObject::CallStaticVoid(const char* method_name) %p \"%s\"", this, method_name));
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();
	jmethodID mid = env->GetStaticMethodID(class_, method_name, "()V");
	if (!mid)
	{
		qWarning("%s: method not found.", __FUNCTION__);
		throw MethodNotFoundException();
	}
	env->CallStaticVoidMethod(class_, mid);
}

void QJniObject::callStaticParamVoid(const char* method_name, const char * param_signature, ...)
{
	VERBOSE(qDebug("void QJniObject(%p)::CallParamVoid(\"%s\", \"%s\", ...)", this, method_name, param_signature));

	va_list args;
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();

	std::string signature = "(";
	signature += param_signature;
	signature += ")V";
	jmethodID mid = env->GetStaticMethodID(class_, method_name, signature.c_str());
	if (!mid)
	{
		qWarning("%s: method not found.", __FUNCTION__);
		throw MethodNotFoundException();
	}

	va_start(args, param_signature);
	env->CallStaticVoidMethodV(class_, mid, args);
	va_end(args);

	if (env->ExceptionCheck())
	{
		qWarning("void QJniObject(%p)::CallParamVoid(\"%s\", \"%s\", ...): exception occured", this, method_name, param_signature);
		env->ExceptionDescribe();
		env->ExceptionClear();
		return;
	}
}

void QJniObject::callStaticVoid(const char* method_name, const QString & string)
{
	QJniEnvPtr jep;
	JNIEnv * env = jep.env();
	jstring js = jep.JStringFromQString(string);
	callStaticParamVoid(method_name, "Ljava/lang/String;", js);
	env->DeleteLocalRef(js);
}

QString QJniObject::callString(const char *method_name)
{
	VERBOSE(qDebug("QString QJniObject::CallString(const char* method_name) %p \"%s\"",this,method_name));
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();
	jmethodID mid = env->GetMethodID(class_, method_name, "()Ljava/lang/String;");
	if (!mid)
	{
		qWarning("%s: method not found.", __FUNCTION__);
		throw MethodNotFoundException();
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

QString QJniObject::callStaticString(const char *method_name)
{
	VERBOSE(qDebug("QString QJniObject::CallStaticString(const char* method_name) %p \"%s\"",this,method_name));
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();
	jmethodID mid = env->GetStaticMethodID(class_, method_name, "()Ljava/lang/String;");
	if (!mid)
	{
		qWarning("%s: method not found.", __FUNCTION__);
		throw MethodNotFoundException();
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

QString QJniObject::getString(const char *field_name)
{
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();
	jfieldID fid = env->GetFieldID(class_, field_name, "Ljava/lang/String;");
	if (!fid)
	{
		qWarning("%s: method not found.", __FUNCTION__);
		throw FieldNotFoundException();
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

QJniObject* QJniObject::callStaticObject(const char *method_name, const char *objname)
{
	VERBOSE(qDebug("QJniObject::CallStaticObject(\"%s\",\"%s\")", method_name, objname));
	std::string signature = "()L";
	signature += objname;
	signature += ";";

	VERBOSE(qDebug("QJniObject::CallStaticObject signature: %s", signature.c_str()));
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();

	VERBOSE(qDebug("env->GetStaticMethodID"));
	jmethodID mid = env->GetStaticMethodID(class_, method_name, signature.c_str());
	if (!mid)
	{
		qWarning("%s: method not found.", __FUNCTION__);
		throw MethodNotFoundException();
	}

	VERBOSE(qDebug("new QJniObject(env->CallStaticObjectMethod(class_,mid), true);"));
	jobject jret = env->CallStaticObjectMethod(class_, mid);
	if (jret == 0)
	{
		return NULL;
	}
	return new QJniObject(jret, true);
}

int QJniObject::getIntField(const char* field_name)
{
	VERBOSE(qDebug("int QJniObject::GetInt(const char* fieldd_name) %p \"%s\"",this,field_name));
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();
	jfieldID fid = env->GetFieldID(class_, field_name, "I");
	if (!fid)
	{
		qWarning("%s: method not found.", __FUNCTION__);
		throw FieldNotFoundException();
	}
	return (int)env->GetIntField(instance_, fid);
}

// TODO: add something like what they do in jni.h:
//	#define CALL_TYPE_METHOD(_jtype, _jname)
//	_jtype Call##_jname##Method(jobject obj, jmethodID methodID, ...)
void QJniObject::callParamVoid(const char* method_name, const char* param_signature, ...)
{
	VERBOSE(qDebug("void QJniObject(%p)::CallParamVoid(\"%s\", \"%s\", ...)", this, method_name, param_signature));

	va_list args;
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();

	std::string signature = "(";
	signature += param_signature;
	signature += ")V";
	jmethodID mid = env->GetMethodID(class_, method_name, signature.c_str());
	if (!mid)
	{
		qWarning("%s: method not found.", __FUNCTION__);
		throw MethodNotFoundException();
	}

	va_start(args, param_signature);
	env->CallVoidMethodV(instance_, mid, args);
	va_end(args);

	if (env->ExceptionCheck())
	{
		qWarning("void QJniObject(%p)::CallParamVoid(\"%s\", \"%s\", ...): exception occured", this, method_name, param_signature);
		env->ExceptionDescribe();
		env->ExceptionClear();
		return;
	}
}

void QJniObject::callVoid(const char * method_name, jint x)
{
	callParamVoid(method_name, "I", x);
}

void QJniObject::callVoid(const char * method_name, jlong x)
{
	callParamVoid(method_name, "J", x);
}

void QJniObject::callVoid(const char * method_name, jboolean x)
{
	callParamVoid(method_name, "Z", x);
}

void QJniObject::callVoid(const char * method_name, const QString & string)
{
	QJniEnvPtr jep;
	JNIEnv * env = jep.env();
	jstring js = jep.JStringFromQString(string);
	callParamVoid(method_name, "Ljava/lang/String;", js);
	env->DeleteLocalRef(js);
}

void QJniObject::callVoid(const char * method_name, const QString & string1, const QString & string2)
{
	QJniEnvPtr jep;
	JNIEnv * env = jep.env();
	jstring js1 = jep.JStringFromQString(string1);
	jstring js2 = jep.JStringFromQString(string2);
	callParamVoid(method_name, "Ljava/lang/String;Ljava/lang/String;", js1, js2);
	env->DeleteLocalRef(js1);
	env->DeleteLocalRef(js2);
}

void QJniObject::callVoid(const char * method_name, const QString & string1, const QString & string2, const QString & string3)
{
	QJniEnvPtr jep;
	JNIEnv * env = jep.env();
	jstring js1 = jep.JStringFromQString(string1);
	jstring js2 = jep.JStringFromQString(string2);
	jstring js3 = jep.JStringFromQString(string3);
	callParamVoid(method_name, "Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;", js1, js2, js3);
	env->DeleteLocalRef(js1);
	env->DeleteLocalRef(js2);
	env->DeleteLocalRef(js3);
}

void QJniObject::callVoid(const char * method_name, const QString & string1, const QString & string2, const QString & string3, const QString & string4)
{
	QJniEnvPtr jep;
	JNIEnv * env = jep.env();
	jstring js1 = jep.JStringFromQString(string1);
	jstring js2 = jep.JStringFromQString(string2);
	jstring js3 = jep.JStringFromQString(string3);
	jstring js4 = jep.JStringFromQString(string4);
	callParamVoid(method_name, "Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;", js1, js2, js3, js4);
	env->DeleteLocalRef(js1);
	env->DeleteLocalRef(js2);
	env->DeleteLocalRef(js3);
	env->DeleteLocalRef(js4);
}

void QJniObject::callVoid(const char * method_name, const QString & string1, const QString & string2, const QString & string3, const QString & string4, const QString & string5)
{
	QJniEnvPtr jep;
	JNIEnv * env = jep.env();
	jstring js1 = jep.JStringFromQString(string1);
	jstring js2 = jep.JStringFromQString(string2);
	jstring js3 = jep.JStringFromQString(string3);
	jstring js4 = jep.JStringFromQString(string4);
	jstring js5 = jep.JStringFromQString(string5);
	callParamVoid(method_name, "Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;", js1, js2, js3, js4, js5);
	env->DeleteLocalRef(js1);
	env->DeleteLocalRef(js2);
	env->DeleteLocalRef(js3);
	env->DeleteLocalRef(js4);
	env->DeleteLocalRef(js5);
}

bool QJniObject::registerNativeMethod(const char* name, const char* signature, void* ptr)
{
	JNINativeMethod jnm = {name, signature, ptr};
	return registerNativeMethods(&jnm, sizeof(jnm));
}

bool QJniObject::registerNativeMethods(const JNINativeMethod * methods_list, size_t sizeof_methods_list)
{
	QJniEnvPtr jep;
	jint result = jep.env()->RegisterNatives(class_, methods_list, sizeof_methods_list/sizeof(JNINativeMethod));
	return result == 0;
}
