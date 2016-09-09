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
#include "QJniHelpers.h"
#include "QAndroidQPAPluginGap.h"

#if defined(QJNIHELPERS_VERBOSE_LOG)
	#define VERBOSE(x) x
#else
	#define VERBOSE(x)
#endif

/////////////////////////////////////////////////////////////////////////////
// Private Stuff
/////////////////////////////////////////////////////////////////////////////

namespace {

//! Data type to keep list of JNI references to Java classes ever preloaded or loaded.
typedef QMap<QString, jclass> PreloadedClasses;

//! QThreadStorage object to detach thread from JNI when it's finished and prevent Java reference leak.
class QJniEnvPtrThreadDetacher
{
public:
	~QJniEnvPtrThreadDetacher();
};

/*!
 * Static object to free Java class references upon deinitalization of the module. This is not totally
 * necessary in Android as whold Java machine is destroyed when application exists and the reference
 * leak is not an issue, but let's just be on the good side.
 */
class QJniClassUnloader
{
public:
	~QJniClassUnloader()
	{
		QJniEnvPtr().unloadAllClasses();
	}
};

static JavaVM * g_JavaVm = 0;
static QMutex g_PreloadedClassesMutex;
static PreloadedClasses g_PreloadedClasses;
static QThreadStorage<QJniEnvPtrThreadDetacher*> g_JavaThreadDetacher;
static QJniClassUnloader g_class_unloader_;

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

// Historically, we allow to specify returing object types like this: "java/lang/String".
// It is converted to "Ljava/lang/String;" for JNI signatures automatically.
// However, sometimes it is desirable to pass the type as "Ljava/lang/String;"
// and also we may need to pass an array name: "[F" (which means float[] in JNI)
// and always adding the L...; around would break it.
static inline void appendNormalizedObjectName(
	QByteArray & out_signature
	, const char * objname)
{
	if (size_t length = strlen(objname))
	{
		if (objname[0] != '[' && objname[length-1] != ';')
		{
			out_signature.append('L');
			out_signature.append(objname, length);
			out_signature.append(';');
		}
		else
		{
			out_signature.append(objname, length);
		}
	}
}

static inline void makeObjectFunctionSignature(
	QByteArray & out_signature
	, const char * param_signature
	, const char * returning_objname)
{
	out_signature.append('(');
	out_signature.append(param_signature);
	out_signature.append(')');
	appendNormalizedObjectName(out_signature, returning_objname);
}

static inline bool classObjectMayHaveNullClass(const char * class_name)
{
	if (!class_name)
	{
		return true;
	}
	return class_name[0] == '[';
}

} // anonymous namespace


/////////////////////////////////////////////////////////////////////////////
// Exceptions
/////////////////////////////////////////////////////////////////////////////

QJniBaseException::QJniBaseException(const char * message)
	: message_(message? message: "JNI: Java exception.")
{
	qDebug() << "QJniHelpers: throwing an exception:" << what();
}

const char * QJniBaseException::what() const throw()
{
	return message_;
}

QJniThreadAttachException::QJniThreadAttachException()
	: QJniBaseException("JNI: Thread attaching exception.")
{
}

QJniClassNotFoundException::QJniClassNotFoundException()
	: QJniBaseException("JNI: Java class not found.")
{
}

QJniClassNotSetException::QJniClassNotSetException()
	: QJniBaseException("JNI: Java class is null.")
{
}

QJniMethodNotFoundException::QJniMethodNotFoundException()
	: QJniBaseException("JNI: Java method not found.")
{
}

QJniFieldNotFoundException::QJniFieldNotFoundException()
	: QJniBaseException("JNI: Java field not found.")
{
}

QJniJavaCallException::QJniJavaCallException(const char * callDetails, const char * callDetailsMore)
	: QJniBaseException("JNI: Java method raised an unhandled exception.")
{
	qDebug() << "QJniHelpers: QJniJavaCallException with" << callDetails << "and" <<  callDetailsMore;
}

QJniJavaCallException::QJniJavaCallException(const char * callDetails)
	: QJniBaseException("JNI: Java method raised an unhandled exception.")
{
	qDebug() << "QJniHelpers: QJniJavaCallException with" << callDetails;
}



/////////////////////////////////////////////////////////////////////////////
// QJniEnvPtr
/////////////////////////////////////////////////////////////////////////////

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
	VERBOSE(qDebug("Preloading class \"%s\"", class_name));
	QJniLocalRef clazz(env_, env_->FindClass(class_name)); // jclass
	if (clearException() || clazz.jObject() == 0)
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

JavaVM * QJniEnvPtr::getJavaVM()
{
	return g_JavaVm;
}

bool QJniEnvPtr::isCurrentThreadAttached()
{
	if (g_JavaVm)
	{
		JNIEnv * env = 0;
		int errsv = g_JavaVm->GetEnv((void**)(&env), JNI_VERSION_1_6);
		if (errsv == JNI_OK && env)
		{
			return true;
		}
	}
	return false;
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

jstring QJniEnvPtr::JStringFromQString(const QString & str)
{
	jstring ret = env_->NewString(str.utf16(), str.length());
	if (clearException())
	{
		qWarning("Failed to convert QString to jstring.");
		return 0; // Not throwing an exception here
	}
	return ret;
}

QString QJniEnvPtr::QStringFromJString(jstring str)
{
	if (str == 0)
	{
		return QString();
	}

	int length = env_->GetStringLength(str);
	if (length == 0)
	{
		return QString();
	}

	const jchar* str_ptr = env_->GetStringChars(str, 0);
	if (str_ptr == 0)
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



/////////////////////////////////////////////////////////////////////////////
// QJniClass
/////////////////////////////////////////////////////////////////////////////

QJniClass::QJniClass(jclass clazz)
	: class_(0)
{
	QJniEnvPtr jep;
	initClass(jep.env(), clazz);
}

QJniClass::QJniClass(const char * full_class_name)
	: class_(0)
{
	QJniEnvPtr jep;
	jclass cls = jep.findClass(full_class_name); // this is a preloaded global ref, we don't need to delete it as a local ref
	if (jep.clearException())
	{
		throw QJniBaseException();
	}
	if (cls == 0)
	{
		qWarning("Could not find class %s", full_class_name);
		throw QJniClassNotFoundException();
	}
	VERBOSE(qDebug("Class \"%s\" is loaded @ %p", full_class_name, cls));

	initClass(jep.env(), cls);
}

QJniClass::QJniClass(jobject object)
	: class_(0)
{
	QJniEnvPtr jep;
	// Note: class is expected to be a valid ref during the whole lifetime of the object.
	if (object)
	{
		QJniLocalRef clazz(jep.env(), jep.env()->GetObjectClass(object));
		// Note: clazz may be null (for arrays).
		initClass(jep.env(), clazz);
	}
}

QJniClass::QJniClass(const QJniClass &other)
	: class_(0)
{
	QJniEnvPtr jep;
	initClass(jep.env(), other.class_);
}

QJniClass::~QJniClass()
{
	VERBOSE(qDebug("QJniClass::~QJniClass() %p",this));
	QJniEnvPtr jep;
	clearClass(jep.env());
}

void QJniClass::initClass(JNIEnv* env, jclass clazz)
{
	QJniEnvPtr jep(env);
	clearClass(jep.env());
	if (clazz)
	{
		class_ = static_cast<jclass>(env->NewGlobalRef(clazz));
		if (jep.clearException())
		{
			throw QJniBaseException();
		}
	}
}

void QJniClass::clearClass(JNIEnv* env)
{
	if (class_ != 0)
	{
		env->DeleteGlobalRef(class_);
		class_ = 0;
	}
}

void QJniClass::callStaticVoid(const char* method_name)
{
	VERBOSE(qDebug("void QJniClass::CallStaticVoid(const char* method_name) %p \"%s\"", this, method_name));
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();
	jmethodID mid = env->GetStaticMethodID(checkedClass(), method_name, "()V");
	if (!mid)
	{
		qWarning("%s: method %s not found.", __FUNCTION__, method_name);
		throw QJniMethodNotFoundException();
	}
	env->CallStaticVoidMethod(class_, mid);
	if (jep.clearException())
	{
		throw QJniJavaCallException(method_name);
	}
}

jint QJniClass::callStaticInt(const char* method_name)
{
	VERBOSE(qDebug("void QJniClass::callStaticInt(const char* method_name) %p \"%s\"", this, method_name));
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();
	jmethodID mid = env->GetStaticMethodID(checkedClass(), method_name, "()I");
	if (!mid)
	{
		qWarning("%s: method %s not found.", __FUNCTION__, method_name);
		throw QJniMethodNotFoundException();
	}
	jint result = env->CallStaticIntMethod(jClass(), mid);
	if (jep.clearException())
	{
		throw QJniJavaCallException(method_name);
	}
	return result;
}

jlong QJniClass::callStaticLong(const char* method_name)
{
	VERBOSE(qDebug("void QJniClass::callStaticLong(const char* method_name) %p \"%s\"", this, method_name));
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();
	jmethodID mid = env->GetStaticMethodID(checkedClass(), method_name, "()J");
	if (!mid)
	{
		qWarning("%s: method %s not found.", __FUNCTION__, method_name);
		throw QJniMethodNotFoundException();
	}
	jlong result = env->CallStaticLongMethod(jClass(), mid);
	if (jep.clearException())
	{
		throw QJniJavaCallException(method_name);
	}
	return result;
}

bool QJniClass::callStaticBoolean(const char* method_name)
{
	VERBOSE(qDebug("void QJniClass::callStaticBoolean(const char* method_name) %p \"%s\"", this, method_name));
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();
	jmethodID mid = env->GetStaticMethodID(checkedClass(), method_name, "()Z");
	if (!mid)
	{
		qWarning("%s: method %s not found.", __FUNCTION__, method_name);
		throw QJniMethodNotFoundException();
	}
	bool result = env->CallStaticBooleanMethod(jClass(), mid)? true: false;
	if (jep.clearException())
	{
		throw QJniJavaCallException(method_name);
	}
	return result;
}

void QJniClass::callStaticParamVoid(const char * method_name, const char * param_signature, ...)
{
	VERBOSE(qDebug("void QJniClass(%p)::CallStaticParamVoid(\"%s\", \"%s\", ...)", this, method_name, param_signature));
	va_list args;
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();

	QByteArray signature("(");
	signature += param_signature;
	signature += ")V";
	jmethodID mid = env->GetStaticMethodID(checkedClass(), method_name, signature.data());
	if (!mid)
	{
		qWarning("%s: method %s not found.", __FUNCTION__, method_name);
		throw QJniMethodNotFoundException();
	}

	va_start(args, param_signature);
	env->CallStaticVoidMethodV(jClass(), mid, args);
	va_end(args);
	if (jep.clearException())
	{
		qWarning("void QJniClass(%p)::CallStaticParamVoid(\"%s\", \"%s\", ...): exception occured", this, method_name, param_signature);
		throw QJniJavaCallException(method_name);
	}
}

bool QJniClass::callStaticParamBoolean(const char * method_name, const char * param_signature, ...)
{
	VERBOSE(qDebug("void QJniClass(%p)::CallStaticParamBoolean(\"%s\", \"%s\", ...)", this, method_name, param_signature));
	va_list args;
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();

	QByteArray signature("(");
	signature += param_signature;
	signature += ")Z";
	jmethodID mid = env->GetStaticMethodID(checkedClass(), method_name, signature.data());
	if (!mid)
	{
		qWarning("%s: method %s not found.", __FUNCTION__, method_name);
		throw QJniMethodNotFoundException();
	}

	va_start(args, param_signature);
	bool result = env->CallStaticBooleanMethodV(jClass(), mid, args);
	va_end(args);
	if (jep.clearException())
	{
		qWarning("void QJniClass(%p)::CallStatucParamBoolean(\"%s\", \"%s\", ...): exception occured", this, method_name, param_signature);
		throw QJniJavaCallException(method_name);
	}
	return result;
}

jint QJniClass::callStaticParamInt(const char * method_name, const char * param_signature, ...)
{
	VERBOSE(qDebug("void QJniClass(%p)::CallStaticParamInt(\"%s\", \"%s\", ...)", this, method_name, param_signature));
	va_list args;
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();

	QByteArray signature("(");
	signature += param_signature;
	signature += ")I";
	jmethodID mid = env->GetStaticMethodID(checkedClass(), method_name, signature.data());
	if (!mid)
	{
		qWarning("%s: method %s not found.", __FUNCTION__, method_name);
		throw QJniMethodNotFoundException();
	}

	va_start(args, param_signature);
	jint result = env->CallStaticIntMethodV(jClass(), mid, args);
	va_end(args);
	if (jep.clearException())
	{
		qWarning("void QJniClass(%p)::CallStatucParamInt(\"%s\", \"%s\", ...): exception occured", this, method_name, param_signature);
		throw QJniJavaCallException(method_name);
	}
	return result;
}

jfloat QJniClass::callStaticParamFloat(const char * method_name, const char * param_signature, ...)
{
	VERBOSE(qDebug("void QJniClass(%p)::CallStaticParamFloat(\"%s\", \"%s\", ...)", this, method_name, param_signature));
	va_list args;
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();

	QByteArray signature("(");
	signature += param_signature;
	signature += ")F";
	jmethodID mid = env->GetStaticMethodID(checkedClass(), method_name, signature.data());
	if (!mid)
	{
		qWarning("%s: method %s not found.", __FUNCTION__, method_name);
		throw QJniMethodNotFoundException();
	}

	va_start(args, param_signature);
	jfloat result = env->CallStaticFloatMethodV(jClass(), mid, args);
	va_end(args);
	if (jep.clearException())
	{
		qWarning("void QJniClass(%p)::CallStatucParamFloat(\"%s\", \"%s\", ...): exception occured", this, method_name, param_signature);
		throw QJniJavaCallException(method_name);
	}
	return result;
}


QString QJniClass::callStaticParamString(const char * method_name, const char * param_signature, ...)
{
	VERBOSE(qDebug("void QJniClass(%p)::CallStaticParamString(\"%s\", \"%s\", ...)", this, method_name, param_signature));
	va_list args;
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();

	QByteArray signature("(");
	signature += param_signature;
	signature += ")Ljava/lang/String;";
	jmethodID mid = env->GetStaticMethodID(checkedClass(), method_name, signature.data());
	if (!mid)
	{
		qWarning("%s: method %s not found.", __FUNCTION__, method_name);
		throw QJniMethodNotFoundException();
	}

	va_start(args, param_signature);
	QString ret = QJniLocalRef(env, env->CallStaticObjectMethodV(jClass(), mid, args));
	va_end(args);
	if (jep.clearException())
	{
		qWarning("void QJniClass(%p)::CallStatucParamString(\"%s\", \"%s\", ...): exception occured", this, method_name, param_signature);
		throw QJniJavaCallException(method_name);
	}
	return ret;
}

void QJniClass::callStaticVoid(const char* method_name, const QString & string)
{
	callStaticParamVoid(method_name, "Ljava/lang/String;", QJniLocalRef(string).jObject());
}

QString QJniClass::callStaticString(const char *method_name)
{
	VERBOSE(qDebug("QString QJniClass::CallStaticString(const char* method_name) %p \"%s\"",this,method_name));
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();
	jmethodID mid = env->GetStaticMethodID(checkedClass(), method_name, "()Ljava/lang/String;");
	if (!mid)
	{
		qWarning("%s: method %s not found.", __FUNCTION__, method_name);
		throw QJniMethodNotFoundException();
	}
	QString ret = QJniLocalRef(env, env->CallStaticObjectMethod(jClass(), mid));
	if (jep.clearException())
	{
		throw QJniJavaCallException(method_name);
	}
	return ret;
}

QJniObject * QJniClass::getStaticObjectField(const char * field_name, const char * objname)
{
	VERBOSE(qDebug("int QJniObject::getStaticObjectField(const char * field_name, const char * objname) %p \"%s\"", this, field_name, objname));
	QByteArray obj;
	appendNormalizedObjectName(obj, objname);
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();
	jfieldID fid = env->GetStaticFieldID(checkedClass(), field_name, obj.data());
	if (!fid)
	{
		qWarning("%s: field %s not found.", __FUNCTION__, field_name);
		throw QJniFieldNotFoundException();
	}
	jobject jo = env->GetStaticObjectField(jClass(), fid);
	if (jep.clearException())
	{
		if (jo)
		{
			env->DeleteLocalRef(jo);
		}
		throw QJniJavaCallException(field_name, objname);
	}
	return new QJniObject(jo, true, objname);
}

QString QJniClass::getStaticStringField(const char * field_name)
{
	VERBOSE(qDebug("int QJniObject::getStaticStringField(const char * field_name) %p \"%s\"", this, field_name));
	QJniEnvPtr jep;
	JNIEnv * env = jep.env();
	jfieldID fid = env->GetStaticFieldID(checkedClass(), field_name, "Ljava/lang/String;");
	if (!fid)
	{
		qWarning("%s: field %s not found.", __FUNCTION__, field_name);
		throw QJniFieldNotFoundException();
	}
	QString ret = QJniLocalRef(env, env->GetStaticObjectField(class_, fid));
	if (jep.clearException())
	{
		throw QJniJavaCallException(field_name);
	}
	return ret;
}

jint QJniClass::getStaticIntField(const char * field_name)
{
	VERBOSE(qDebug("int QJniObject::getStaticIntField(const char * field_name) %p \"%s\"", this, field_name));
	QJniEnvPtr jep;
	JNIEnv * env = jep.env();
	jfieldID fid = env->GetStaticFieldID(checkedClass(), field_name, "I");
	if (!fid)
	{
		qWarning("%s: field %s not found.", __FUNCTION__, field_name);
		throw QJniFieldNotFoundException();
	}
	jint result = env->GetStaticIntField(jClass(), fid);
	if (jep.clearException())
	{
		throw QJniJavaCallException(field_name);
	}
	return result;
}

bool QJniClass::getStaticBooleanField(const char * field_name)
{
	VERBOSE(qDebug("int QJniObject::getStaticBooleanField(const char * field_name) %p \"%s\"", this, field_name));
	QJniEnvPtr jep;
	JNIEnv * env = jep.env();
	jfieldID fid = env->GetStaticFieldID(checkedClass(), field_name, "Z");
	if (!fid)
	{
		qWarning("%s: field %s not found.", __FUNCTION__, field_name);
		throw QJniFieldNotFoundException();
	}
	jint result = env->GetStaticBooleanField(jClass(), fid);
	if (jep.clearException())
	{
		throw QJniJavaCallException(field_name);
	}
	return (result)? true: false;
}

QJniObject* QJniClass::callStaticObject(const char * method_name, const char * objname)
{
	VERBOSE(qDebug("QJniClass::CallStaticObject(\"%s\",\"%s\")", method_name, objname));
	QByteArray signature("()");
	appendNormalizedObjectName(signature, objname);

	VERBOSE(qDebug("QJniClass::CallStaticObject signature: %s", signature.data()));
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();

	VERBOSE(qDebug("env->GetStaticMethodID"));
	jmethodID mid = env->GetStaticMethodID(checkedClass(), method_name, signature.data());
	if (!mid)
	{
		qWarning("%s: method %s not found.", __FUNCTION__, method_name);
		throw QJniMethodNotFoundException();
	}

	VERBOSE(qDebug("new QJniClass(env->CallStaticObjectMethod(jClass(),mid), true);"));
	jobject jret = env->CallStaticObjectMethod(jClass(), mid);
	if (jep.clearException())
	{
		if (jret)
		{
			env->DeleteLocalRef(jret);
		}
		throw QJniJavaCallException(method_name, objname);
	}
	if (jret == 0)
	{
		return 0; // Not an exception
	}
	return new QJniObject(jret, true, objname);
}

QJniObject * QJniClass::callStaticParamObject(const char * method_name, const char * objname, const char * param_signature, ...)
{
	VERBOSE(qDebug("void QJniClass(%p)::callStaticParamObject(\"%s\", \"%s\", \"%s\"...)", this, method_name, objname, param_signature));
	va_list args;
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();

	QByteArray signature;
	makeObjectFunctionSignature(signature, param_signature, objname);

	jmethodID mid = env->GetStaticMethodID(checkedClass(), method_name, signature.data());
	if (!mid)
	{
		qWarning("%s: method %s not found.", __FUNCTION__, method_name);
		throw QJniMethodNotFoundException();
	}

	va_start(args, param_signature);
	jobject jret = env->CallStaticObjectMethodV(jClass(), mid, args);
	va_end(args);
	if (jep.clearException())
	{
		qWarning("void QJniClass(%p)::callStaticParamObject(\"%s\", \"%s\", ...): exception occured", this, method_name, param_signature);
		if (jret)
		{
			env->DeleteLocalRef(jret);
		}
		throw QJniJavaCallException(method_name, objname);
	}
	return new QJniObject(jret, true, objname);
}

QJniClass & QJniClass::operator=(const QJniClass &other)
{
	if (this != &other)
	{
		initClass(QJniEnvPtr().env(), other.jClass());
	}
	return *this;
}

bool QJniClass::registerNativeMethod(const char* name, const char* signature, void* ptr)
{
	JNINativeMethod jnm = {name, signature, ptr};
	return registerNativeMethods(&jnm, sizeof(jnm));
}

bool QJniClass::registerNativeMethods(const JNINativeMethod * methods_list, size_t sizeof_methods_list)
{
	QJniEnvPtr jep;
	jint result = jep.env()->RegisterNatives(checkedClass(), methods_list, sizeof_methods_list/sizeof(JNINativeMethod));
	if (jep.clearException())
	{
		throw QJniJavaCallException("registerNativeMethods with list");
	}
	return result == 0;
}


/////////////////////////////////////////////////////////////////////////////
// QJniObject
/////////////////////////////////////////////////////////////////////////////

QJniObject::QJniObject(jobject instance, bool take_ownership, const char * known_class_name)
	: QJniClass(instance)
	, instance_(0)
{
	QJniEnvPtr jep;
	// Creates global reference
	initObject(jep.env(), instance, classObjectMayHaveNullClass(known_class_name));
	if (take_ownership)
	{
		jep.env()->DeleteLocalRef(instance);
	}
}

QJniObject::QJniObject(const QJniClass &clazz, const char* param_signature, ...)
	: QJniClass(clazz)
	, instance_(0)
{
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();

	QByteArray signature("(");
	if (param_signature)
	{
		signature += param_signature;
	}
	signature += ")V";

	jmethodID mid_init = env->GetMethodID(checkedClass(), "<init>", signature.data());
	if (jep.clearException())
	{
		throw QJniBaseException();
	}
	if (!mid_init)
	{
		qWarning("%s: method <init> not found.", __FUNCTION__);
		throw QJniMethodNotFoundException();
	}

	va_list args;
	va_start(args, param_signature);
	QJniLocalRef obj(env, env->NewObjectV(jClass(), mid_init, args));
	va_end(args);
	if (jep.clearException())
	{
		throw QJniBaseException();
	}

	// it is dangerous to go alone, use this
	initObject(env, obj);
}

QJniObject::QJniObject(const char* class_name, const char* param_signature, ...)
	: QJniClass(class_name)
	, instance_(0)
{
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();

	QByteArray signature("(");
	if (param_signature)
	{
		signature += param_signature;
	}
	signature += ")V";

	jmethodID mid_init = env->GetMethodID(checkedClass(), "<init>", signature.data());
	if (jep.clearException())
	{
		throw QJniBaseException();
	}
	if (!mid_init)
	{
		qWarning("%s: method <init> not found.", __FUNCTION__);
		throw QJniMethodNotFoundException();
	}

	va_list args;
	va_start(args, param_signature);
	QJniLocalRef obj(env, env->NewObjectV(jClass(), mid_init, args));
	va_end(args);
	if (jep.clearException())
	{
		throw QJniBaseException();
	}

	// it is dangerous to go alone, use this
	initObject(env, obj);
}

QJniObject::~QJniObject()
{
	VERBOSE(qDebug("QJniObject::~QJniObject() %p",this));
	QJniEnvPtr jep;
	if (instance_ != 0)
	{
		jep.env()->DeleteGlobalRef(instance_);
	}
}

jobject QJniObject::takeJobjectOver()
{
	jobject ret = instance_;
	instance_ = 0;
	return ret;
}

void QJniObject::initObject(JNIEnv* env, jobject instance, bool can_have_null_class)
{
	VERBOSE(qDebug("QJniObject::init(JNIEnv* env, jobject instance) %p", this));
	if (!can_have_null_class)
	{
		checkedClass();
	}
	QJniEnvPtr jep(env);
	instance_ = env->NewGlobalRef(instance);
	if (jep.clearException())
	{
		throw QJniBaseException();
	}
}

void QJniObject::callVoid(const char* method_name)
{
	VERBOSE(qDebug("void QJniObject::CallVoid(const char* method_name) %p \"%s\"",this,method_name));
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();
	jmethodID mid = env->GetMethodID(checkedClass(), method_name, "()V");
	if (!mid)
	{
		qWarning("%s: method %s not found.", __FUNCTION__, method_name);
		throw QJniMethodNotFoundException();
	}
	env->CallVoidMethod(instance_, mid);
	if (jep.clearException())
	{
		throw QJniJavaCallException(method_name);
	}
}

bool QJniObject::callBool(const char* method_name)
{
	VERBOSE(qDebug("bool QJniObject::CallBool(const char* method_name) %p \"%s\"",this,method_name));
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();
	jmethodID mid = env->GetMethodID(checkedClass(), method_name, "()Z");
	if (!mid)
	{
		qWarning("%s: method %s not found.", __FUNCTION__, method_name);
		throw QJniMethodNotFoundException();
	}
	bool result = (JNI_TRUE==env->CallBooleanMethod(instance_, mid));
	if (jep.clearException())
	{
		throw QJniJavaCallException(method_name);
	}
	return result;
}

bool QJniObject::callBool(const char* method_name, bool param)
{
	VERBOSE(qDebug("bool QJniObject::CallBool(const char* method_name, bool param) %p \"%s\"",this,method_name));
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();
	jmethodID mid = env->GetMethodID(checkedClass(), method_name, "(Z)Z");
	if (!mid)
	{
		qWarning("%s: method %s not found.", __FUNCTION__, method_name);
		throw QJniMethodNotFoundException();
	}
	bool result = (JNI_TRUE==env->CallBooleanMethod(instance_, mid, jboolean(param)));
	if (jep.clearException())
	{
		throw QJniJavaCallException(method_name);
	}
	return result;
}

int QJniObject::callInt(const char* method_name)
{
	VERBOSE(qDebug("int QJniObject::CallInt(const char* method_name) %p \"%s\"",this,method_name));
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();
	jmethodID mid = env->GetMethodID(checkedClass(), method_name, "()I");
	if (!mid)
	{
		qWarning("%s: method %s not found.", __FUNCTION__, method_name);
		throw QJniMethodNotFoundException();
	}
	int result = (int)env->CallIntMethod(instance_, mid);
	if (jep.clearException())
	{
		throw QJniJavaCallException(method_name);
	}
	return result;
}

long long QJniObject::callLong(const char* method_name)
{
	VERBOSE(qDebug("int QJniObject::CallLong(const char* method_name) %p \"%s\"",this,method_name));
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();
	jmethodID mid = env->GetMethodID(checkedClass(), method_name, "()J");
	if (!mid)
	{
		qWarning("%s: method %s not found.", __FUNCTION__, method_name);
		throw QJniMethodNotFoundException();
	}
	long long result = (long long)env->CallLongMethod(instance_, mid);
	if (jep.clearException())
	{
		throw QJniJavaCallException(method_name);
	}
	return result;
}

float QJniObject::callFloat(const char* method_name)
{
	VERBOSE(qDebug("float QJniObject::CallFloat(const char* method_name) %p \"%s\"",this,method_name));
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();
	jmethodID mid = env->GetMethodID(checkedClass(), method_name, "()F");
	if (!mid)
	{
		qWarning("%s: method %s not found.", __FUNCTION__, method_name);
		throw QJniMethodNotFoundException();
	}
	float result = (float)env->CallFloatMethod(instance_, mid);
	if (jep.clearException())
	{
		throw QJniJavaCallException(method_name);
	}
	return result;
}

float QJniObject::callFloat(const char* method_name, int param)
{
	VERBOSE(qDebug("float QJniObject::CallFloat(const char* method_name) %p \"%s\" (%d)",this,method_name, param));
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();
	jmethodID mid = env->GetMethodID(checkedClass(), method_name, "(I)F");
	if (!mid)
	{
		qWarning("%s: method %s not found.", __FUNCTION__, method_name);
		throw QJniMethodNotFoundException();
	}
	float result = (float)env->CallFloatMethod(instance_, mid, jint(param));
	if (jep.clearException())
	{
		throw QJniJavaCallException(method_name);
	}
	return result;
}

double QJniObject::callDouble(const char* method_name)
{
	VERBOSE(qDebug("float QJniObject::CallDouble(const char* method_name) %p \"%s\"",this,method_name));
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();
	jmethodID mid = env->GetMethodID(checkedClass(), method_name, "()D");
	if (!mid)
	{
		qWarning("%s: method %s not found.", __FUNCTION__, method_name);
		throw QJniMethodNotFoundException();
	}
	double result = (double)env->CallDoubleMethod(instance_, mid);
	if (jep.clearException())
	{
		throw QJniJavaCallException(method_name);
	}
	return result;
}

QJniObject * QJniObject::callObject(const char * method_name, const char * objname)
{
	QByteArray signature("()");
	appendNormalizedObjectName(signature, objname);

	VERBOSE(qDebug("QJniObject::callObject: \"%s\", \"%s\"", method_name, signature.data()));
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();
	jmethodID mid = env->GetMethodID(checkedClass(), method_name, signature.data());
	if (!mid)
	{
		qWarning("%s: method %s not found.", __FUNCTION__, method_name);
		throw QJniMethodNotFoundException();
	}
	jobject object = env->CallObjectMethod(instance_, mid);
	if (jep.clearException())
	{
		if (object)
		{
			env->DeleteLocalRef(object);
		}
		throw QJniJavaCallException(method_name, objname);
	}
	QJniObject * result = new QJniObject(object, true, objname);
	return result;
}

QJniObject * QJniObject::callParamObject(const char * method_name, const char * objname, const char * param_signature, ...)
{
	VERBOSE(qDebug("void QJniObject(%p)::callParamObject(\"%s\", \"%s\", \"%s\"...)", this, method_name, objname, param_signature));

	va_list args;
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();

	QByteArray signature;
	makeObjectFunctionSignature(signature, param_signature, objname);

	jmethodID mid = env->GetMethodID(checkedClass(), method_name, signature.data());
	if (!mid)
	{
		qWarning()<<"callParamObject: method"<<method_name<<"not found, sig ="<<signature;
		throw QJniMethodNotFoundException();
	}

	va_start(args, param_signature);
	jobject jret = env->CallObjectMethodV(instance_, mid, args);
	va_end(args);
	if (jep.clearException())
	{
		qWarning("void QJniObject(%p)::callParamObject(\"%s\", \"%s\", ...): exception occured", this, method_name, param_signature);
		if (jret)
		{
			env->DeleteLocalRef(jret);
		}
		throw QJniJavaCallException(method_name, objname);
	}
	return new QJniObject(jret, true, objname);
}

jint QJniObject::callParamInt(const char* method_name, const char* param_signature, ...)
{
	VERBOSE(qDebug("void QJniObject(%p)::callParamInt(\"%s\", \"%s\", ...)", this, method_name, param_signature));
	va_list args;
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();
	QByteArray signature("(");
	signature += param_signature;
	signature += ")I";
	jmethodID mid = env->GetMethodID(checkedClass(), method_name, signature.data());
	if (!mid)
	{
		qWarning("%s: method %s not found.", method_name, method_name);
		throw QJniMethodNotFoundException();
	}
	va_start(args, param_signature);
	jint result = env->CallIntMethodV(instance_, mid, args);
	va_end(args);
	if (jep.clearException())
	{
		qWarning("void QJniObject(%p)::callParamInt(\"%s\", \"%s\", ...): exception occured", this, method_name, param_signature);
		throw QJniJavaCallException(method_name);
	}
	return result;
}

jlong QJniObject::callParamLong(const char* method_name, const char* param_signature, ...)
{
	VERBOSE(qDebug("void QJniObject(%p)::callParamLong(\"%s\", \"%s\", ...)", this, method_name, param_signature));
	va_list args;
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();
	QByteArray signature("(");
	signature += param_signature;
	signature += ")J";
	jmethodID mid = env->GetMethodID(checkedClass(), method_name, signature.data());
	if (!mid)
	{
		qWarning("%s: method %s not found.", method_name, method_name);
		throw QJniMethodNotFoundException();
	}
	va_start(args, param_signature);
	jlong result = env->CallLongMethodV(instance_, mid, args);
	va_end(args);
	if (jep.clearException())
	{
		qWarning("void QJniObject(%p)::callParamLong(\"%s\", \"%s\", ...): exception occured", this, method_name, param_signature);
		throw QJniJavaCallException(method_name);
	}
	return result;
}

jfloat QJniObject::callParamFloat(const char* method_name, const char* param_signature, ...)
{
	VERBOSE(qDebug("void QJniObject(%p)::callParamFloat(\"%s\", \"%s\", ...)", this, method_name, param_signature));
	va_list args;
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();
	QByteArray signature("(");
	signature += param_signature;
	signature += ")F";
	jmethodID mid = env->GetMethodID(checkedClass(), method_name, signature.data());
	if (!mid)
	{
		qWarning("%s: method %s not found.", method_name, method_name);
		throw QJniMethodNotFoundException();
	}
	va_start(args, param_signature);
	jfloat result = env->CallFloatMethodV(instance_, mid, args);
	va_end(args);
	if (jep.clearException())
	{
		qWarning("void QJniObject(%p)::callParamFloat(\"%s\", \"%s\", ...): exception occured", this, method_name, param_signature);
		throw QJniJavaCallException(method_name);
	}
	return result;
}

jdouble QJniObject::callParamDouble(const char* method_name, const char* param_signature, ...)
{
	VERBOSE(qDebug("void QJniObject(%p)::callParamDouble(\"%s\", \"%s\", ...)", this, method_name, param_signature));
	va_list args;
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();
	QByteArray signature("(");
	signature += param_signature;
	signature += ")D";
	jmethodID mid = env->GetMethodID(checkedClass(), method_name, signature.data());
	if (!mid)
	{
		qWarning("%s: method %s not found.", method_name, method_name);
		throw QJniMethodNotFoundException();
	}
	va_start(args, param_signature);
	jdouble result = env->CallFloatMethodV(instance_, mid, args);
	va_end(args);
	if (jep.clearException())
	{
		qWarning("void QJniObject(%p)::callParamDouble(\"%s\", \"%s\", ...): exception occured", this, method_name, param_signature);
		throw QJniJavaCallException(method_name);
	}
	return result;
}

jboolean QJniObject::callParamBoolean(const char* method_name, const char* param_signature, ...)
{
	VERBOSE(qDebug("void QJniObject(%p)::callParamBoolean(\"%s\", \"%s\", ...)", this, method_name, param_signature));
	va_list args;
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();
	QByteArray signature("(");
	signature += param_signature;
	signature += ")Z";
	jmethodID mid = env->GetMethodID(checkedClass(), method_name, signature.data());
	if (!mid)
	{
		qWarning("%s: method %s not found.", method_name, method_name);
		throw QJniMethodNotFoundException();
	}
	va_start(args, param_signature);
	jboolean result = env->CallBooleanMethodV(instance_, mid, args);
	va_end(args);
	if (jep.clearException())
	{
		qWarning("void QJniObject(%p)::callParamBoolean(\"%s\", \"%s\", ...): exception occured", this, method_name, param_signature);
		throw QJniJavaCallException(method_name);
	}
	return result;
}


QString QJniObject::callString(const char *method_name)
{
	VERBOSE(qDebug("QString QJniObject::CallString(const char* method_name) %p \"%s\"",this,method_name));
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();
	jmethodID mid = env->GetMethodID(checkedClass(), method_name, "()Ljava/lang/String;");
	if (!mid)
	{
		qWarning("%s: method %s not found.", __FUNCTION__, method_name);
		throw QJniMethodNotFoundException();
	}
	QString ret = QJniLocalRef(env, env->CallObjectMethod(instance_, mid));
	if (jep.clearException())
	{
		throw QJniJavaCallException(method_name);
	}
	return ret;
}


QString QJniObject::callParamString(const char *method_name, const char* param_signature, ...)
{
	VERBOSE(qDebug("void QJniObject(%p)::callParamString(\"%s\", \"%s\", ...)", this, method_name, param_signature));

	va_list args;
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();

	QByteArray signature("(");
	signature += param_signature;
	signature += ")Ljava/lang/String;";
	jmethodID mid = env->GetMethodID(checkedClass(), method_name, signature.data());

	if (!mid)
	{
		qWarning("%s: method %s not found.", __FUNCTION__, method_name);
		throw QJniMethodNotFoundException();
	}

	va_start(args, param_signature);
	QString ret = QJniLocalRef(env, env->CallObjectMethodV(instance_, mid, args));
	va_end(args);

	if (jep.clearException())
	{
		throw QJniJavaCallException(method_name);
	}

	return ret;
}


QString QJniObject::getString(const char *field_name)
{
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();
	jfieldID fid = env->GetFieldID(checkedClass(), field_name, "Ljava/lang/String;");
	if (!fid)
	{
		qWarning("%s: field %s not found.", __FUNCTION__, field_name);
		throw QJniFieldNotFoundException();
	}
	QString ret = QJniLocalRef(env, env->GetObjectField(instance_, fid));
	if (jep.clearException())
	{
		throw QJniJavaCallException(field_name);
	}
	return ret;
}

int QJniObject::getIntField(const char * field_name)
{
	VERBOSE(qDebug("int QJniObject::getIntField(const char* fieldd_name) %p \"%s\"",this,field_name));
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();
	jfieldID fid = env->GetFieldID(checkedClass(), field_name, "I");
	if (!fid)
	{
		qWarning("%s: field %s not found.", __FUNCTION__, field_name);
		throw QJniFieldNotFoundException();
	}
	int result = (int)env->GetIntField(instance_, fid);
	if (jep.clearException())
	{
		throw QJniJavaCallException(field_name);
	}
	return result;
}

float QJniObject::getFloatField(const char * field_name)
{
	VERBOSE(qDebug("int QJniObject::getFloatField(const char* field_name) %p \"%s\"",this,field_name));
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();
	jfieldID fid = env->GetFieldID(checkedClass(), field_name, "F");
	if (!fid)
	{
		qWarning("%s: field %s not found.", __FUNCTION__, field_name);
		throw QJniFieldNotFoundException();
	}
	float result = (float)env->GetFloatField(instance_, fid);
	if (jep.clearException())
	{
		throw QJniJavaCallException(field_name);
	}
	return result;
}

void QJniObject::setIntField(const char * field_name, jint value)
{
	VERBOSE(qDebug("int QJniObject::setIntField(const char* field_name) %p \"%s\"",this,field_name));
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();
	jfieldID fid = env->GetFieldID(checkedClass(), field_name, "I");
	if (!fid)
	{
		qWarning("%s: field %s not found.", __FUNCTION__, field_name);
		throw QJniFieldNotFoundException();
	}
	env->SetIntField(instance_, fid, value);
	if (jep.clearException())
	{
		throw QJniJavaCallException(field_name);
	}
}

void QJniObject::setBooleanField(const char * field_name, jboolean value)
{
	VERBOSE(qDebug("int QJniObject::setBooleanField(const char* field_name) %p \"%s\"",this,field_name));
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();
	jfieldID fid = env->GetFieldID(checkedClass(), field_name, "Z");
	if (!fid)
	{
		qWarning("%s: field %s not found.", __FUNCTION__, field_name);
		throw QJniFieldNotFoundException();
	}
	env->SetBooleanField(instance_, fid, value);
	if (jep.clearException())
	{
		throw QJniJavaCallException(field_name);
	}
}

QJniObject * QJniObject::getObjectField(const char* field_name, const char * objname)
{
	VERBOSE(qDebug("int QJniObject::getObjectField(const char * field_name, const char * objname) %p \"%s\" \"%s\"", this, field_name, objname));
	QByteArray obj;
	appendNormalizedObjectName(obj, objname);

	QJniEnvPtr jep;
	JNIEnv* env = jep.env();
	jfieldID fid = env->GetFieldID(checkedClass(), field_name, obj.data());
	if (!fid)
	{
		qWarning("%s: field %s not found.", __FUNCTION__, field_name);
		throw QJniFieldNotFoundException();
	}
	jobject jo = env->GetObjectField(instance_, fid);
	if (jep.clearException())
	{
		if (jo)
		{
			env->DeleteLocalRef(jo);
		}
		throw QJniJavaCallException(field_name);
	}
	return new QJniObject(jo, true, objname);
}

QString QJniObject::getStringField(const char * field_name)
{
	VERBOSE(qDebug("int QJniObject::getStringField(const char * field_name) %p \"%s\"", this, field_name));
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();
	jfieldID fid = env->GetFieldID(checkedClass(), field_name, "Ljava/lang/String;");
	if (!fid)
	{
		qWarning("%s: field %s not found.", __FUNCTION__, field_name);
		throw QJniFieldNotFoundException();
	}
	QString ret = QJniLocalRef(env, env->GetObjectField(instance_, fid));
	if (jep.clearException())
	{
		throw QJniJavaCallException(field_name);
	}
	return ret;
}

void QJniObject::callParamVoid(const char * method_name, const char * param_signature, ...)
{
	VERBOSE(qDebug("void QJniObject(%p)::callParamVoid(\"%s\", \"%s\", ...)", this, method_name, param_signature));

	va_list args;
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();

	QByteArray signature("(");
	signature += param_signature;
	signature += ")V";
	jmethodID mid = env->GetMethodID(checkedClass(), method_name, signature.data());
	if (!mid)
	{
		qWarning("%s: method %s not found.", __FUNCTION__, method_name);
		throw QJniMethodNotFoundException();
	}

	va_start(args, param_signature);
	env->CallVoidMethodV(instance_, mid, args);
	va_end(args);

	if (jep.clearException())
	{
		qWarning("void QJniObject(%p)::callParamVoid(\"%s\", \"%s\", ...): exception occured", this, method_name, param_signature);
		throw QJniJavaCallException(method_name);
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

void QJniObject::callVoid(const char * method_name, jlong x1, jlong x2)
{
	callParamVoid(method_name, "JJ", x1, x2);
}

void QJniObject::callVoid(const char * method_name, jboolean x)
{
	callParamVoid(method_name, "Z", x);
}

void QJniObject::callVoid(const char * method_name, jfloat x)
{
	callParamVoid(method_name, "F", x);
}

void QJniObject::callVoid(const char * method_name, jdouble x)
{
	callParamVoid(method_name, "D", x);
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

void QJniObject::callVoid(const char * method_name, const QString & string1, const QString & string2, const QString & string3, const QString & string4, const QString & string5, const QString & string6)
{
	QJniEnvPtr jep;
	callParamVoid(method_name, "Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;",
		QJniLocalRef(jep, string1).jObject(), QJniLocalRef(jep, string2).jObject(), QJniLocalRef(jep, string3).jObject(),
		QJniLocalRef(jep, string4).jObject(), QJniLocalRef(jep, string5).jObject(), QJniLocalRef(jep, string6).jObject());
}


/////////////////////////////////////////////////////////////////////////////
// QJniLocalRef
/////////////////////////////////////////////////////////////////////////////

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
