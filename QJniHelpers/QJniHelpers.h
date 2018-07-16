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

#pragma once
#include <jni.h>
#include <QtCore/QString>
#include <QtCore/QByteArray>
#include <QtCore/QDebug>
#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>
#include <QtCore/QThreadStorage>


class QJniBaseException: public std::exception
{
public:
	QJniBaseException(const QByteArray & message);
	virtual const char * what() const throw();
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


class QJniMethodNotFoundException: public QJniBaseException
{
public:
	QJniMethodNotFoundException(const char * class_name, const char * method_name, const char * call_point_info);
};


class QJniFieldNotFoundException: public QJniBaseException
{
public:
	QJniFieldNotFoundException(const char * class_name, const char * field_name, const char * call_point_info);
};


class QJniJavaCallException: public QJniBaseException
{
public:
	QJniJavaCallException(const char * class_name, const char * method_name, const char * call_point_info);
};




//! Basic functionality to get JNIEnv valid for current thread and scope.
class QJniEnvPtr
{
public:
	/*!
	 * \param env can be 0, then the constructor gets env for current thread,
	 *  and attaches current thread to JNI if necessary.
	 *  QJniEnvPtr always contains a valid JNIEnv pointer or exception is thrown.
	 *  If attaching the thread has been made, it is correctly detached when
	 *  it exits.
	 * \throw Throws QJniThreadAttachException if attaching to thread failed.
	 */
	QJniEnvPtr(JNIEnv * env = 0);
	~QJniEnvPtr();

	//! \brief Get current Java environment.
	JNIEnv * env() const;

	//! \brief Get current JavaVM. Note: QJniHelpers supports only one JVM per process.
	static JavaVM * getJavaVM();

	//! \brief Check if current thread looks properly attached to JNI.
	static bool isCurrentThreadAttached();

	/*!
	 * \brief Preload class by name, e.g.: "ru/dublgis/offscreenview/OffscreenWebView".
	 * Required as Android's JNIEnv->FindClass doesn't work in threads created in native code.
	 * After preloading, any thread can instantiate classes via QJniEnvPtr::findClass().
	 */
	bool preloadClass(const char * class_name);

	/*!
	 * \brief Preload mutliple classes.
	 * \param class_list - 0-terminated array of pointers to class names.
	 * \return Number of loaded classes.
	 */
	int preloadClasses(const char * const * class_list);

	/*!
	 * \brief Check if the class has been pre-loaded (see PreloadClass()).
	 * \param class_name
	 * \return
	 */
	bool isClassPreloaded(const char * class_name);

	/*!
	 * \brief Get a global reference to a Java class.
	 * \param name - full name of the class, e.g.: "ru/dublgis/offscreenview/OffscreenWebView".
	 * \return jclass or 0 if class not found.
	 */
	jclass findClass(const char * name);

	/*!
	 * \brief Unload all preloaded classes to free Java objects.
	 */
	void unloadAllClasses();

	/*!
	 * \brief Convert QString into jstring.
	 * \return Java String reference or 0. Don't forget to call DeleteLocalRef on the returned reference.
	 */
	jstring JStringFromQString(const QString & qstring);
	jstring QStringToJString(const QString & qstring) { return JStringFromQString(qstring); }

	/*!
	 * \brief Convert jstring to QString.
	 * \param javastring - Java reference to String object.
	 * \return QString.
	 */
	QString QStringFromJString(jstring javastring);
	QString JStringToQString(jstring javastring) { return QStringFromJString(javastring); }

	/*!
	 * \brief Clear Java exception without taking any specific actions.
	 * \param describe - if true, will call ExceptionDescribe() to print the exception description into stderr.
	 * \return Returns false if there was no exceptions.
	 */
	bool clearException(bool describe = true);

public:
	static void setJavaVM(JavaVM*);
	static void setJavaVM(JNIEnv*);

private:
	JNIEnv * env_;
	Q_DISABLE_COPY(QJniEnvPtr)
};


class QJniObject;


/*!
 * Convenience wrapper for Java classes to provide cleaner and more object-oriented access to them.
 */
class QJniClass
{
public:
	/*!
	 * Create a wrapper for class 'clazz'.
	 */
	QJniClass(jclass clazz);

	/*!
	 * Create a wrapper for class 'full_class_name'.
	 */
	QJniClass(const char * full_class_name);

	//! Create QJniClass for the jobject.
	QJniClass(jobject object);

	QJniClass(const QJniClass & other);

	virtual ~QJniClass();

	//! Call void static method of the wrapped Java class
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

	/*!
	 * Call object static method of the wrapped Java class.
	 * \param method_name - name of a static method which returns Java object.
	 * \param
	 * \return Pointer to a wrapper for the object returned by the call.
	 * The wrapper should be deleted after use via 'delete'.
	 */
	QJniObject * callStaticObject(const char * method_name, const char * objname);
	QJniObject * callStaticParamObject(const char * method_name, const char * objname, const char * param_signature, ...);

	/*!
	 * Call static jstring method of the wrapped Java class and
	 * return the result as a QString.
	 */
	QString callStaticString(const char * method_name);

	QJniObject * getStaticObjectField(const char * field_name, const char * objname);
	QString getStaticStringField(const char * field_name);
	jint getStaticIntField(const char * field_name);
	bool getStaticBooleanField(const char * field_name);

	//! Register native method in the wrapped class
	bool registerNativeMethod(const char * name, const char * signature, void * ptr);

	/*!
	 * Register native methods in the wrapped class.
	 * \param sizeof_methods_list is the size of the whole array pointed by methods_list,
	 * not count of the methods to register!
	 */
	bool registerNativeMethods(const JNINativeMethod * methods_list, size_t sizeof_methods_list);

	/*!
	 * Unregister native methods in the wrapped class.
	 */
	bool unregisterNativeMethods();

	QJniClass & operator=(const QJniClass &other);

	operator bool() const { return class_ != 0; }

	//! Get JNI reference to the wrapped Java class.
	jclass jClass() const { return class_; }

	/*!
	 * Retrieve class name (via JNI). If \a simple is true then only the class name is returned
	 * (e.g.: "String"), if it's false then full name with class path (e.g.: "java/lang/String").
	 */
	QString getClassName(bool simple = false) const;

	const QByteArray & constructionClassName() const { return construction_class_name_; }

	QByteArray debugClassName() const;

protected:
	void initClass(JNIEnv * env, jclass clazz);
	void clearClass(JNIEnv * env);

	inline jclass checkedClass(const char * call_point_info)
	{
		if (!class_)
		{
			throw QJniClassNotSetException(construction_class_name_.constData(), call_point_info);
		}
		return class_;
	}

private:
	jclass class_;
	QByteArray construction_class_name_;
};


/*!
 * Convenience wrapper for Java objects to provide cleaner and more object-oriented access to them.
 */
class QJniObject: public QJniClass
{
public:
	/*!
	 * Create QJniObject wrapper around specified jobject.
	 * \param take_ownership means "delete local ref of this object
	 * and only keep the global one here"
	 * Note that this implies the instance is a valid local ref,
	 * not global one or whatever.
	 * \param known_class_name shoule be set to 0 or class of name of instance,
	 * It is used to check if the object must have a non-zero class reference.
	 * For example, Java arrays don't have a class instance.
	 */
	QJniObject(jobject instance, bool take_ownership, const char * known_class_name = 0);

	/*!
	 * Create a wrapper for a new instance of class 'clazz'.
	 * \param param_signature - signature for parameter of constructor.
	 */
	QJniObject(const QJniClass & clazz, const char * param_signature = 0, ...);

	/*!
	 * Create a wrapper for a new instance of class 'class name'.
	 * \param param_signature - signature for parameter of constructor.
	 */
	QJniObject(const char * class_name, const char * param_signature = 0, ...);

	virtual ~QJniObject();

	/*!
	 * Returns jobject and zeroes it.
	 */
	jobject takeJobjectOver();

	//! Call void method of the wrapped Java object
	void callVoid(const char * method_name);

	//! Call boolean method of the wrapped Java object
	bool callBool(const char * method_name);

	bool callBool(const char * method_name, bool param);

	//! Call int method of the wrapped Java object
	int callInt(const char * method_name);

	//! Call long method of the wrapped Java object
	long long callLong(const char * method_name);

	//! Call float method of the wrapped Java object
	float callFloat(const char * method_name);

	//! Call float method of the wrapped Java object with int parameter
	float callFloat(const char * method_name, int param);

	//! Call double method of the wrapped Java object
	double callDouble(const char * method_name);

	/*!
	 * Call object method of the wrapped Java object.
	 * \return Pointer to a wrapper for the object returned by the call.
	 * The wrapper should be deleted after use via 'delete'.
	 */
	QJniObject * callObject(const char * method_name, const char * objname);
	QJniObject * callParamObject(const char * method_name, const char * objname, const char * param_signature, ...);

	/*!
	 * Call void method of the wrapped Java object with specified
	 * Java method signature and parameters.
	 */
	void callParamVoid(const char * method_name, const char * param_signature, ...);
	void callVoid(const char * method_name, jint x);
	void callVoid(const char * method_name, jlong x);
	void callVoid(const char * method_name, jlong x1, jlong x2);
	void callVoid(const char * method_name, jboolean x);
	void callVoid(const char * method_name, jfloat x);
	void callVoid(const char * method_name, jdouble x);
	void callVoid(const char * method_name, const QString & string);
	void callVoid(const char * method_name, const QString & string1, const QString & string2);
	void callVoid(const char * method_name, const QString & string1, const QString & string2, const QString & string3);
	void callVoid(const char * method_name, const QString & string1, const QString & string2, const QString & string3, const QString & string4);
	void callVoid(const char * method_name, const QString & string1, const QString & string2, const QString & string3, const QString & string4, const QString & string5);
	void callVoid(const char * method_name, const QString & string1, const QString & string2, const QString & string3, const QString & string4, const QString & string5, const QString & string6);

	jint callParamInt(const char * method_name, const char * param_signature, ...);
	jlong callParamLong(const char * method_name, const char * param_signature, ...);
	jfloat callParamFloat(const char * method_name, const char * param_signature, ...);
	jdouble callParamDouble(const char * method_name, const char * param_signature, ...);
	jboolean callParamBoolean(const char * method_name, const char * param_signature, ...);

	//! Get value of int field of the wrapped Java object
	int getIntField(const char * field_name);

	//! Get value of long field of the wrapped Java object
	jlong getLongField(const char * field_name);

	//! Get value of float field of the wrapped Java object
	float getFloatField(const char * field_name);

	jboolean getBooleanField(const char * field_name);

	void setIntField(const char * field_name, jint value);
	void setBooleanField(const char * field_name, jboolean value);

	//! Get value of float field of the wrapped Java object
	QJniObject * getObjectField(const char * field_name, const char * objname);

	QString getStringField(const char * field_name);

	/*!
	 * Call jstring method of the wrapped Java object and
	 * return the result as a QString.
	 */
	QString callString(const char* method_name);

	QString callParamString(const char *method_name, const char* param_signature, ...);

	/*!
	 * Get value of jstring field of the wrapped Java object and
	 * return the result as a QString.
	 */
	QString getString(const char* field_name);

	operator bool() const { return instance_ != 0; }

	//! Get JNI reference to the wrapped Java object
	jobject jObject() const { return instance_; }

protected:
	void initObject(JNIEnv* env, jobject instance, bool can_have_null_class = false);

protected:
	jobject instance_;

private:
	Q_DISABLE_COPY(QJniObject)
};


/*!
 * A helper class which keeps and automatically deletes local JNI references.
 * Global references are handled by QJniObject or inside of QJniEnvPtr, but we also
 * need a cleaner way to handle local ones.
 * Also this class can handle QString <=> jstring conversion in a shorter manner
 * than QJniEnvPtr.
 */
class QJniLocalRef
{
public:
	/*!
	 * Create without JNIEnv pointer. The object will use QJniEnvPtr to find it.
	 * The object takes ownership over the existing local reference.
	 */
	explicit QJniLocalRef(jobject local): local_(local), env_(0) {}

	//! The object takes ownership over the existing local reference.
	explicit QJniLocalRef(JNIEnv * env, jobject local): local_(local), env_(env) {}

	//! The object takes ownership over the existing local reference.
	explicit QJniLocalRef(QJniEnvPtr & env, jobject local): local_(local), env_(env.env()) {}

	//! Converts QString to jstring and keeps the reference.
	explicit QJniLocalRef(const QString & string);

	//! Converts QString to jstring and keeps the reference.
	explicit QJniLocalRef(QJniEnvPtr & env, const QString & string);

	~QJniLocalRef();

	operator jobject() const { return local_; }
	operator jstring() const { return static_cast<jstring>(local_); }
	operator jclass() const { return static_cast<jclass>(local_); }
	jobject jObject() const { return local_; }
	operator QString() const { return QJniEnvPtr(env_).JStringToQString(operator jstring()); }

private:
	jobject local_;
	JNIEnv * env_;
	Q_DISABLE_COPY(QJniLocalRef)
};

