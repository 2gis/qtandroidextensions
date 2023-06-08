/*
  QJniHelpers library

  Sergey A. Galin <sergey.galin@gmail.com>
  Based on idea & some code by Ivan Avdeev <marflon@gmail.com>

  Distrbuted under The BSD License

  Copyright (c) 2017-2023, DoubleGIS, LLC.
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

#pragma once
#include <initializer_list>
#include <jni.h>
#include <QtCore/QString>
#include <QtCore/QByteArray>


class QJniBaseException: public std::exception
{
public:
	QJniBaseException(const QByteArray & message);
public: // std::exception
	const char * what() const throw() override;
protected:
	static QByteArray readableIdString(const char * id);
private:
	const QByteArray message_;
};


class QJniThreadAttachException: public QJniBaseException
{
public:
	QJniThreadAttachException(const char * detail);
};


class QJniClassNotFoundException: public QJniBaseException
{
public:
	QJniClassNotFoundException(const char * class_name);
};


class QJniClassNotSetException: public QJniBaseException
{
public:
	QJniClassNotSetException(const char * class_name, const char * call_point_info);
};


class QJniObjectIsNullException: public QJniBaseException
{
public:
	QJniObjectIsNullException(const char * class_name, const char * call_point_info);
};


class QJniMethodNotFoundException: public QJniBaseException
{
public:
	QJniMethodNotFoundException(
		const char * class_name,
		const char * method_name,
		const char * call_point_info);
};


class QJniFieldNotFoundException: public QJniBaseException
{
public:
	QJniFieldNotFoundException(
		const char * class_name,
		const char * field_name,
		const char * call_point_info);
};


class QJniJavaCallException: public QJniBaseException
{
public:
	QJniJavaCallException(
		const char * class_name,
		const char * method_name,
		const char * call_point_info);
};



class QJniObject;


// Basic functionality to get JNIEnv valid for current thread and scope.
// Using this object across threads is UB.
class QJniEnvPtr
{
public:
	// env can be 0, then the constructor gets env for current thread,
	// and attaches current thread to JNI if necessary.
	// QJniEnvPtr either gets a valid JNIEnv pointer or exception is thrown.
	// If attaching the thread has been made, it is automatically detached on finish.
	explicit QJniEnvPtr(JNIEnv * env = 0);

	// Note: copying / moving across threads is UB!
	QJniEnvPtr(const QJniEnvPtr & other);
	QJniEnvPtr(QJniEnvPtr && other);
	QJniEnvPtr & operator=(const QJniEnvPtr & other);
	QJniEnvPtr & operator=(QJniEnvPtr && other);

	// Get current Java environment.
	JNIEnv * env() const;

	// Get current JavaVM. Note: QJniHelpers supports only one JVM per process.
	static JavaVM * getJavaVM();

	// Check if current thread looks properly attached to JNI.
	static bool isCurrentThreadAttached();

	// Preload a class by its name, e.g.: "ru/dublgis/offscreenview/OffscreenWebView".
	// Required as Android's JNIEnv->FindClass doesn't work in threads created in native code.
	// After preloading, any thread can instantiate class via QJniEnvPtr::findClass().
	bool preloadClass(const char * class_name);

	// Preload mutliple classes.
	// class_list must be nullptr-terminated:
	bool preloadClasses(const char * const * class_list);
	bool preloadClasses(const char * const * class_list, size_t count);
	bool preloadClasses(const std::initializer_list<const char *> & class_list);
	bool preloadClasses(const std::vector<const char *> & class_list);

	bool isClassPreloaded(const char * class_name);

	// Get a global reference to a Java class.
	// param name - is a full name of the class, e.g.: "ru/dublgis/offscreenview/OffscreenWebView".
	// May return 0 if FindClass() fails.
	jclass findClass(const char * name);

	// Unload all preloaded classes to free Java objects (usually not necessary to call).
	void unloadAllClasses();

	// These helpers are noexcept and in case of any errors just return empty string or null.
	jstring toJString(const QString & qstring) noexcept;
	QString toQString(jstring javastring) noexcept;

	std::vector<bool> convert(jbooleanArray jarray);
	std::vector<jint> convert(jintArray jarray);
	std::vector<jlong> convert(jlongArray jarray);
	std::vector<jfloat> convert(jfloatArray jarray);
	std::vector<jdouble> convert(jdoubleArray jarray);
	std::vector<QJniObject> convert(jobjectArray jarray);
	QStringList convertToStringList(jobjectArray jarray);

	// Clears Java exception without taking any specific actions.
	// If describe == true it will call ExceptionDescribe() to print the exception
	// description into stderr.
	// Returns false if there was no exception.
	bool clearException(bool describe = true);

#if !defined(QTANDROIDEXTENSIONS_NO_DEPRECATES)
	[[deprecated("Use toJString()")]] jstring JStringFromQString(const QString & qstring)
	{ return toJString(qstring); }
	[[deprecated("Use toJString()")]] jstring QStringToJString(const QString & qstring)
	{ return toJString(qstring); }
	[[deprecated("Use toQString()")]] QString QStringFromJString(jstring javastring)
	{ return toQString(javastring); }
	[[deprecated("Use toQString()")]] QString JStringToQString(jstring javastring)
	{ return toQString(javastring); }
#endif

public:
	static void setJavaVM(JavaVM *);
	static void setJavaVM(JNIEnv *);

private:
	void checkEnv();

private:
	JNIEnv * env_ = nullptr;
};


class QJniObject;


// Convenience wrapper for Java classes to provide cleaner and more object-oriented access to them.
 class QJniClass
{
public:
	// Empty (null) class
	QJniClass();

	// Create a wrapper for class 'clazz'.
	explicit QJniClass(jclass clazz);

	// Create an instance of the specified class.
	explicit QJniClass(const char * full_class_name);

	// Create QJniClass for the object.
	explicit QJniClass(jobject object);

	QJniClass(const QJniClass & other);
	QJniClass(QJniClass && other);
	QJniClass & operator=(const QJniClass & other);
	QJniClass & operator=(QJniClass && other);

	virtual ~QJniClass() noexcept;

	static bool classAvailable(const char * full_class_name);

	void callStaticVoid(const char * method_name);
	jint callStaticInt(const char * method_name);
	jlong callStaticLong(const char * method_name);
	bool callStaticBoolean(const char * method_name);
	void callStaticParamVoid(const char * method_name, const char * param_signature, ...);
	bool callStaticParamBoolean(const char * method_name, const char * param_signature, ...);
	jint callStaticParamInt(const char * method_name, const char * param_signature, ...);
	jlong callStaticParamLong(const char * method_name, const char * param_signature, ...);
	jfloat callStaticParamFloat(const char * method_name, const char * param_signature, ...);
	QString callStaticParamString(const char * method_name, const char * param_signature, ...);
	void callStaticVoid(const char * method_name, const QString & string);
	QJniObject callStaticObj(const char * method_name, const char * objname);
	QJniObject callStaticParamObj(const char * method_name, const char * objname, const char * param_signature, ...);
	QString callStaticString(const char * method_name);

	QJniObject getStaticObjField(const char * field_name, const char * objname) const;
	QString getStaticStringField(const char * field_name) const;
	jint getStaticIntField(const char * field_name) const;
	bool getStaticBooleanField(const char * field_name) const;
	jfloat getStaticFloatField(const char * field_name) const;
	jfloat getStaticDoubleField(const char * field_name) const;

	std::vector<bool> getStaticBooleanArrayField(const char * name) const;
	std::vector<jint> getStaticIntArrayField(const char * name) const;
	std::vector<jlong> getStaticLongArrayField(const char * name) const;
	std::vector<jfloat> getStaticFloatArrayField(const char * name) const;
	std::vector<jdouble> getStaticDoubleArrayField(const char * name) const;
	std::vector<QJniObject> getStaticObjectArrayField(const char * name, const char * objname) const;
	QStringList getStaticStringArrayField(const char * name) const;

	// Register native method in the wrapped class
	bool registerNativeMethod(const char * name, const char * signature, void * ptr);
	bool registerNativeMethodsN(const JNINativeMethod * methods_list, size_t count);
	bool registerNativeMethods(const std::initializer_list<JNINativeMethod> & list);
	bool registerNativeMethods(const std::vector<JNINativeMethod> & list);

	// Unregister native methods in the wrapped class.
	bool unregisterNativeMethods();

	virtual bool isNull() const { return !class_; }
	operator bool() const { return !isNull(); }

	// Get JNI reference to the wrapped Java class.
	jclass jClass() const { return class_; }

	// Retrieve class name (via JNI). If 'simple' is true then only the class name is returned
	// (e.g.: "String"), if it's false then full name with class path (e.g.: "java/lang/String").
	QString getClassName(bool simple = false) const;

	const QByteArray & constructionClassName() const { return construction_class_name_; }

	QByteArray debugClassName() const;

#if !defined(QTANDROIDEXTENSIONS_NO_DEPRECATES)
	[[nodiscard, deprecated("Use callStaticObj()")]] QJniObject * callStaticObject(
		const char * method_name,
		const char * objname);
	[[nodiscard, deprecated("Use callStaticParamObj()")]] QJniObject * callStaticParamObject(
		const char * method_name,
		const char * objname,
		const char * param_signature, ...);
	[[nodiscard, deprecated("Use getStaticObjField()")]] QJniObject * getStaticObjectField(
		const char * field_name,
		const char * objname) const;
	// sizeof_methods_list is the size of the whole array pointed by methods_list,
	// not count of the methods to register!
	[[deprecated("Use registerNativeMethodsN() or other version of registerNativeMethods()")]]
	bool registerNativeMethods(const JNINativeMethod * methods_list, size_t sizeof_methods_list);
#endif

protected:
	void initClass(JNIEnv * env, jclass clazz);
	void clearClass(JNIEnv * env);
	inline jclass checkedClass(const char * call_point_info) const;

private:
	jclass class_ = 0;
	QByteArray construction_class_name_;
};


/*!
 * Convenience wrapper for Java objects to provide cleaner and more object-oriented access to them.
 */
class QJniObject: public QJniClass
{
public:
	// Creates null object
	QJniObject();

	// Create QJniObject wrapper around specified jobject.
	// take_ownership_over_local_ref == true means that 'instance' is a local ref and it will be
	// deleted after creating QJniObject(). It it's false the 'instance' can be either global
	// or local ref and will stay valid after creation of the QJniObject.
	// known_class_name should be set to nullptr or class of name of instance,
	// (It is used to check if the object must have a non-zero class reference.
	// For example, Java arrays don't have a class instance.)
	QJniObject(
		jobject instance,
		bool take_ownership_over_local_ref,
		const char * known_class_name = nullptr,
		bool known_can_have_null_class = false);

	// Create an object of the given class with a parametrized constructor.
	QJniObject(const QJniClass & clazz, const char * param_signature = 0, ...);

	// Create an object of the given class with a parametrized constructor.
	QJniObject(const char * class_name, const char * param_signature = 0, ...);

	QJniObject(const QJniObject & other);
	QJniObject(QJniObject && other);
	QJniObject & operator=(const QJniObject & other);
	QJniObject & operator=(QJniObject && other);

	static QJniObject fromString(const QString & str);
	QString toQString() const;
	std::vector<bool> toBooleanArray() const;
	std::vector<jint> toIntArray() const;
	std::vector<jlong> toLongArray() const;
	std::vector<jfloat> toFloatArray() const;
	std::vector<jdouble> toDoubleArray() const;
	std::vector<QJniObject> toObjectArray() const;
	QStringList toStringList() const;

	void dispose();
	~QJniObject() noexcept override;

	// Detach the QJniObject from jobject and return the jobject casted to the
	// template-specified type.
	template<class RESULT_TYPE> RESULT_TYPE detach()
	{
		RESULT_TYPE saved = static_cast<RESULT_TYPE>(instance_);
		instance_ = 0;
		return saved;
	}

	void callVoid(const char * method_name);
	bool callBool(const char * method_name);
	bool callBool(const char * method_name, bool param);
	jint callInt(const char * method_name);
	jlong callLong(const char * method_name);
	jfloat callFloat(const char * method_name);
	jfloat callFloat(const char * method_name, jint param);
	jdouble callDouble(const char * method_name);
	QJniObject callObj(const char * method_name, const char * objname);
	QJniObject callParamObj(const char * method_name, const char * objname, const char * param_signature, ...);
	void callParamVoid(const char * method_name, const char * param_signature, ...);
	void callVoid(const char * method_name, jint x);
	void callVoid(const char * method_name, jlong x);
	void callVoid(const char * method_name, jlong x1, jlong x2);
	void callVoid(const char * method_name, jboolean x);
	void callVoid(const char * method_name, jfloat x);
	void callVoid(const char * method_name, jdouble x);
	void callVoid(const char * method_name, const QString & string);
	void callVoid(const char * method_name, const QString & string1, const QString & string2);
	void callVoid(
		const char * method_name,
		const QString & string1,
		const QString & string2,
		const QString & string3);
	void callVoid(
		const char * method_name,
		const QString & string1,
		const QString & string2,
		const QString & string3,
		const QString & string4);
	void callVoid(
		const char * method_name,
		const QString & string1,
		const QString & string2,
		const QString & string3,
		const QString & string4,
		const QString & string5);
	void callVoid(
		const char * method_name,
		const QString & string1,
		const QString & string2,
		const QString & string3,
		const QString & string4,
		const QString & string5,
		const QString & string6);
	jint callParamInt(const char * method_name, const char * param_signature, ...);
	jlong callParamLong(const char * method_name, const char * param_signature, ...);
	jfloat callParamFloat(const char * method_name, const char * param_signature, ...);
	jdouble callParamDouble(const char * method_name, const char * param_signature, ...);
	jboolean callParamBoolean(const char * method_name, const char * param_signature, ...);
	QString callString(const char * method_name);
	QString callParamString(const char *method_name, const char * param_signature, ...);

	jint getIntField(const char * field_name) const;
	jlong getLongField(const char * field_name) const;
	jfloat getFloatField(const char * field_name) const;
	jdouble getDoubleField(const char * field_name) const;
	jboolean getBooleanField(const char * field_name) const;
	QJniObject getObjField(const char * field_name, const char * objname) const;
	QString getStringField(const char * field_name) const;

	std::vector<bool> getBooleanArrayField(const char * name) const;
	std::vector<jint> getIntArrayField(const char * name) const;
	std::vector<jlong> getLongArrayField(const char * name) const;
	std::vector<jfloat> getFloatArrayField(const char * name) const;
	std::vector<jdouble> getDoubleArrayField(const char * name) const;
	std::vector<QJniObject> getObjectArrayField(const char * name, const char * objname) const;
	QStringList getStringArrayField(const char * name) const;

	void setIntField(const char * field_name, jint value);
	void setBooleanField(const char * field_name, jboolean value);

	jobject jObject() const { return instance_; }

	// No need to check for class_: sometimes it is valid to have null class;
	// when it's not valid, null class will cause instance_ to be also null.
	bool isNull() const override { return !instance_; }

#if !defined(QTANDROIDEXTENSIONS_NO_DEPRECATES)
	[[deprecated("Use detach<jobject>()")]] jobject takeJobjectOver()
	{ return detach<jobject>(); }
	[[nodiscard, deprecated("Use callObj()")]] QJniObject * callObject(
		const char * method_name,
		const char * objname);
	[[nodiscard, deprecated("Use callParamObj()")]] QJniObject * callParamObject(
		const char * method_name,
		const char * objname,
		const char * param_signature, ...);
	[[nodiscard, deprecated("Use getObjField()")]] QJniObject * getObjectField(
		const char * field_name,
		const char * objname) const;
	[[deprecated("Use getStringField()")]] QString getString(const char * field_name)
	{ return getStringField(field_name); }
#endif

protected:
	void initObject(JNIEnv* env, jobject instance, bool can_have_null_class = false);
	inline jobject checkedInstance(const char * call_point_info) const;

protected:
	jobject instance_ = 0;
};


// A helper class that keeps and automatically deletes local JNI references.
// It is tad more effective than QJniObject.
// Most popular use is converting C strings for JNI to pass a JNI call parameter:
// QJniLocalRef("SomeString").jstring()
 class QJniLocalRef
{
public:
	// Create without JNIEnv pointer. The object will use QJniEnvPtr to get it.
	// The object takes ownership over the existing local reference.
	explicit QJniLocalRef(jobject local): local_(local), env_(nullptr) {}

	// The object takes ownership over the existing local reference.
	explicit QJniLocalRef(JNIEnv * env, jobject local): local_(local), env_(env) {}

	// The object takes ownership over the existing local reference.
	explicit QJniLocalRef(QJniEnvPtr & env, jobject local): local_(local), env_(env.env()) {}

	// Converts QString to jstring and keeps the reference.
	explicit QJniLocalRef(const QString & string);

	// Converts QString to jstring and keeps the reference.
	explicit QJniLocalRef(QJniEnvPtr & env, const QString & string);

	QJniLocalRef();
	QJniLocalRef(const QJniLocalRef & other);
	QJniLocalRef(QJniLocalRef && other);
	QJniLocalRef & operator=(const QJniLocalRef & other);
	QJniLocalRef & operator=(QJniLocalRef && other);

	void dispose();
	~QJniLocalRef() noexcept;

	// Detach the QJniLocalRef from jobject and return the jobject casted to the
	// template-specified type.
	// Use to return an object from a JNICALL function, e.g.:
	// JNICALL jstring .... foo() { return QJniLocalRef("Hello World!").detach<jstring>(); }
	template<class RESULT_TYPE> RESULT_TYPE detach()
	{
		RESULT_TYPE saved = static_cast<RESULT_TYPE>(local_);
		local_ = 0;
		return saved;
	}

	operator jobject() const { return local_; }
	operator jstring() const { return static_cast<jstring>(local_); }
	operator jclass() const { return static_cast<jclass>(local_); }
	jobject jObject() const { return local_; }
	operator QString() const { return QJniEnvPtr(env_).toQString(operator jstring()); }

private:
	jobject local_ = 0;
	JNIEnv * env_ = nullptr;
};

