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

#pragma once
#include <string>
#include <jni.h>
#include <QString>

//! Basic functionality to get JNIEnv valid for current thread and scope.
class JniEnvPtr
{	
public:
	JniEnvPtr();
	JniEnvPtr(JNIEnv* env);
	~JniEnvPtr();

	//! \brief Get current Java environment.
	JNIEnv * env() const;

	//! \brief Get current JavaVM. Note: JNIUtils supports only one JVM per process.
	JavaVM * getJavaVM() const;

	/*!
	 * \brief Preload class by name, e.g.: "ru/dublgis/offscreenview/OffscreenWebView".
	 * Required as Android's JNIEnv->FindClass doesn't work in threads created in native code.
	 * After preloading, any thread can instantiate classes via JniEnvPtr::FindClass().
	 */
	bool PreloadClass(const char * class_name);

	/*!
	 * \brief Preload mutliple classes.
	 * \param class_list - 0-terminated array of pointers to class names.
	 * \return Number of loaded classes.
	 */
	int PreloadClasses(const char * const * class_list);

	/*!
	 * \brief Check if the class has been pre-loaded (see PreloadClass()).
	 * \param class_name
	 * \return
	 */
	bool IsClassPreloaded(const char * class_name);

	/*!
	 * \brief Get a global reference to a Java class.
	 * \param name - full name of the class, e.g.: "ru/dublgis/offscreenview/OffscreenWebView".
	 * \return
	 */
	jclass FindClass(const char * name);

	/*!
	 * \brief Unload all preloaded classes to free Java objects.
	 */
	void UnloadClasses();

	/*!
	 * \brief Convert QString into jstring.
	 * \return Java String reference. Don't forget to call DeleteLocalRef on the returned reference!
	 */
	jstring JStringFromQString(const QString & qstring);

	/*!
	 * \brief Convert jstring to QString.
	 * \param javastring - Java reference to String object.
	 * \return QString.
	 */
	QString QStringFromJString(jstring javastring);

	/*!
	 * \brief Clear Java exception without taking any specific actions.
	 * \param describe - if true, will call ExceptionDescribe() to print the exception description into stderr.
	 * \return Returns false if there was no exceptions.
	 */
	bool SuppressException(bool describe = true);
	
public:
	static void SetJavaVM(JavaVM*);
	static void SetJavaVM(JNIEnv*);
	//static std::string PackageName();

private:
	JNIEnv * env_;
};

