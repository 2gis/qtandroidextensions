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

// a simple class for convenient access to java classes

#pragma once
#include <jni.h>
#include <QString>

/*!
 * Convenience wrapper for Java objects (and classes)
 * to provide cleaner and more object-oriented access to them.
 */
class jcGeneric
{
public:
	class method_not_found_exception: public std::exception
	{
	public:
		method_not_found_exception(){}
		virtual const char * what() const throw();
	};

	class field_not_found_exception: public std::exception
	{
	public:
		field_not_found_exception(){}
		virtual const char * what() const throw();
	};

	/*!
	 * Create jcGeneric wrapper around specified jobject.
	 * \param  take_ownership means "delete local ref of this object
	 * and only keep the global one here"
	 * Note that this implies the instance is a valid local ref,
	 * not global one or whatever.
	 */
	jcGeneric(jobject instance, bool take_ownership=false);

	/*!
	 * Create a wrapper for a new instance of class 'clazz'.
	 * \param create - set to true to create object instance immediately.
	 */
	jcGeneric(jclass clazz, bool create=true);

	/*!
	 * Same as jcGeneric(jclass clazz, bool create) but
	 * the class is specified by its name.
	 */
	jcGeneric(const char* full_class_name, bool create=true);

	//! Create fully uninitialized jcGeneric.
	jcGeneric();
	virtual ~jcGeneric();

	//! Call void method of the wrapped Java object
	void CallVoid(const char* method_name);

	//! Call boolean method of the wrapped Java object
	bool CallBool(const char* method_name);

	//! Call int method of the wrapped Java object
	int CallInt(const char* method_name);

	//! Call long method of the wrapped Java object
	long long CallLong(const char* method_name);

	//! Call float method of the wrapped Java object
	float CallFloat(const char* method_name);

	//! Call float method of the wrapped Java object with int parameter
	float CallFloat(const char* method_name, int param);

	//! Call double method of the wrapped Java object
	double CallDouble(const char* method_name);

	/*!
	 * Call object method of the wrapped Java object.
	 * \return Pointer to a wrapper for the object returned by the call.
	 * The wrapper should be deleted after use via 'delete'.
	 */
	jcGeneric* CallObject(const char* method_name, const char* objname);

	/*!
	 * Call void method of the wrapped Java object with specified
	 * Java method signature and parameters.
	 */
	void CallParamVoid(const char* method_name, const char* param_signature, ...);

	void CallVoid(const char * method_name, jint x);
	void CallVoid(const char * method_name, jlong x);
	void CallVoid(const char * method_name, const QString & string);
	void CallVoid(const char * method_name, const QString & string1, const QString & string2);

	//! Call void static method of the wrapped Java class
	void CallStaticVoid(const char* method_name);

	/*!
	 * Call object static method of the wrapped Java class.
	 * \param method_name - name of a static method which returns Java object.
	 * \param
	 * \return Pointer to a wrapper for the object returned by the call.
	 * The wrapper should be deleted after use via 'delete'.
	 */
	jcGeneric* CallStaticObject(const char* method_name, const char* objname);

	//! Get value of int field of the wrapped Java object
	int GetInt(const char* field_name);

#ifdef QT_CORE_LIB
	/*!
	 * Call jstring method of the wrapped Java object and
	 * return the result as a QString.
	 */
	QString CallString(const char* method_name);

	/*!
	 * Call static jstring method of the wrapped Java class and
	 * return the result as a QString.
	 */
	QString CallStaticString(const char* method_name);

	/*!
	 * Get value of jstring field of the wrapped Java object and
	 * return the result as a QString.
	 */
	QString GetString(const char* field_name);
#endif

	//! Register native method in the wrapped class
	void RegisterNativeMethod(const char* name, const char* signature, void* ptr);

	//! Register native methods in the wrapped class
	void RegisterNativeMethods(JNINativeMethod* methods_list, size_t sizeof_methods_list);

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
	Q_DISABLE_COPY(jcGeneric)
};
