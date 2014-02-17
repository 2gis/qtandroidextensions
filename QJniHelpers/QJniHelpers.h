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

#pragma once
#include <jni.h>
#include <QString>
#include <QDebug>
#include <QMutex>
#include <QMutexLocker>
#include <QThreadStorage>

class QJniBaseException: public std::exception
{
public:
	QJniBaseException(){}
	virtual const char * what() const throw();
};

class QJniThreadAttachException: public std::exception
{
public:
	QJniThreadAttachException(){}
	virtual const char * what() const throw();
};

class QJniClassNotFoundException: public QJniBaseException
{
public:
	QJniClassNotFoundException(){}
	virtual const char * what() const throw();
};

class QJniMethodNotFoundException: public QJniBaseException
{
public:
	QJniMethodNotFoundException(){}
	virtual const char * what() const throw();
};

class QJniFieldNotFoundException: public QJniBaseException
{
public:
	QJniFieldNotFoundException(){}
	virtual const char * what() const throw();
};

class QJniJavaCallException: public QJniBaseException
{
public:
	QJniJavaCallException(){}
	virtual const char * what() const throw();
};




//! Basic functionality to get JNIEnv valid for current thread and scope.
class QJniEnvPtr
{	
public:
	QJniEnvPtr(JNIEnv * env = 0);
	~QJniEnvPtr();

	//! \brief Get current Java environment.
	JNIEnv * env() const;

	//! \brief Get current JavaVM. Note: QJniHelpers supports only one JVM per process.
	JavaVM * getJavaVM() const;

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
	 * \return
	 */
	jclass findClass(const char * name);

	/*!
	 * \brief Unload all preloaded classes to free Java objects.
	 */
	void unloadAllClasses();

	/*!
	 * \brief Convert QString into jstring.
	 * \return Java String reference. Don't forget to call DeleteLocalRef on the returned reference!
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

/*!
 * Convenience wrapper for Java objects (and classes)
 * to provide cleaner and more object-oriented access to them.
 */
class QJniObject
{
public:
	/*!
	 * Create QJniObject wrapper around specified jobject.
	 * \param take_ownership means "delete local ref of this object
	 * and only keep the global one here"
	 * Note that this implies the instance is a valid local ref,
	 * not global one or whatever.
	 */
	QJniObject(jobject instance, bool take_ownership = false);

	/*!
	 * Create a wrapper for a new instance of class 'clazz'.
	 * \param create - set to true to create object instance immediately.
	 */
	QJniObject(jclass clazz, bool create=true);

	/*!
	 * Same as QJniObject(jclass clazz, bool create) but
	 * the class is specified by its name.
	 */
	QJniObject(const char * full_class_name, bool create=true);

	//! Create fully uninitialized QJniObject.
	QJniObject();

	virtual ~QJniObject();

	/*!
	 * Returns jobject and zeroes it.
	 */
	jobject takeJobjectOver();

	//! Call void method of the wrapped Java object
	void callVoid(const char* method_name);

	//! Call boolean method of the wrapped Java object
	bool callBool(const char* method_name);

	bool callBool(const char* method_name, bool param);

	//! Call int method of the wrapped Java object
	int callInt(const char* method_name);

	//! Call long method of the wrapped Java object
	long long callLong(const char* method_name);

	//! Call float method of the wrapped Java object
	float callFloat(const char* method_name);

	//! Call float method of the wrapped Java object with int parameter
	float callFloat(const char* method_name, int param);

	//! Call double method of the wrapped Java object
	double callDouble(const char* method_name);

	/*!
	 * Call object method of the wrapped Java object.
	 * \return Pointer to a wrapper for the object returned by the call.
	 * The wrapper should be deleted after use via 'delete'.
	 */
	QJniObject* callObject(const char* method_name, const char* objname);

	/*!
	 * Call void method of the wrapped Java object with specified
	 * Java method signature and parameters.
	 */
	void callParamVoid(const char* method_name, const char* param_signature, ...);
	void callVoid(const char * method_name, jint x);
	void callVoid(const char * method_name, jlong x);
	void callVoid(const char * method_name, jboolean x);
	void callVoid(const char * method_name, jfloat x);
	void callVoid(const char * method_name, jdouble x);
	void callVoid(const char * method_name, const QString & string);
	void callVoid(const char * method_name, const QString & string1, const QString & string2);
	void callVoid(const char * method_name, const QString & string1, const QString & string2, const QString & string3);
	void callVoid(const char * method_name, const QString & string1, const QString & string2, const QString & string3, const QString & string4);
	void callVoid(const char * method_name, const QString & string1, const QString & string2, const QString & string3, const QString & string4, const QString & string5);

	//! Call void static method of the wrapped Java class
	void callStaticVoid(const char * method_name);
	jint callStaticInt(const char * method_name);
	jlong callStaticLong(const char * method_name);
	bool callStaticBoolean(const char * method_name);
	void callStaticParamVoid(const char * method_name, const char * param_signature, ...);
	void callStaticVoid(const char * method_name, const QString & string);

	/*!
	 * Call object static method of the wrapped Java class.
	 * \param method_name - name of a static method which returns Java object.
	 * \param
	 * \return Pointer to a wrapper for the object returned by the call.
	 * The wrapper should be deleted after use via 'delete'.
	 */
	QJniObject * callStaticObject(const char* method_name, const char* objname);
	QJniObject * callStaticParamObject(const char * method_name, const char * objname, const char * param_signature, ...);

	//! Get value of int field of the wrapped Java object
	int getIntField(const char* field_name);

	/*!
	 * Call jstring method of the wrapped Java object and
	 * return the result as a QString.
	 */
	QString callString(const char* method_name);

	/*!
	 * Call static jstring method of the wrapped Java class and
	 * return the result as a QString.
	 */
	QString callStaticString(const char* method_name);

	/*!
	 * Get value of jstring field of the wrapped Java object and
	 * return the result as a QString.
	 */
	QString getString(const char* field_name);

	//! Register native method in the wrapped class
	bool registerNativeMethod(const char* name, const char* signature, void* ptr);

	/*!
	 * Register native methods in the wrapped class.
	 * \param sizeof_methods_list is the size of the whole array pointed by methods_list,
	 * not count of the methods to register!
	 */
	bool registerNativeMethods(const JNINativeMethod * methods_list, size_t sizeof_methods_list);

	//! Get JNI reference to the wrapped Java object
	jobject jObject() { return instance_; }

	//! Get JNI reference to the wrapped Java class
	jclass jClass() { return class_; }

protected:
	void init(JNIEnv* env, jobject instance);
	void init(JNIEnv* env, jclass class_to_instantiate, bool create);
	void init(JNIEnv* env, const char* full_class_name, bool create);

protected:
	jobject instance_;
	jclass class_;
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

	operator jobject() { return local_; }
	operator jstring() { return (jstring)local_; }
	operator jclass() { return (jclass)local_; }
	jobject jObject() { return local_; }
	operator QString() { return QJniEnvPtr(env_).JStringToQString(jstring()); }

private:
	jobject local_;
	JNIEnv * env_;
	Q_DISABLE_COPY(QJniLocalRef)
};

/*!
 * Create an instance of this class in main() to have Java class references deleted
 * as main() exits.
 */
class QJniClassUnloader
{
public:
	~QJniClassUnloader()
	{
		QJniEnvPtr().unloadAllClasses();
	}
};

