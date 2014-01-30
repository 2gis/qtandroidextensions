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

#include <unistd.h>
#include <sys/types.h>
#include <QJniHelpers.h>
#include <QAndroidQPAPluginGap.h>

#if defined(QJNIHELPERS_VERBOSE_LOG)
	#define VERBOSE(x) x
#else
	#define VERBOSE(x)
#endif

typedef QMap<QString, jclass> PreloadedClasses;

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




const char * QJniBaseException::what() const throw()
{
	return "JNI: Java exception.";
}

const char * QJniThreadAttachException::what() const throw()
{
	return "JNI: Thread attaching exception.";
}

const char * QJniClassNotFoundException::what() const throw()
{
	return "JNI: Java clss not found.";
}

const char * QJniMethodNotFoundException::what() const throw()
{
	return "JNI: Java method not found.";
}

const char * QJniFieldNotFoundException::what() const throw()
{
	return "JNI: Java field not found.";
}

const char * QJniJavaCallException::what() const throw()
{
	return "JNI: Java method raised an unhandled exception.";
}




QJniEnvPtr::QJniEnvPtr(JNIEnv * env)
	: env_(env)
{
	#if defined(Q_OS_ANDROID) || defined(ANDROID)
		AutoSetJavaVM();
	#endif
	if (g_JavaVm == 0)
	{
		qWarning("Java VM pointer is not set!");
		throw QJniThreadAttachException();
	}
	if (!env_)
	{
		int errsv = g_JavaVm->GetEnv((void**)(&env_), JNI_VERSION_1_6);
		if (errsv == JNI_EDETACHED)
		{
			VERBOSE(qDebug("Current thread %d is not attached, attaching it...", (int)gettid()));
			errsv = g_JavaVm->AttachCurrentThread(&env_, 0);
			if (errsv != 0)
			{
				qWarning("Error attaching current thread %d: %d", (int)gettid(), errsv);
				throw QJniThreadAttachException();
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
			throw QJniThreadAttachException();
		}
	}
}

QJniEnvPtr::~QJniEnvPtr()
{
}

JNIEnv* QJniEnvPtr::env() const
{
	return env_;
}

bool QJniEnvPtr::preloadClass(const char * class_name)
{
	qWarning("Preloading class \"%s\"", class_name);
	QJniLocalRef clazz(env_, env_->FindClass(class_name)); // jclass
	if (clearException() || clazz.jObject() == NULL)
	{
		qWarning("...Failed to preload class %s (tid %d)", class_name, (int)gettid());
		return false;
	}
	jclass gclazz = (jclass)env_->NewGlobalRef(clazz);
	QMutexLocker locker(&g_PreloadedClassesMutex);
	g_PreloadedClasses.insert(QLatin1String(class_name), gclazz);
	VERBOSE(qDebug("...Preloaded class \"%s\" as %p for tid %d", class_name, gclazz, (int)gettid()));
	return true;
}

int QJniEnvPtr::preloadClasses(const char * const * class_list)
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
	return g_PreloadedClasses.contains(QLatin1String(class_name));
}

void QJniEnvPtr::unloadAllClasses()
{
	QMutexLocker locker(&g_PreloadedClassesMutex);
	for (PreloadedClasses::iterator it = g_PreloadedClasses.begin(); it != g_PreloadedClasses.end(); ++it)
	{
		env_->DeleteGlobalRef(it.value());
	}
	g_PreloadedClasses.clear();
}

jclass QJniEnvPtr::findClass(const char * name)
{
	// First try find a preloaded class
	{
		QMutexLocker locker(&g_PreloadedClassesMutex);
		VERBOSE(qDebug("Searching for class \"%s\" in tid %d", name, (int)gettid()));
		PreloadedClasses::iterator it = g_PreloadedClasses.find(QLatin1String(name));
		if (it != g_PreloadedClasses.end())
		{
			return it.value();
		}
	}

	// If it wasn't preloaded, try to load it in JNI (will fail for custom classes in native-created threads)
	VERBOSE(qDebug("Trying to construct the class directly: \"%s\" in tid %d", name, (int)gettid()));
	QJniLocalRef cls(env_, env_->FindClass(name)); // jclass
	if (clearException())
	{
		qWarning("Failed to find class \"%s\"", name);
		return 0;
	}

	// We must store class ref in a global ref
	jclass ret = (jclass)env_->NewGlobalRef(cls);

	// Add it to a list of preloaded classes for convenience
	QMutexLocker locker(&g_PreloadedClassesMutex);
	g_PreloadedClasses.insert(QLatin1String(name), ret);

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
	QJniEnvPtr jep(env);

	instance_ = env->NewGlobalRef(instance);
	if (jep.clearException())
	{
		throw QJniBaseException();
	}

	// Note: class is expected to be a valid ref during the whole lifetime of the instance_ object.
	QJniLocalRef clazz(env, env->GetObjectClass(instance_));

	class_ = static_cast<jclass>(env->NewGlobalRef(clazz));

	if (jep.clearException())
	{
		throw QJniBaseException();
	}
}

void QJniObject::init(JNIEnv * env, jclass class_to_instantiate, bool create)
{
	VERBOSE(qDebug("QJniObject::init(JNIEnv* env, jclass class_to_instantiate, %d) %p", (int)create, this));

	if( create )
	{
		QJniEnvPtr jep(env);
		jmethodID mid_init = env->GetMethodID(class_to_instantiate, "<init>", "()V");
		if (jep.clearException())
		{
			throw QJniBaseException();
		}
		if (!mid_init)
		{
			qWarning("%s: method not found.", __FUNCTION__);
			throw QJniMethodNotFoundException();
		}

		QJniLocalRef obj(env, env->NewObject(class_to_instantiate, mid_init));
		if (jep.clearException())
		{
			throw QJniBaseException();
		}

		// it is dangerous to go alone, use this
		init(env, obj);
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
	QJniEnvPtr jep(env);
	jclass cls = jep.findClass(full_class_name); // this is a preloaded global ref, we don't need to delete it as a local ref
	if (jep.clearException())
	{
		throw QJniBaseException();
	}
	if (cls == NULL)
	{
		qWarning("Could not find class %s", full_class_name);
		throw QJniClassNotFoundException();
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
		throw QJniMethodNotFoundException();
	}
	env->CallVoidMethod(instance_, mid);
	if (jep.clearException())
	{
		throw QJniJavaCallException();
	}
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
		throw QJniMethodNotFoundException();
	}
	bool result = (JNI_TRUE==env->CallBooleanMethod(instance_, mid));
	if (jep.clearException())
	{
		throw QJniJavaCallException();
	}
	return result;
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
		throw QJniMethodNotFoundException();
	}
	bool result = (JNI_TRUE==env->CallBooleanMethod(instance_, mid, jboolean(param)));
	if (jep.clearException())
	{
		throw QJniJavaCallException();
	}
	return result;
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
		throw QJniMethodNotFoundException();
	}
	int result = (int)env->CallIntMethod(instance_, mid);
	if (jep.clearException())
	{
		throw QJniJavaCallException();
	}
	return result;
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
		throw QJniMethodNotFoundException();
	}
	long long result = (long long)env->CallLongMethod(instance_, mid);
	if (jep.clearException())
	{
		throw QJniJavaCallException();
	}
	return result;
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
		throw QJniMethodNotFoundException();
	}
	float result = (float)env->CallFloatMethod(instance_, mid);
	if (jep.clearException())
	{
		throw QJniJavaCallException();
	}
	return result;
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
		throw QJniMethodNotFoundException();
	}
	float result = (float)env->CallFloatMethod(instance_, mid, jint(param));
	if (jep.clearException())
	{
		throw QJniJavaCallException();
	}
	return result;
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
		throw QJniMethodNotFoundException();
	}
	double result = (double)env->CallDoubleMethod(instance_, mid);
	if (jep.clearException())
	{
		throw QJniJavaCallException();
	}
	return result;
}

QJniObject * QJniObject::callObject(const char * method_name, const char * objname)
{
	QByteArray signature("()L");
	signature += objname;
	signature += ";";
	VERBOSE(qDebug("QJniObject::CallObject: \"%s\", \"%s\"", method_name, signature.data()));
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();
	jmethodID mid = env->GetMethodID(class_, method_name, signature.data());
	if (!mid)
	{
		qWarning("%s: method not found.", __FUNCTION__);
		throw QJniMethodNotFoundException();
	}
	jobject object = env->CallObjectMethod(instance_,mid);
	if (jep.clearException())
	{
		throw QJniJavaCallException();
	}
	QJniObject * result = new QJniObject(object, true);
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
		throw QJniMethodNotFoundException();
	}
	env->CallStaticVoidMethod(class_, mid);
	if (jep.clearException())
	{
		throw QJniJavaCallException();
	}
}

void QJniObject::callStaticParamVoid(const char * method_name, const char * param_signature, ...)
{
	VERBOSE(qDebug("void QJniObject(%p)::CallParamVoid(\"%s\", \"%s\", ...)", this, method_name, param_signature));

	va_list args;
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();

	QByteArray signature("(");
	signature += param_signature;
	signature += ")V";
	jmethodID mid = env->GetStaticMethodID(class_, method_name, signature.data());
	if (!mid)
	{
		qWarning("%s: method not found.", __FUNCTION__);
		throw QJniMethodNotFoundException();
	}

	va_start(args, param_signature);
	env->CallStaticVoidMethodV(class_, mid, args);
	va_end(args);
	if (jep.clearException())
	{
		qWarning("void QJniObject(%p)::CallParamVoid(\"%s\", \"%s\", ...): exception occured", this, method_name, param_signature);
		throw QJniJavaCallException();
	}
}

void QJniObject::callStaticVoid(const char* method_name, const QString & string)
{
	callStaticParamVoid(method_name, "Ljava/lang/String;", QJniLocalRef(string).jObject());
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
		throw QJniMethodNotFoundException();
	}
	QString ret = QJniLocalRef(env, env->CallObjectMethod(instance_, mid));
	if (jep.clearException())
	{
		throw QJniJavaCallException();
	}
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
		throw QJniMethodNotFoundException();
	}
	QString ret = QJniLocalRef(env, env->CallStaticObjectMethod(class_, mid));
	if (jep.clearException())
	{
		throw QJniJavaCallException();
	}
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
		throw QJniFieldNotFoundException();
	}
	QString ret = QJniLocalRef(env, env->GetObjectField(instance_, fid));
	if (jep.clearException())
	{
		throw QJniJavaCallException();
	}
	return ret;
}

QJniObject* QJniObject::callStaticObject(const char * method_name, const char * objname)
{
	VERBOSE(qDebug("QJniObject::CallStaticObject(\"%s\",\"%s\")", method_name, objname));
	QByteArray signature("()L");
	signature += objname;
	signature += ";";

	VERBOSE(qDebug("QJniObject::CallStaticObject signature: %s", signature.data()));
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();

	VERBOSE(qDebug("env->GetStaticMethodID"));
	jmethodID mid = env->GetStaticMethodID(class_, method_name, signature.data());
	if (!mid)
	{
		qWarning("%s: method not found.", __FUNCTION__);
		throw QJniMethodNotFoundException();
	}

	VERBOSE(qDebug("new QJniObject(env->CallStaticObjectMethod(class_,mid), true);"));
	jobject jret = env->CallStaticObjectMethod(class_, mid);
	if (jep.clearException())
	{
		throw QJniJavaCallException();
	}
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
		throw QJniFieldNotFoundException();
	}
	int result = (int)env->GetIntField(instance_, fid);
	if (jep.clearException())
	{
		throw QJniJavaCallException();
	}
	return result;
}

// TODO: add something like what they do in jni.h:
//	#define CALL_TYPE_METHOD(_jtype, _jname)
//	_jtype Call##_jname##Method(jobject obj, jmethodID methodID, ...)
void QJniObject::callParamVoid(const char * method_name, const char * param_signature, ...)
{
	VERBOSE(qDebug("void QJniObject(%p)::CallParamVoid(\"%s\", \"%s\", ...)", this, method_name, param_signature));

	va_list args;
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();

	QByteArray signature("(");
	signature += param_signature;
	signature += ")V";
	jmethodID mid = env->GetMethodID(class_, method_name, signature.data());
	if (!mid)
	{
		qWarning("%s: method not found.", __FUNCTION__);
		throw QJniMethodNotFoundException();
	}

	va_start(args, param_signature);
	env->CallVoidMethodV(instance_, mid, args);
	va_end(args);

	if (jep.clearException())
	{
		qWarning("void QJniObject(%p)::CallParamVoid(\"%s\", \"%s\", ...): exception occured", this, method_name, param_signature);
		throw QJniJavaCallException();
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
	callParamVoid(method_name, "Ljava/lang/String;", QJniLocalRef(string).jObject());
}

void QJniObject::callVoid(const char * method_name, const QString & string1, const QString & string2)
{
	QJniEnvPtr jep;
	callParamVoid(method_name, "Ljava/lang/String;Ljava/lang/String;",
		QJniLocalRef(jep, string1).jObject(), QJniLocalRef(jep, string2).jObject());
}

void QJniObject::callVoid(const char * method_name, const QString & string1, const QString & string2, const QString & string3)
{
	QJniEnvPtr jep;
	callParamVoid(method_name, "Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;",
		QJniLocalRef(jep, string1).jObject(), QJniLocalRef(jep, string2).jObject(), QJniLocalRef(jep, string3).jObject());
}

void QJniObject::callVoid(const char * method_name, const QString & string1, const QString & string2, const QString & string3, const QString & string4)
{
	QJniEnvPtr jep;
	callParamVoid(method_name, "Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;",
		QJniLocalRef(jep, string1).jObject(), QJniLocalRef(jep, string2).jObject(), QJniLocalRef(jep, string3).jObject(), QJniLocalRef(jep, string4).jObject());
}

void QJniObject::callVoid(const char * method_name, const QString & string1, const QString & string2, const QString & string3, const QString & string4, const QString & string5)
{
	QJniEnvPtr jep;
	callParamVoid(method_name, "Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;",
		QJniLocalRef(jep, string1).jObject(), QJniLocalRef(jep, string2).jObject(), QJniLocalRef(jep, string3).jObject(),
		QJniLocalRef(jep, string4).jObject(), QJniLocalRef(jep, string5).jObject());
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
	if (jep.clearException())
	{
		throw QJniJavaCallException();
	}
	return result == 0;
}



QJniLocalRef::~QJniLocalRef()
{
	if (local_)
	{
		if (!env_)
		{
			QJniEnvPtr jep;
			env_ = jep.env();
		}
		if (env_)
		{
			env_->DeleteLocalRef(local_);
		}
	}
}

QJniLocalRef::QJniLocalRef(const QString & string)
	: local_(0), env_(0)
{
	QJniEnvPtr jep;
	local_ = jep.JStringFromQString(string);
	env_ = jep.env();
}

QJniLocalRef::QJniLocalRef(QJniEnvPtr & env, const QString & string)
	: local_(env.JStringFromQString(string)), env_(env.env())
{
}
