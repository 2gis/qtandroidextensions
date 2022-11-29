/*
  QJniHelpers library

  Authors:
  Ivan Avdeev <marflon@gmail.com>
  Sergey A. Galin <sergey.galin@gmail.com>

  Distrbuted under The BSD License

  Copyright (c) 2017, DoubleGIS, LLC.
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

static JavaVM * g_JavaVm = 0;

//! Data type to keep list of JNI references to Java classes ever preloaded or loaded.
typedef QHash<QString, jclass> PreloadedClasses;

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
		if (g_JavaVm != 0)
		{
			QJniEnvPtr().unloadAllClasses();
		}
	}
};


static QMutex g_PreloadedClassesMutex(QMutex::Recursive);
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
			qWarning("Thread %d detach failed: %d", static_cast<int>(gettid()), errsv);
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
// Examples:
// in: "java/lang/String" => "Ljava/lang/String;"
// in: "Ljava/lang/String;" => "Ljava/lang/String;"
// in: "[F" => "[F"
static inline void appendNormalizedObjectName(
	QByteArray & out_signature
	, const char * objname)
{
	if (size_t length = strlen(objname))
	{
		if (objname[0] != '[' && objname[length-1] != ';')
		{
			out_signature.append('L');
			out_signature.append(objname, static_cast<int>(length));
			out_signature.append(';');
		}
		else
		{
			out_signature.append(objname, static_cast<int>(length));
		}
	}
}


// Append function signature to out_signature:
// (param_signature)normalized(returning_objname)
// Examples:
// params = "II", returning: "java/lang/String" => "(II)Ljava/lang/String;"
// params = "II", returning: "Ljava/lang/String;" => "(II)Ljava/lang/String;"
// params = "II", returning: "[F" => "(II)[F"
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


// Automatically ignore null class reference for arrays
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

QJniBaseException::QJniBaseException(const QByteArray & message)
	: message_(message.isEmpty() ? "JNI: Java exception." : message)
{
	qWarning() << "QJniHelpers: throwing an exception:" << what();
}


QByteArray QJniBaseException::readableIdString(const char * id)
{
	return QByteArray((id) ?
		((id[0]) ? id : "<empty>")
		: "<null>");
}


const char * QJniBaseException::what() const throw()
{
	return message_.constData();
}


QJniThreadAttachException::QJniThreadAttachException(const char * detail)
	: QJniBaseException(QByteArray("JNI: Thread attaching exception: ")
		.append((detail) ? detail : "<no detail>"))
{
}


QJniClassNotFoundException::QJniClassNotFoundException(const char * class_name)
	: QJniBaseException(QByteArray("JNI: Java class not found: ")
		.append(readableIdString(class_name)))
{
}


QJniClassNotSetException::QJniClassNotSetException(const char * class_name, const char * call_point_info)
	: QJniBaseException(QByteArray("JNI: Java class is null: ")
		.append(readableIdString(class_name))
		.append(", more info: ")
		.append(readableIdString(call_point_info)))
{
}


QJniMethodNotFoundException::QJniMethodNotFoundException(
		const char * class_name
		, const char * method_name
		, const char * call_point_info)
	: QJniBaseException(QByteArray("JNI: Java method not found: ")
		.append(readableIdString(class_name))
		.append(".")
		.append(readableIdString(method_name))
		.append(", more info: ")
		.append(readableIdString(call_point_info)))
{
}


QJniFieldNotFoundException::QJniFieldNotFoundException(
		const char * class_name
		, const char * field_name
		, const char * call_point_info)
	: QJniBaseException(QByteArray("JNI: Java field not found: ")
		.append(readableIdString(class_name))
		.append(".")
		.append(readableIdString(field_name))
		.append(", more info: ")
		.append(readableIdString(call_point_info)))
{
}


QJniJavaCallException::QJniJavaCallException(
		const char * class_name
		, const char * method_name
		, const char * call_point_info)
	: QJniBaseException(QByteArray("JNI: Java method raised an unhandled exception: ")
		.append(readableIdString(class_name))
		.append(".")
		.append(readableIdString(method_name))
		.append(", more info: ")
		.append(readableIdString(call_point_info)))
{
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
		throw QJniThreadAttachException("Java VM pointer is not set.");
	}
	if (!env_)
	{
		int errsv = g_JavaVm->GetEnv(reinterpret_cast<void**>(&env_), JNI_VERSION_1_6);
		if (errsv == JNI_EDETACHED)
		{
			VERBOSE(qWarning("Current thread %d is not attached, attaching it...", (int)gettid()));
			errsv = g_JavaVm->AttachCurrentThread(&env_, 0);
			if (errsv != 0)
			{
				throw QJniThreadAttachException(
					QString(QLatin1String("Error attaching current thread %1, errsv=%2"))
						.arg(gettid())
						.arg(errsv)
						.toLatin1());
			}
			if (!env_)
			{
				throw QJniThreadAttachException(
					QString(QLatin1String("Error attaching current thread %1 - returned env ptr is null."))
						.arg(gettid())
						.toLatin1());
			}
			if (!g_JavaThreadDetacher.hasLocalData())
			{
				g_JavaThreadDetacher.setLocalData(new QJniEnvPtrThreadDetacher());
			}
			VERBOSE(qWarning("Attached current thread %d successfully.", (int)gettid()));
		}
		else if (errsv != JNI_OK)
		{
			throw QJniThreadAttachException(
				QString(QLatin1String("Error getting Java environment, thread %1, errsv=%2"))
					.arg(gettid())
					.arg(errsv)
					.toLatin1());
		}
	}
}


QJniEnvPtr::~QJniEnvPtr()
{
}


void QJniEnvPtr::checkEnv()
{
	if (!env_)
	{
		throw QJniThreadAttachException(
			QString(QLatin1String("Attempted use of null Java environment, thread %1"))
				.arg(gettid())
				.toLatin1());
	}
}


JNIEnv * QJniEnvPtr::env() const
{
	return env_;
}


bool QJniEnvPtr::preloadClass(const char * class_name)
{
	checkEnv();
	QMutexLocker locker(&g_PreloadedClassesMutex);
	const QString class_name_qstr = QLatin1String(class_name);
	if (g_PreloadedClasses.contains(class_name_qstr)) {
		VERBOSE(qWarning("Class already preloaded: \"%s\" as %p for tid %d",
			class_name, gclazz, (int)gettid()));
		return true;
	}
	VERBOSE(qWarning("Preloading class \"%s\"", class_name));
	QJniLocalRef clazz(env_, env_->FindClass(class_name)); // jclass
	if (clearException() || clazz.jObject() == 0)
	{
		qWarning("Failed to preload class %s (tid %d)", class_name, static_cast<int>(gettid()));
		return false;
	}
	jclass gclazz = static_cast<jclass>(env_->NewGlobalRef(clazz));
	g_PreloadedClasses.insert(class_name_qstr, gclazz);
	VERBOSE(qWarning("...Preloaded class \"%s\" as %p for tid %d",
		class_name, gclazz, (int)gettid()));
	return true;
}


int QJniEnvPtr::preloadClasses(const char * const * class_list)
{
	checkEnv();
	int loaded = 0;
	for(; *class_list != 0; ++class_list)
	{
		if (preloadClass(*class_list))
		{
			loaded++;
		}
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
	checkEnv();
	QMutexLocker locker(&g_PreloadedClassesMutex);
	for (PreloadedClasses::iterator it = g_PreloadedClasses.begin(); it != g_PreloadedClasses.end(); ++it)
	{
		env_->DeleteGlobalRef(it.value());
	}
	g_PreloadedClasses.clear();
}


jclass QJniEnvPtr::findClass(const char * name)
{
	checkEnv();
	// First try find a preloaded class
	{
		QMutexLocker locker(&g_PreloadedClassesMutex);
		VERBOSE(qWarning("Searching for class \"%s\" in tid %d", name, (int)gettid()));
		PreloadedClasses::iterator it = g_PreloadedClasses.find(QLatin1String(name));
		if (it != g_PreloadedClasses.end())
		{
			return it.value();
		}
	}

	// If it wasn't preloaded, try to load it in JNI (will fail for custom classes in native-created threads)
	VERBOSE(qWarning("Trying to construct the class directly: \"%s\" in tid %d", name, (int)gettid()));
	QJniLocalRef cls(env_, env_->FindClass(name)); // jclass
	if (clearException())
	{
		qWarning("Failed to find class \"%s\"", name);
		return 0;
	}

	// We must store class ref in a global ref
	jclass ret = static_cast<jclass>(env_->NewGlobalRef(cls));

	// Add it to a list of preloaded classes for convenience
	QMutexLocker locker(&g_PreloadedClassesMutex);
	g_PreloadedClasses.insert(QLatin1String(name), ret);

	VERBOSE(qWarning("Successfuly found Java class: \"%s\" in tid %d", name, (int)gettid()));

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
		int errsv = g_JavaVm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6);
		if (errsv == JNI_OK && env)
		{
			return true;
		}
	}
	return false;
}


void QJniEnvPtr::setJavaVM(JavaVM* vm)
{
	// qWarning()<<"SetJavaVM"<<vm;
	g_JavaVm = vm;
}


void QJniEnvPtr::setJavaVM(JNIEnv* env)
{
	env->GetJavaVM(&g_JavaVm);
}


jstring QJniEnvPtr::JStringFromQString(const QString & str)
{
	checkEnv();
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
	checkEnv();
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

	QString ret = QString(reinterpret_cast<const QChar *>(str_ptr), length);
	env_->ReleaseStringChars(str, str_ptr);
	return ret;
}


std::vector<bool> QJniEnvPtr::convert(jbooleanArray jarray)
{
	checkEnv();
	std::vector<bool> result;
	if (!jarray)
	{
		return result;
	}
	const int count  = env_->GetArrayLength(jarray);
	if (count)
	{
		jboolean * elements = env_->GetBooleanArrayElements(jarray, nullptr);
		result.reserve(count);
		const jboolean * ptr = elements;
		for (int i = 0; i < count; i++, ptr++)
		{
			result.push_back(static_cast<bool>(*ptr));
		}
		env_->ReleaseBooleanArrayElements(jarray, elements, JNI_ABORT);
	}
	return result;
}


std::vector<jint> QJniEnvPtr::convert(jintArray jarray)
{
	checkEnv();
	std::vector<jint> result;
	if (!jarray)
	{
		return result;
	}
	const int count  = env_->GetArrayLength(jarray);
	if (count)
	{
		jint * elements = env_->GetIntArrayElements(jarray, nullptr);
		result.reserve(count);
		const jint * ptr = elements;
		for (int i = 0; i < count; i++, ptr++)
		{
			result.push_back(*ptr);
		}
		env_->ReleaseIntArrayElements(jarray, elements, JNI_ABORT);
	}
	return result;
}


std::vector<jlong> QJniEnvPtr::convert(jlongArray jarray)
{
	checkEnv();
	std::vector<jlong> result;
	if (!jarray)
	{
		return result;
	}
	const int count  = env_->GetArrayLength(jarray);
	if (count)
	{
		jlong * elements = env_->GetLongArrayElements(jarray, nullptr);
		result.reserve(count);
		const jlong * ptr = elements;
		for (int i = 0; i < count; i++, ptr++)
		{
			result.push_back(*ptr);
		}
		env_->ReleaseLongArrayElements(jarray, elements, JNI_ABORT);
	}
	return result;
}


std::vector<jfloat> QJniEnvPtr::convert(jfloatArray jarray)
{
	checkEnv();
	std::vector<jfloat> result;
	if (!jarray)
	{
		return result;
	}
	const int count  = env_->GetArrayLength(jarray);
	if (count)
	{
		jfloat * elements = env_->GetFloatArrayElements(jarray, nullptr);
		result.reserve(count);
		const jfloat * ptr = elements;
		for (int i = 0; i < count; i++, ptr++)
		{
			result.push_back(*ptr);
		}
		env_->ReleaseFloatArrayElements(jarray, elements, JNI_ABORT);
	}
	return result;
}


std::vector<jdouble> QJniEnvPtr::convert(jdoubleArray jarray)
{
	checkEnv();
	std::vector<double> result;
	if (!jarray)
	{
		return result;
	}
	const int count  = env_->GetArrayLength(jarray);
	if (count)
	{
		double * elements = env_->GetDoubleArrayElements(jarray, nullptr);
		result.reserve(count);
		const double * ptr = elements;
		for (int i = 0; i < count; i++, ptr++)
		{
			result.push_back(*ptr);
		}
		env_->ReleaseDoubleArrayElements(jarray, elements, JNI_ABORT);
	}
	return result;
}


std::vector<QJniObject> QJniEnvPtr::convert(jobjectArray jarray)
{
	checkEnv();
	std::vector<QJniObject> result;
	if (!jarray)
	{
		return result;
	}
	const int count  = env_->GetArrayLength(jarray);
	if (count)
	{
		result.reserve(count);
		for (int i = 0; i < count; i++)
		{
			result.emplace_back(env_->GetObjectArrayElement(jarray, static_cast<jsize>(i)), true);
		}
	}
	return result;
}


bool QJniEnvPtr::clearException(bool describe /*= true*/)
{
	checkEnv();
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


QJniClass::QJniClass()
	: class_(0)
{
}


QJniClass::QJniClass(jclass clazz)
	: class_(0)
{
	QJniEnvPtr jep;
	initClass(jep.env(), clazz);
}


QJniClass::QJniClass(const char * full_class_name)
	: class_(0)
{
	if (!full_class_name)
	{
		throw QJniBaseException("Null class name in QJniClass::QJniClass");
	}
	if (!(*full_class_name))
	{
		throw QJniBaseException("Empty class name in QJniClass::QJniClass");
	}
	construction_class_name_ = full_class_name;

	QJniEnvPtr jep;
	jclass cls = jep.findClass(full_class_name); // this is a preloaded global ref, we don't need to delete it as a local ref
	if (jep.clearException())
	{
		throw QJniBaseException("Exception in JniEnvPtr::findClass");
	}
	if (cls == 0)
	{
		throw QJniClassNotFoundException(full_class_name);
	}
	VERBOSE(qWarning("Class \"%s\" is loaded @ %p", full_class_name, cls));

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


QJniClass::QJniClass(const QJniClass & other)
	: class_(0)
	, construction_class_name_(other.construction_class_name_)
{
	QJniEnvPtr jep;
	initClass(jep.env(), other.class_);
}


QJniClass::QJniClass(QJniClass && other)
{
	class_ = other.class_;
	other.class_ = nullptr;
	construction_class_name_ = std::move(other.construction_class_name_);
}


QJniClass::~QJniClass()
{
	VERBOSE(qWarning("QJniClass::~QJniClass() %p",this));
	QJniEnvPtr jep;
	clearClass(jep.env());
}


bool QJniClass::classAvailable(const char * full_class_name)
{
	QJniEnvPtr jep;
	const jclass cls = jep.findClass(full_class_name);
	if (jep.clearException())
	{
		return false;
	}
	return cls != nullptr;
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
			throw QJniBaseException("Exception near JniEnvPtr::NewGlobalRef");
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
	VERBOSE(qWarning("void QJniClass::CallStaticVoid(const char* method_name) %p \"%s\"", reinterpret_cast<void*>(this), method_name));
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();
	jmethodID mid = env->GetStaticMethodID(checkedClass(__FUNCTION__), method_name, "()V");
	if (!mid)
	{
		throw QJniMethodNotFoundException(debugClassName().constData(), method_name, __FUNCTION__);
	}
	env->CallStaticVoidMethod(class_, mid);
	if (jep.clearException())
	{
		throw QJniJavaCallException(debugClassName().constData(), method_name, __FUNCTION__);
	}
}


jint QJniClass::callStaticInt(const char* method_name)
{
	VERBOSE(qWarning("void QJniClass::callStaticInt(const char* method_name) %p \"%s\"", reinterpret_cast<void*>(this), method_name));
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();
	jmethodID mid = env->GetStaticMethodID(checkedClass(__FUNCTION__), method_name, "()I");
	if (!mid)
	{
		throw QJniMethodNotFoundException(debugClassName().constData(), method_name, __FUNCTION__);
	}
	jint result = env->CallStaticIntMethod(jClass(), mid);
	if (jep.clearException())
	{
		throw QJniJavaCallException(debugClassName().constData(), method_name, __FUNCTION__);
	}
	return result;
}


jlong QJniClass::callStaticLong(const char* method_name)
{
	VERBOSE(qWarning("void QJniClass::callStaticLong(const char* method_name) %p \"%s\"", reinterpret_cast<void*>(this), method_name));
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();
	jmethodID mid = env->GetStaticMethodID(checkedClass(__FUNCTION__), method_name, "()J");
	if (!mid)
	{
		throw QJniMethodNotFoundException(debugClassName().constData(), method_name, __FUNCTION__);
	}
	jlong result = env->CallStaticLongMethod(jClass(), mid);
	if (jep.clearException())
	{
		throw QJniJavaCallException(debugClassName().constData(), method_name, __FUNCTION__);
	}
	return result;
}


bool QJniClass::callStaticBoolean(const char* method_name)
{
	VERBOSE(qWarning("void QJniClass::callStaticBoolean(const char* method_name) %p \"%s\"", reinterpret_cast<void*>(this), method_name));
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();
	jmethodID mid = env->GetStaticMethodID(checkedClass(__FUNCTION__), method_name, "()Z");
	if (!mid)
	{
		throw QJniMethodNotFoundException(debugClassName().constData(), method_name, __FUNCTION__);
	}
	bool result = env->CallStaticBooleanMethod(jClass(), mid)? true: false;
	if (jep.clearException())
	{
		throw QJniJavaCallException(debugClassName().constData(), method_name, __FUNCTION__);
	}
	return result;
}


void QJniClass::callStaticParamVoid(const char * method_name, const char * param_signature, ...)
{
	VERBOSE(qWarning("void QJniClass(%p)::CallStaticParamVoid(\"%s\", \"%s\", ...)", reinterpret_cast<void*>(this), method_name, param_signature));
	va_list args;
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();

	QByteArray signature("(");
	signature += param_signature;
	signature += ")V";
	jmethodID mid = env->GetStaticMethodID(checkedClass(__FUNCTION__), method_name, signature.data());
	if (!mid)
	{
		throw QJniMethodNotFoundException(debugClassName().constData(), method_name, __FUNCTION__);
	}

	va_start(args, param_signature);
	env->CallStaticVoidMethodV(jClass(), mid, args);
	va_end(args);
	if (jep.clearException())
	{
		throw QJniJavaCallException(debugClassName().constData(), method_name, __FUNCTION__);
	}
}


bool QJniClass::callStaticParamBoolean(const char * method_name, const char * param_signature, ...)
{
	VERBOSE(qWarning("void QJniClass(%p)::CallStaticParamBoolean(\"%s\", \"%s\", ...)", reinterpret_cast<void*>(this), method_name, param_signature));
	va_list args;
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();

	QByteArray signature("(");
	signature += param_signature;
	signature += ")Z";
	jmethodID mid = env->GetStaticMethodID(checkedClass(__FUNCTION__), method_name, signature.data());
	if (!mid)
	{
		throw QJniMethodNotFoundException(debugClassName().constData(), method_name, __FUNCTION__);
	}

	va_start(args, param_signature);
	bool result = env->CallStaticBooleanMethodV(jClass(), mid, args);
	va_end(args);
	if (jep.clearException())
	{
		throw QJniJavaCallException(debugClassName().constData(), method_name, __FUNCTION__);
	}
	return result;
}


jint QJniClass::callStaticParamInt(const char * method_name, const char * param_signature, ...)
{
	VERBOSE(qWarning("void QJniClass(%p)::CallStaticParamInt(\"%s\", \"%s\", ...)", reinterpret_cast<void*>(this), method_name, param_signature));
	va_list args;
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();

	QByteArray signature("(");
	signature += param_signature;
	signature += ")I";
	jmethodID mid = env->GetStaticMethodID(checkedClass(__FUNCTION__), method_name, signature.data());
	if (!mid)
	{
		throw QJniMethodNotFoundException(debugClassName().constData(), method_name, __FUNCTION__);
	}

	va_start(args, param_signature);
	jint result = env->CallStaticIntMethodV(jClass(), mid, args);
	va_end(args);
	if (jep.clearException())
	{
		throw QJniJavaCallException(debugClassName().constData(), method_name, __FUNCTION__);
	}
	return result;
}


jlong QJniClass::callStaticParamLong(const char * method_name, const char * param_signature, ...)
{
	VERBOSE(qWarning("void QJniClass(%p)::CallStaticParamLong(\"%s\", \"%s\", ...)", reinterpret_cast<void*>(this), method_name, param_signature));
	va_list args;
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();

	QByteArray signature("(");
	signature += param_signature;
	signature += ")J";
	jmethodID mid = env->GetStaticMethodID(checkedClass(__FUNCTION__), method_name, signature.data());
	if (!mid)
	{
		throw QJniMethodNotFoundException(debugClassName().constData(), method_name, __FUNCTION__);
	}

	va_start(args, param_signature);
	jlong result = env->CallStaticLongMethodV(jClass(), mid, args);
	va_end(args);
	if (jep.clearException())
	{
		throw QJniJavaCallException(debugClassName().constData(), method_name, __FUNCTION__);
	}
	return result;
}


jfloat QJniClass::callStaticParamFloat(const char * method_name, const char * param_signature, ...)
{
	VERBOSE(qWarning("void QJniClass(%p)::CallStaticParamFloat(\"%s\", \"%s\", ...)", reinterpret_cast<void*>(this), method_name, param_signature));
	va_list args;
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();

	QByteArray signature("(");
	signature += param_signature;
	signature += ")F";
	jmethodID mid = env->GetStaticMethodID(checkedClass(__FUNCTION__), method_name, signature.data());
	if (!mid)
	{
		throw QJniMethodNotFoundException(debugClassName().constData(), method_name, __FUNCTION__);
	}

	va_start(args, param_signature);
	jfloat result = env->CallStaticFloatMethodV(jClass(), mid, args);
	va_end(args);
	if (jep.clearException())
	{
		throw QJniJavaCallException(debugClassName().constData(), method_name, __FUNCTION__);
	}
	return result;
}


QString QJniClass::callStaticParamString(const char * method_name, const char * param_signature, ...)
{
	VERBOSE(qWarning("void QJniClass(%p)::CallStaticParamString(\"%s\", \"%s\", ...)", reinterpret_cast<void*>(this), method_name, param_signature));
	va_list args;
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();

	QByteArray signature("(");
	signature += param_signature;
	signature += ")Ljava/lang/String;";
	jmethodID mid = env->GetStaticMethodID(checkedClass(__FUNCTION__), method_name, signature.data());
	if (!mid)
	{
		throw QJniMethodNotFoundException(debugClassName().constData(), method_name, __FUNCTION__);
	}

	va_start(args, param_signature);
	QString ret = QJniLocalRef(env, env->CallStaticObjectMethodV(jClass(), mid, args));
	va_end(args);
	if (jep.clearException())
	{
		throw QJniJavaCallException(debugClassName().constData(), method_name, __FUNCTION__);
	}
	return ret;
}


void QJniClass::callStaticVoid(const char* method_name, const QString & string)
{
	callStaticParamVoid(method_name, "Ljava/lang/String;", QJniLocalRef(string).jObject());
}


QString QJniClass::callStaticString(const char *method_name)
{
	VERBOSE(qWarning("QString QJniClass::CallStaticString(const char* method_name) %p \"%s\"", reinterpret_cast<void*>(this), method_name));
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();
	jmethodID mid = env->GetStaticMethodID(checkedClass(__FUNCTION__), method_name, "()Ljava/lang/String;");
	if (!mid)
	{
		throw QJniMethodNotFoundException(debugClassName().constData(), method_name, __FUNCTION__);
	}
	QString ret = QJniLocalRef(env, env->CallStaticObjectMethod(jClass(), mid));
	if (jep.clearException())
	{
		throw QJniJavaCallException(debugClassName().constData(), method_name, __FUNCTION__);
	}
	return ret;
}


QJniObject * QJniClass::getStaticObjectField(const char * field_name, const char * objname)
{
	VERBOSE(qWarning("int QJniObject::getStaticObjectField(const char * field_name, const char * objname) %p \"%s\"", reinterpret_cast<void*>(this), field_name, objname));
	QByteArray obj;
	appendNormalizedObjectName(obj, objname);
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();
	jfieldID fid = env->GetStaticFieldID(checkedClass(__FUNCTION__), field_name, obj.data());
	if (!fid)
	{
		throw QJniFieldNotFoundException(debugClassName().constData(), field_name, __FUNCTION__);
	}
	jobject jret = env->GetStaticObjectField(jClass(), fid);
	if (jep.clearException())
	{
		if (jret)
		{
			env->DeleteLocalRef(jret);
		}
		throw QJniJavaCallException(
			debugClassName().constData()
			, QByteArray().append(field_name).append("->").append(objname)
			, __FUNCTION__);
	}
	if (!jret)
	{
		return nullptr;
	}
	return new QJniObject(jret, true, objname);
}


QString QJniClass::getStaticStringField(const char * field_name)
{
	VERBOSE(qWarning("int QJniObject::getStaticStringField(const char * field_name) %p \"%s\"", reinterpret_cast<void*>(this), field_name));
	QJniEnvPtr jep;
	JNIEnv * env = jep.env();
	jfieldID fid = env->GetStaticFieldID(checkedClass(__FUNCTION__), field_name, "Ljava/lang/String;");
	if (!fid)
	{
		throw QJniFieldNotFoundException(debugClassName().constData(), field_name, __FUNCTION__);
	}
	QString ret = QJniLocalRef(env, env->GetStaticObjectField(class_, fid));
	if (jep.clearException())
	{
		throw QJniJavaCallException(debugClassName().constData(), field_name, __FUNCTION__);
	}
	return ret;
}


jint QJniClass::getStaticIntField(const char * field_name)
{
	VERBOSE(qWarning("int QJniObject::getStaticIntField(const char * field_name) %p \"%s\"", reinterpret_cast<void*>(this), field_name));
	QJniEnvPtr jep;
	JNIEnv * env = jep.env();
	jfieldID fid = env->GetStaticFieldID(checkedClass(__FUNCTION__), field_name, "I");
	if (!fid)
	{
		throw QJniFieldNotFoundException(debugClassName().constData(), field_name, __FUNCTION__);
	}
	jint result = env->GetStaticIntField(jClass(), fid);
	if (jep.clearException())
	{
		throw QJniJavaCallException(debugClassName().constData(), field_name, __FUNCTION__);
	}
	return result;
}


bool QJniClass::getStaticBooleanField(const char * field_name)
{
	VERBOSE(qWarning("int QJniObject::getStaticBooleanField(const char * field_name) %p \"%s\"", reinterpret_cast<void*>(this), field_name));
	QJniEnvPtr jep;
	JNIEnv * env = jep.env();
	jfieldID fid = env->GetStaticFieldID(checkedClass(__FUNCTION__), field_name, "Z");
	if (!fid)
	{
		throw QJniFieldNotFoundException(debugClassName().constData(), field_name, __FUNCTION__);
	}
	jint result = env->GetStaticBooleanField(jClass(), fid);
	if (jep.clearException())
	{
		throw QJniJavaCallException(debugClassName().constData(), field_name, __FUNCTION__);
	}
	return (result)? true: false;
}


QJniObject* QJniClass::callStaticObject(const char * method_name, const char * objname)
{
	VERBOSE(qWarning("QJniClass::CallStaticObject(\"%s\",\"%s\")", method_name, objname));
	QByteArray signature("()");
	appendNormalizedObjectName(signature, objname);

	VERBOSE(qWarning("QJniClass::CallStaticObject signature: %s", signature.data()));
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();

	VERBOSE(qWarning("env->GetStaticMethodID"));
	jmethodID mid = env->GetStaticMethodID(checkedClass(__FUNCTION__), method_name, signature.data());
	if (!mid)
	{
		throw QJniMethodNotFoundException(debugClassName().constData(), method_name, __FUNCTION__);
	}

	VERBOSE(qWarning("new QJniClass(env->CallStaticObjectMethod(jClass(),mid), true);"));
	jobject jret = env->CallStaticObjectMethod(jClass(), mid);
	if (jep.clearException())
	{
		if (jret)
		{
			env->DeleteLocalRef(jret);
		}
		throw QJniJavaCallException(
			debugClassName().constData()
			, QByteArray().append(method_name).append("->").append(objname).constData()
			, __FUNCTION__);
	}
	if (!jret)
	{
		return nullptr;
	}
	return new QJniObject(jret, true, objname);
}


QJniObject * QJniClass::callStaticParamObject(const char * method_name, const char * objname, const char * param_signature, ...)
{
	VERBOSE(qWarning("void QJniClass(%p)::callStaticParamObject(\"%s\", \"%s\", \"%s\"...)", reinterpret_cast<void*>(this), method_name, objname, param_signature));
	va_list args;
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();

	QByteArray signature;
	makeObjectFunctionSignature(signature, param_signature, objname);

	jmethodID mid = env->GetStaticMethodID(checkedClass(__FUNCTION__), method_name, signature.data());
	if (!mid)
	{
		throw QJniMethodNotFoundException(debugClassName().constData(), method_name, __FUNCTION__);
	}

	va_start(args, param_signature);
	jobject jret = env->CallStaticObjectMethodV(jClass(), mid, args);
	va_end(args);
	if (jep.clearException())
	{
		qWarning("void QJniClass(%p)::callStaticParamObject(\"%s\", \"%s\", ...): exception occured", reinterpret_cast<void*>(this), method_name, param_signature);
		if (jret)
		{
			env->DeleteLocalRef(jret);
		}
		throw QJniJavaCallException(
			debugClassName().constData()
			, QByteArray().append(method_name).append("->").append(objname).constData()
			, __FUNCTION__);
	}
	if (!jret)
	{
		return nullptr;
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
	jint result = jep.env()->RegisterNatives(
		checkedClass(__FUNCTION__)
		, methods_list
		, static_cast<jint>(sizeof_methods_list/sizeof(JNINativeMethod)));
	if (jep.clearException())
	{
		throw QJniJavaCallException(debugClassName().constData(), "registerNativeMethods", __FUNCTION__);
	}
	return result == 0;
}


bool QJniClass::unregisterNativeMethods()
{
	QJniEnvPtr jep;
	jint result = jep.env()->UnregisterNatives(checkedClass(__FUNCTION__));
	if (jep.clearException())
	{
		throw QJniJavaCallException(debugClassName().constData(), "unregisterNativeMethods", __FUNCTION__);
	}
	return result == 0;
}


QString QJniClass::getClassName(bool simple) const
{
	QString result;
	if (jClass())
	{
		// Not recursing into QJniObject::QJniObject to avoid stack overflow in case we want to
		// print class names from there. Otherwise we could simply do something like this:
		// return QJniObject(QJniEnvPtr().env()->GetObjectClass(jClass()), true).callString("getSimpleName");
		QJniEnvPtr jep;
		if (jclass classClazz = jep.env()->GetObjectClass(jClass()))
		{
			if (jmethodID methodId = jep.env()->GetMethodID(
				classClazz
				, (simple) ? "getSimpleName" : "getName"
				, "()Ljava/lang/String;"))
			{
				if (jstring className = static_cast<jstring>(jep.env()->CallObjectMethod(jClass(), methodId)))
				{
					result = jep.JStringToQString(className);
					jep.env()->DeleteLocalRef(className);
				}
			}
			jep.env()->DeleteLocalRef(classClazz);
		}
		jep.clearException();
	}
	return result;
}


QByteArray QJniClass::debugClassName() const
{
	if (!construction_class_name_.isEmpty())
	{
		return construction_class_name_;
	}
	const QString java_name = getClassName(false);
	if (java_name.isEmpty())
	{
		return "<unknown>";
	}
	return java_name.toLatin1();
}



/////////////////////////////////////////////////////////////////////////////
// QJniObject
/////////////////////////////////////////////////////////////////////////////


QJniObject::QJniObject()
	: QJniClass()
	, instance_(0)
{
}


QJniObject::QJniObject(
		jobject instance,
		bool take_ownership,
		const char * known_class_name,
		bool known_can_have_null_class)
	: QJniClass(instance)
	, instance_(0)
{
	QJniEnvPtr jep;
	// Creates global reference
	initObject(
		jep.env(),
		instance,
		known_can_have_null_class || classObjectMayHaveNullClass(known_class_name));
	if (take_ownership)
	{
		jep.env()->DeleteLocalRef(instance);
	}

}


QJniObject::QJniObject(const QJniClass & clazz, const char * param_signature, ...)
	: QJniClass(clazz)
	, instance_(0)
{
	QJniEnvPtr jep;
	JNIEnv * env = jep.env();

	QByteArray signature("(");
	if (param_signature)
	{
		signature += param_signature;
	}
	signature += ")V";

	jmethodID mid_init = env->GetMethodID(checkedClass(__FUNCTION__), "<init>", signature.data());
	if (jep.clearException())
	{
		throw QJniBaseException("Exception near getting method id for <init>");
	}
	if (!mid_init)
	{
		throw QJniMethodNotFoundException(
			debugClassName().constData()
			, QByteArray("<init>(").append(signature).append(")")
			, __FUNCTION__);
	}

	va_list args;
	va_start(args, param_signature);
	QJniLocalRef obj(env, env->NewObjectV(jClass(), mid_init, args));
	va_end(args);
	if (jep.clearException())
	{
		throw QJniBaseException("Exception in JniEnvPtr::NewObjectV");
	}

	// it is dangerous to go alone, use this
	initObject(env, obj);
}


QJniObject::QJniObject(const char * class_name, const char * param_signature, ...)
	: QJniClass(class_name)
	, instance_(0)
{
	QJniEnvPtr jep;
	JNIEnv * env = jep.env();

	QByteArray signature("(");
	if (param_signature)
	{
		signature += param_signature;
	}
	signature += ")V";

	jmethodID mid_init = env->GetMethodID(checkedClass(__FUNCTION__), "<init>", signature.data());
	if (jep.clearException())
	{
		throw QJniBaseException("Exception near getting method id for <init>");
	}
	if (!mid_init)
	{
		throw QJniMethodNotFoundException(
			debugClassName().constData()
			, QByteArray("<init>(").append(signature).append(")")
			, __FUNCTION__);
	}

	va_list args;
	va_start(args, param_signature);
	QJniLocalRef obj(env, env->NewObjectV(jClass(), mid_init, args));
	va_end(args);
	if (jep.clearException())
	{
		throw QJniBaseException("Exception in JniEnvPtr::NewObjectV");
	}
	if (!obj.jObject())
	{
		throw QJniBaseException(QByteArray("Object constructor returned null for ").append(class_name));
	}

	// it is dangerous to go alone, use this
	initObject(env, obj);
}


QJniObject::QJniObject(const QJniObject & other)
	: QJniClass(other)
{
	QJniEnvPtr jep;
	if (other.jObject())
	{
		instance_ = jep.env()->NewGlobalRef(other.jObject());
	}
}


QJniObject::QJniObject(QJniObject && other)
	: QJniClass(other)
{
	instance_ = other.instance_;
	other.instance_ = nullptr;
}


QJniObject::~QJniObject()
{
	VERBOSE(qWarning("QJniObject::~QJniObject() %p",this));
	QJniEnvPtr jep;
	if (instance_ != 0)
	{
		jep.env()->DeleteGlobalRef(instance_);
	}
}


void QJniObject::initObject(JNIEnv * env, jobject instance, bool can_have_null_class)
{
	VERBOSE(qWarning("QJniObject::initObject(JNIEnv* env, jobject %p) %p, class %s", instance, reinterpret_cast<void*>(this), getClassName().toLatin1().data()));
	if (!can_have_null_class)
	{
		checkedClass(__FUNCTION__);
	}
	QJniEnvPtr jep(env);
	instance_ = env->NewGlobalRef(instance);
	if (jep.clearException())
	{
		throw QJniBaseException("Exception near JniEnvPtr::NewGlobalRef");
	}
	if (instance && !instance_)
	{
		throw QJniBaseException("Failed to make additional global reference to an existing object.");
	}
	#if 0 // Reference logging
		qWarning() << QString(QLatin1String("QJniObject::initObject: creating %1: 0x%2 => 0x%3"))
			.arg(getClassName())
			.arg(reinterpret_cast<unsigned long>(instance), 0, 16)
			.arg(reinterpret_cast<unsigned long>(instance_), 0, 16);
	#endif
}


void QJniObject::callVoid(const char* method_name)
{
	VERBOSE(qWarning("void QJniObject::CallVoid(const char* method_name) %p \"%s\"", reinterpret_cast<void*>(this), method_name));
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();
	jmethodID mid = env->GetMethodID(checkedClass(__FUNCTION__), method_name, "()V");
	if (!mid)
	{
		throw QJniMethodNotFoundException(debugClassName().constData(), method_name, __FUNCTION__);
	}
	env->CallVoidMethod(instance_, mid);
	if (jep.clearException())
	{
		throw QJniJavaCallException(debugClassName().constData(), method_name, __FUNCTION__);
	}
}


bool QJniObject::callBool(const char* method_name)
{
	VERBOSE(qWarning("bool QJniObject::CallBool(const char* method_name) %p \"%s\"", reinterpret_cast<void*>(this), method_name));
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();
	jmethodID mid = env->GetMethodID(checkedClass(__FUNCTION__), method_name, "()Z");
	if (!mid)
	{
		throw QJniMethodNotFoundException(debugClassName().constData(), method_name, __FUNCTION__);
	}
	bool result = (JNI_TRUE==env->CallBooleanMethod(instance_, mid));
	if (jep.clearException())
	{
		throw QJniJavaCallException(debugClassName().constData(), method_name, __FUNCTION__);
	}
	return result;
}


bool QJniObject::callBool(const char* method_name, bool param)
{
	VERBOSE(qWarning("bool QJniObject::CallBool(const char* method_name, bool param) %p \"%s\"", reinterpret_cast<void*>(this), method_name));
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();
	jmethodID mid = env->GetMethodID(checkedClass(__FUNCTION__), method_name, "(Z)Z");
	if (!mid)
	{
		throw QJniMethodNotFoundException(debugClassName().constData(), method_name, __FUNCTION__);
	}
	bool result = (JNI_TRUE==env->CallBooleanMethod(instance_, mid, jboolean(param)));
	if (jep.clearException())
	{
		throw QJniJavaCallException(debugClassName().constData(), method_name, __FUNCTION__);
	}
	return result;
}


int QJniObject::callInt(const char* method_name)
{
	VERBOSE(qWarning("int QJniObject::CallInt(const char* method_name) %p \"%s\"", reinterpret_cast<void*>(this), method_name));
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();
	jmethodID mid = env->GetMethodID(checkedClass(__FUNCTION__), method_name, "()I");
	if (!mid)
	{
		throw QJniMethodNotFoundException(debugClassName().constData(), method_name, __FUNCTION__);
	}
	int result = static_cast<int>(env->CallIntMethod(instance_, mid));
	if (jep.clearException())
	{
		throw QJniJavaCallException(debugClassName().constData(), method_name, __FUNCTION__);
	}
	return result;
}


long long QJniObject::callLong(const char* method_name)
{
	VERBOSE(qWarning("int QJniObject::CallLong(const char* method_name) %p \"%s\"", reinterpret_cast<void*>(this), method_name));
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();
	jmethodID mid = env->GetMethodID(checkedClass(__FUNCTION__), method_name, "()J");
	if (!mid)
	{
		throw QJniMethodNotFoundException(debugClassName().constData(), method_name, __FUNCTION__);
	}
	long long result = static_cast<long long>(env->CallLongMethod(instance_, mid));
	if (jep.clearException())
	{
		throw QJniJavaCallException(debugClassName().constData(), method_name, __FUNCTION__);
	}
	return result;
}


float QJniObject::callFloat(const char* method_name)
{
	VERBOSE(qWarning("float QJniObject::CallFloat(const char* method_name) %p \"%s\"", reinterpret_cast<void*>(this), method_name));
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();
	jmethodID mid = env->GetMethodID(checkedClass(__FUNCTION__), method_name, "()F");
	if (!mid)
	{
		throw QJniMethodNotFoundException(debugClassName().constData(), method_name, __FUNCTION__);
	}
	float result = static_cast<float>(env->CallFloatMethod(instance_, mid));
	if (jep.clearException())
	{
		throw QJniJavaCallException(debugClassName().constData(), method_name, __FUNCTION__);
	}
	return result;
}


float QJniObject::callFloat(const char* method_name, int param)
{
	VERBOSE(qWarning("float QJniObject::CallFloat(const char* method_name) %p \"%s\" (%d)", reinterpret_cast<void*>(this), method_name, param));
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();
	jmethodID mid = env->GetMethodID(checkedClass(__FUNCTION__), method_name, "(I)F");
	if (!mid)
	{
		throw QJniMethodNotFoundException(debugClassName().constData(), method_name, __FUNCTION__);
	}
	float result = static_cast<float>(env->CallFloatMethod(instance_, mid, jint(param)));
	if (jep.clearException())
	{
		throw QJniJavaCallException(debugClassName().constData(), method_name, __FUNCTION__);
	}
	return result;
}


double QJniObject::callDouble(const char* method_name)
{
	VERBOSE(qWarning("float QJniObject::CallDouble(const char* method_name) %p \"%s\"", reinterpret_cast<void*>(this), method_name));
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();
	jmethodID mid = env->GetMethodID(checkedClass(__FUNCTION__), method_name, "()D");
	if (!mid)
	{
		throw QJniMethodNotFoundException(debugClassName().constData(), method_name, __FUNCTION__);
	}
	double result = static_cast<double>(env->CallDoubleMethod(instance_, mid));
	if (jep.clearException())
	{
		throw QJniJavaCallException(debugClassName().constData(), method_name, __FUNCTION__);
	}
	return result;
}


QJniObject * QJniObject::callObject(const char * method_name, const char * objname)
{
	QByteArray signature("()");
	appendNormalizedObjectName(signature, objname);

	VERBOSE(qWarning("QJniObject::callObject: \"%s\", \"%s\"", method_name, signature.data()));
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();
	jmethodID mid = env->GetMethodID(checkedClass(__FUNCTION__), method_name, signature.data());
	if (!mid)
	{
		throw QJniMethodNotFoundException(debugClassName().constData(), method_name, __FUNCTION__);
	}
	jobject jret = env->CallObjectMethod(instance_, mid);
	if (jep.clearException())
	{
		if (jret)
		{
			env->DeleteLocalRef(jret);
		}
		throw QJniJavaCallException(
			debugClassName().constData()
			, QByteArray().append(method_name).append("->").append(objname).constData()
			, __FUNCTION__);
	}
	if (!jret)
	{
		return nullptr;
	}
	QJniObject * result = new QJniObject(jret, true, objname);
	return result;
}


QJniObject * QJniObject::callParamObject(const char * method_name, const char * objname, const char * param_signature, ...)
{
	VERBOSE(qWarning("void QJniObject(%p)::callParamObject(\"%s\", \"%s\", \"%s\"...)", reinterpret_cast<void*>(this), method_name, objname, param_signature));

	va_list args;
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();

	QByteArray signature;
	makeObjectFunctionSignature(signature, param_signature, objname);

	jmethodID mid = env->GetMethodID(checkedClass(__FUNCTION__), method_name, signature.data());
	if (!mid)
	{
		throw QJniMethodNotFoundException(debugClassName().constData(), method_name, __FUNCTION__);
	}

	va_start(args, param_signature);
	jobject jret = env->CallObjectMethodV(instance_, mid, args);
	va_end(args);
	if (jep.clearException())
	{
		if (jret)
		{
			env->DeleteLocalRef(jret);
		}
		throw QJniJavaCallException(
			debugClassName().constData()
			, QByteArray().append(method_name).append("->").append(objname).constData()
			, __FUNCTION__);
	}
	if (!jret)
	{
		return nullptr;
	}
	return new QJniObject(jret, true, objname);
}


jint QJniObject::callParamInt(const char* method_name, const char* param_signature, ...)
{
	VERBOSE(qWarning("void QJniObject(%p)::callParamInt(\"%s\", \"%s\", ...)", reinterpret_cast<void*>(this), method_name, param_signature));
	va_list args;
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();
	QByteArray signature("(");
	signature += param_signature;
	signature += ")I";
	jmethodID mid = env->GetMethodID(checkedClass(__FUNCTION__), method_name, signature.data());
	if (!mid)
	{
		throw QJniMethodNotFoundException(debugClassName().constData(), method_name, __FUNCTION__);
	}
	va_start(args, param_signature);
	jint result = env->CallIntMethodV(instance_, mid, args);
	va_end(args);
	if (jep.clearException())
	{
		throw QJniJavaCallException(debugClassName().constData(), method_name, __FUNCTION__);
	}
	return result;
}


jlong QJniObject::callParamLong(const char* method_name, const char* param_signature, ...)
{
	VERBOSE(qWarning("void QJniObject(%p)::callParamLong(\"%s\", \"%s\", ...)", reinterpret_cast<void*>(this), method_name, param_signature));
	va_list args;
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();
	QByteArray signature("(");
	signature += param_signature;
	signature += ")J";
	jmethodID mid = env->GetMethodID(checkedClass(__FUNCTION__), method_name, signature.data());
	if (!mid)
	{
		throw QJniMethodNotFoundException(debugClassName().constData(), method_name, __FUNCTION__);
	}
	va_start(args, param_signature);
	jlong result = env->CallLongMethodV(instance_, mid, args);
	va_end(args);
	if (jep.clearException())
	{
		throw QJniJavaCallException(debugClassName().constData(), method_name, __FUNCTION__);
	}
	return result;
}


jfloat QJniObject::callParamFloat(const char* method_name, const char* param_signature, ...)
{
	VERBOSE(qWarning("void QJniObject(%p)::callParamFloat(\"%s\", \"%s\", ...)", reinterpret_cast<void*>(this), method_name, param_signature));
	va_list args;
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();
	QByteArray signature("(");
	signature += param_signature;
	signature += ")F";
	jmethodID mid = env->GetMethodID(checkedClass(__FUNCTION__), method_name, signature.data());
	if (!mid)
	{
		throw QJniMethodNotFoundException(debugClassName().constData(), method_name, __FUNCTION__);
	}
	va_start(args, param_signature);
	jfloat result = env->CallFloatMethodV(instance_, mid, args);
	va_end(args);
	if (jep.clearException())
	{
		throw QJniJavaCallException(debugClassName().constData(), method_name, __FUNCTION__);
	}
	return result;
}


jdouble QJniObject::callParamDouble(const char* method_name, const char* param_signature, ...)
{
	VERBOSE(qWarning("void QJniObject(%p)::callParamDouble(\"%s\", \"%s\", ...)", reinterpret_cast<void*>(this), method_name, param_signature));
	va_list args;
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();
	QByteArray signature("(");
	signature += param_signature;
	signature += ")D";
	jmethodID mid = env->GetMethodID(checkedClass(__FUNCTION__), method_name, signature.data());
	if (!mid)
	{
		throw QJniMethodNotFoundException(debugClassName().constData(), method_name, __FUNCTION__);
	}
	va_start(args, param_signature);
	jdouble result = env->CallFloatMethodV(instance_, mid, args);
	va_end(args);
	if (jep.clearException())
	{
		throw QJniJavaCallException(debugClassName().constData(), method_name, __FUNCTION__);
	}
	return result;
}


jboolean QJniObject::callParamBoolean(const char* method_name, const char* param_signature, ...)
{
	VERBOSE(qWarning("void QJniObject(%p)::callParamBoolean(\"%s\", \"%s\", ...)", reinterpret_cast<void*>(this), method_name, param_signature));
	va_list args;
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();
	QByteArray signature("(");
	signature += param_signature;
	signature += ")Z";
	jmethodID mid = env->GetMethodID(checkedClass(__FUNCTION__), method_name, signature.data());
	if (!mid)
	{
		throw QJniMethodNotFoundException(debugClassName().constData(), method_name, __FUNCTION__);
	}
	va_start(args, param_signature);
	jboolean result = env->CallBooleanMethodV(instance_, mid, args);
	va_end(args);
	if (jep.clearException())
	{
		throw QJniJavaCallException(debugClassName().constData(), method_name, __FUNCTION__);
	}
	return result;
}


QString QJniObject::callString(const char *method_name)
{
	VERBOSE(qWarning("QString QJniObject::CallString(const char* method_name) %p \"%s\"", reinterpret_cast<void*>(this), method_name));
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();
	jmethodID mid = env->GetMethodID(checkedClass(__FUNCTION__), method_name, "()Ljava/lang/String;");
	if (!mid)
	{
		throw QJniMethodNotFoundException(debugClassName().constData(), method_name, __FUNCTION__);
	}
	QString ret = QJniLocalRef(env, env->CallObjectMethod(instance_, mid));
	if (jep.clearException())
	{
		throw QJniJavaCallException(debugClassName().constData(), method_name, __FUNCTION__);
	}
	return ret;
}


QString QJniObject::callParamString(const char *method_name, const char* param_signature, ...)
{
	VERBOSE(qWarning("void QJniObject(%p)::callParamString(\"%s\", \"%s\", ...)", reinterpret_cast<void*>(this), method_name, param_signature));

	va_list args;
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();

	QByteArray signature("(");
	signature += param_signature;
	signature += ")Ljava/lang/String;";
	jmethodID mid = env->GetMethodID(checkedClass(__FUNCTION__), method_name, signature.data());

	if (!mid)
	{
		throw QJniMethodNotFoundException(debugClassName().constData(), method_name, __FUNCTION__);
	}

	va_start(args, param_signature);
	QString ret = QJniLocalRef(env, env->CallObjectMethodV(instance_, mid, args));
	va_end(args);

	if (jep.clearException())
	{
		throw QJniJavaCallException(debugClassName().constData(), method_name, __FUNCTION__);
	}

	return ret;
}


QString QJniObject::getString(const char *field_name)
{
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();
	jfieldID fid = env->GetFieldID(checkedClass(__FUNCTION__), field_name, "Ljava/lang/String;");
	if (!fid)
	{
		throw QJniFieldNotFoundException(debugClassName().constData(), field_name, __FUNCTION__);
	}
	QString ret = QJniLocalRef(env, env->GetObjectField(instance_, fid));
	if (jep.clearException())
	{
		throw QJniJavaCallException(debugClassName().constData(), field_name, __FUNCTION__);
	}
	return ret;
}


int QJniObject::getIntField(const char * field_name)
{
	VERBOSE(qWarning("int QJniObject::getIntField(const char* fieldd_name) %p \"%s\"",this,field_name));
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();
	jfieldID fid = env->GetFieldID(checkedClass(__FUNCTION__), field_name, "I");
	if (!fid)
	{
		throw QJniFieldNotFoundException(debugClassName().constData(), field_name, __FUNCTION__);
	}
	int result = static_cast<int>(env->GetIntField(instance_, fid));
	if (jep.clearException())
	{
		throw QJniJavaCallException(debugClassName().constData(), field_name, __FUNCTION__);
	}
	return result;
}

jlong QJniObject::getLongField(const char * field_name)
{
	VERBOSE(qWarning("int QJniObject::getLongField(const char* field_name) %p \"%s\"", reinterpret_cast<void*>(this), field_name));
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();
	jfieldID fid = env->GetFieldID(checkedClass(__FUNCTION__), field_name, "J");
	if (!fid)
	{
		throw QJniFieldNotFoundException(debugClassName().constData(), field_name, __FUNCTION__);
	}
	jlong result = env->GetLongField(instance_, fid);
	if (jep.clearException())
	{
		throw QJniJavaCallException(debugClassName().constData(), field_name, __FUNCTION__);
	}
	return result;
}


float QJniObject::getFloatField(const char * field_name)
{
	VERBOSE(qWarning("int QJniObject::getFloatField(const char* field_name) %p \"%s\"", reinterpret_cast<void*>(this), field_name));
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();
	jfieldID fid = env->GetFieldID(checkedClass(__FUNCTION__), field_name, "F");
	if (!fid)
	{
		throw QJniFieldNotFoundException(debugClassName().constData(), field_name, __FUNCTION__);
	}
	float result = static_cast<float>(env->GetFloatField(instance_, fid));
	if (jep.clearException())
	{
		throw QJniJavaCallException(debugClassName().constData(), field_name, __FUNCTION__);
	}
	return result;
}

double QJniObject::getDoubleField(const char * field_name)
{
	VERBOSE(qWarning("int QJniObject::getDoubleField(const char* field_name) %p \"%s\"", reinterpret_cast<void*>(this), field_name));
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();
	jfieldID fid = env->GetFieldID(checkedClass(__FUNCTION__), field_name, "D");
	if (!fid)
	{
		throw QJniFieldNotFoundException(debugClassName().constData(), field_name, __FUNCTION__);
	}
	double result = static_cast<double>(env->GetDoubleField(instance_, fid));
	if (jep.clearException())
	{
		throw QJniJavaCallException(debugClassName().constData(), field_name, __FUNCTION__);
	}
	return result;
}


jboolean QJniObject::getBooleanField(const char * field_name)
{
	VERBOSE(qWarning("int QJniObject::getBooleanField(const char* field_name) %p \"%s\"", reinterpret_cast<void*>(this), field_name));
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();
	jfieldID fid = env->GetFieldID(checkedClass(__FUNCTION__), field_name, "Z");
	if (!fid)
	{
		throw QJniFieldNotFoundException(debugClassName().constData(), field_name, __FUNCTION__);
	}
	jboolean result = env->GetBooleanField(instance_, fid);
	if (jep.clearException())
	{
		throw QJniJavaCallException(debugClassName().constData(), field_name, __FUNCTION__);
	}
	return result;
}


void QJniObject::setIntField(const char * field_name, jint value)
{
	VERBOSE(qWarning("int QJniObject::setIntField(const char* field_name) %p \"%s\"", reinterpret_cast<void*>(this), field_name));
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();
	jfieldID fid = env->GetFieldID(checkedClass(__FUNCTION__), field_name, "I");
	if (!fid)
	{
		throw QJniFieldNotFoundException(debugClassName().constData(), field_name, __FUNCTION__);
	}
	env->SetIntField(instance_, fid, value);
	if (jep.clearException())
	{
		throw QJniJavaCallException(debugClassName().constData(), field_name, __FUNCTION__);
	}
}


void QJniObject::setBooleanField(const char * field_name, jboolean value)
{
	VERBOSE(qWarning("int QJniObject::setBooleanField(const char* field_name) %p \"%s\"", reinterpret_cast<void*>(this) ,field_name));
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();
	jfieldID fid = env->GetFieldID(checkedClass(__FUNCTION__), field_name, "Z");
	if (!fid)
	{
		throw QJniFieldNotFoundException(debugClassName().constData(), field_name, __FUNCTION__);
	}
	env->SetBooleanField(instance_, fid, value);
	if (jep.clearException())
	{
		throw QJniJavaCallException(debugClassName().constData(), field_name, __FUNCTION__);
	}
}


QJniObject * QJniObject::getObjectField(const char* field_name, const char * objname)
{
	VERBOSE(qWarning("int QJniObject::getObjectField(const char * field_name, const char * objname) %p \"%s\" \"%s\"", reinterpret_cast<void*>(this), field_name, objname));
	QByteArray obj;
	appendNormalizedObjectName(obj, objname);

	QJniEnvPtr jep;
	JNIEnv* env = jep.env();
	jfieldID fid = env->GetFieldID(checkedClass(__FUNCTION__), field_name, obj.data());
	if (!fid)
	{
		throw QJniFieldNotFoundException(debugClassName().constData(), field_name, __FUNCTION__);
	}
	jobject jret = env->GetObjectField(instance_, fid);
	if (jep.clearException())
	{
		if (jret)
		{
			env->DeleteLocalRef(jret);
		}
		throw QJniJavaCallException(debugClassName().constData(), field_name, __FUNCTION__);
	}
	if (!jret)
	{
		return nullptr;
	}
	return new QJniObject(jret, true, objname);
}


QString QJniObject::getStringField(const char * field_name)
{
	VERBOSE(qWarning("int QJniObject::getStringField(const char * field_name) %p \"%s\"", reinterpret_cast<void*>(this), field_name));
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();
	jfieldID fid = env->GetFieldID(checkedClass(__FUNCTION__), field_name, "Ljava/lang/String;");
	if (!fid)
	{
		throw QJniFieldNotFoundException(debugClassName().constData(), field_name, __FUNCTION__);
	}
	QString ret = QJniLocalRef(env, env->GetObjectField(instance_, fid));
	if (jep.clearException())
	{
		throw QJniJavaCallException(debugClassName().constData(), field_name, __FUNCTION__);
	}
	return ret;
}


void QJniObject::callParamVoid(const char * method_name, const char * param_signature, ...)
{
	VERBOSE(qWarning("void QJniObject(%p)::callParamVoid(\"%s\", \"%s\", ...)", reinterpret_cast<void*>(this), method_name, param_signature));

	va_list args;
	QJniEnvPtr jep;
	JNIEnv* env = jep.env();

	QByteArray signature("(");
	signature += param_signature;
	signature += ")V";
	jmethodID mid = env->GetMethodID(checkedClass(__FUNCTION__), method_name, signature.data());
	if (!mid)
	{
		throw QJniMethodNotFoundException(debugClassName().constData(), method_name, __FUNCTION__);
	}

	va_start(args, param_signature);
	env->CallVoidMethodV(instance_, mid, args);
	va_end(args);

	if (jep.clearException())
	{
		throw QJniJavaCallException(debugClassName().constData(), method_name, __FUNCTION__);
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
	callParamVoid(
		method_name
		, "Ljava/lang/String;Ljava/lang/String;"
		, QJniLocalRef(jep, string1).jObject()
		, QJniLocalRef(jep, string2).jObject());
}


void QJniObject::callVoid(const char * method_name, const QString & string1, const QString & string2, const QString & string3)
{
	QJniEnvPtr jep;
	callParamVoid(
		method_name
		, "Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;"
		, QJniLocalRef(jep, string1).jObject()
		, QJniLocalRef(jep, string2).jObject()
		, QJniLocalRef(jep, string3).jObject());
}


void QJniObject::callVoid(const char * method_name, const QString & string1, const QString & string2, const QString & string3, const QString & string4)
{
	QJniEnvPtr jep;
	callParamVoid(
		method_name
		, "Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;"
		, QJniLocalRef(jep, string1).jObject()
		, QJniLocalRef(jep, string2).jObject()
		, QJniLocalRef(jep, string3).jObject()
		, QJniLocalRef(jep, string4).jObject());
}


void QJniObject::callVoid(const char * method_name, const QString & string1, const QString & string2, const QString & string3, const QString & string4, const QString & string5)
{
	QJniEnvPtr jep;
	callParamVoid(
		method_name
		, "Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;"
		, QJniLocalRef(jep, string1).jObject()
		, QJniLocalRef(jep, string2).jObject()
		, QJniLocalRef(jep, string3).jObject()
		, QJniLocalRef(jep, string4).jObject()
		, QJniLocalRef(jep, string5).jObject());
}


void QJniObject::callVoid(const char * method_name, const QString & string1, const QString & string2, const QString & string3, const QString & string4, const QString & string5, const QString & string6)
{
	QJniEnvPtr jep;
	callParamVoid(
		method_name
		, "Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;"
		, QJniLocalRef(jep, string1).jObject()
		, QJniLocalRef(jep, string2).jObject()
		, QJniLocalRef(jep, string3).jObject()
		, QJniLocalRef(jep, string4).jObject()
		, QJniLocalRef(jep, string5).jObject()
		, QJniLocalRef(jep, string6).jObject());
}


/////////////////////////////////////////////////////////////////////////////
// QJniLocalRef
/////////////////////////////////////////////////////////////////////////////


QJniLocalRef::QJniLocalRef()
	: local_(nullptr)
	, env_(nullptr)
{
}


QJniLocalRef::QJniLocalRef(const QJniLocalRef & other)
	: env_(other.env_)
{
	if (other.local_)
	{
		if (!env_)
		{
			QJniEnvPtr jep;
			env_ = jep.env();
		}
		if (env_)
		{
			local_ = env_->NewLocalRef(other.local_);
		}
	}
}


QJniLocalRef::QJniLocalRef(QJniLocalRef && other)
{
	local_ = other.local_;
	other.local_ = nullptr;
	env_ = other.env_; // other можно не занулять
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
