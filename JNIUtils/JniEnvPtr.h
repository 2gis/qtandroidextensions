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
#ifdef QT_CORE_LIB
	#include <QString>
#endif

//! Basic functionality to get JNIEnv valid for current thread and scope.
class JniEnvPtr
{	
public:
	JniEnvPtr();
	JniEnvPtr(JNIEnv* env);
	~JniEnvPtr();
	JNIEnv* env();

	// preload class by names 
	// required as Android's JNIEnv->FindClass doesn't work in threads created in native code. 
	// (no workarounds with custom ClassLoader were found {yet?})
public:
	int PreloadClass(const char * class_name);
	// last array element == 0
	int PreloadClasses(const char * const * class_list);

	// Возвращает глобальную ссылку на класс
	jclass FindClass(const char * name);

	// unload all preloaded classes for them not to leak
	void UnloadClasses();

#ifdef QT_CORE_LIB
	// don't forget to DeleteLocalRef it
	jstring JStringFromQString(const QString&);
	QString QStringFromJString(jstring);
#endif
	
public:
	//static JavaVM* JavaVM();
	static void SetJavaVM(JavaVM*);
	static void SetJavaVM(JNIEnv*);
	//static std::string PackageName();

private:
	JNIEnv* env_;
	bool was_already_attached_;
};

