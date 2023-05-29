/*
  QJniHelpers library

  Authors:
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
#include "QJniHelpers.h"

/*!
 * This namespace contains some functions which screen differences between various
 * Android QPA plugin implementations.
 */
namespace QAndroidQPAPluginGap
{

#if !defined(QTANDROIDEXTENSIONS_NO_DEPRECATES)
using QAndroidSpecificJniException = QJniBaseException;
#endif


//! A lightweight object to access QtActivity.
class Activity: public QJniObject
{
public:
	Activity();
};


//! A lightweight object to access Context (either custom context or QtActivity).
class Context: public QJniObject
{
public:
	Context();
};


/*!
 * Return pointer to JavaVM using helpers available in current QPA plug-in.
 */
JavaVM * detectJavaVM();

/*!
 * Obtain QtActivity object.
 * Returns new local reference that must be disposed after use if the function was called from C++.
 * Use Activity object to handle that automatically.
 * If errorIfNone == true, the functions will output to qCritical if failed,
 * no-NoThrow versions will also throw an exception if failed.
 */
jobject JNICALL getActivityEx(JNIEnv * env = nullptr, jobject jo = nullptr, bool errorIfNone = true);
jobject JNICALL getActivity(JNIEnv * env = nullptr, jobject jo = nullptr);
jobject JNICALL getActivityNoThrowEx(JNIEnv * env = nullptr, jobject jo = nullptr, bool errorIfNone = true);
jobject JNICALL getActivityNoThrow(JNIEnv * env = nullptr, jobject jo = nullptr);

/*!
 * Helper to check if we have QtActivity without throwing exceptions & logging errors.
 */
bool isActivityAvailable();

/*!
 * Sets custom Context. It is usually expected to be an Activity or Service.
 * Useful for writing Service-based apps.
 */
void setCustomContext(jobject context);

/*!
 * Returns new local reference that must be disposed after use if the function was called from C++.
 * If custom context is not set the function quietly returns nullptr.
 */
jobject JNICALL getCustomContext(JNIEnv * env = nullptr, jobject jo = nullptr);

bool customContextSet();

/*!
 * Returns custom context if it's set, or QtActivity if it's not set.
 * Use Context object to handle that automatically.
 * Returns new local reference that must be disposed after use if the function was called from C++.
 */
jobject JNICALL getCurrentContext(JNIEnv * env = nullptr, jobject jo = nullptr);
jobject JNICALL getCurrentContextNoThrow(JNIEnv * env = nullptr, jobject jo = nullptr);

/*!
 * This function should be called from main thread to pre-load Java class, so
 * objects of the class could later be instantiated from threads not created in Java.
 */
void preloadJavaClass(const char * class_name);
void preloadJavaClasses(const std::initializer_list<const char *> & list);

//! Preload classes used by QAndroidQPAPluginGap itself.
void preloadJavaClasses();

//! Simple & cached access to android.os.Build.VERSION.SDK_INT
int apiLevel();

} // namespace QAndroidQPAPluginGap

