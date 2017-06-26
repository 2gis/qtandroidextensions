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
namespace QAndroidQPAPluginGap {

	class QAndroidSpecificJniException: public QJniBaseException
	{
	public:
		QAndroidSpecificJniException(const char * message);
	};

	/*!
	 * Return pointer to JavaVM using helpers available in current QPA plug-in.
	 */
	JavaVM * detectJavaVM();

	/*!
	 * Obtain Activity object.
	 * \param env, jo are not used and only needed so the function could be set as native
	 *  method in Java object and called from there over JNI.
	 * \return Returns local JNI reference to the Activity object.
	 */
	jobject JNICALL getActivity(JNIEnv * env = 0, jobject jo = 0);

	jobject JNICALL getActivityNoThrow(JNIEnv * env = 0, jobject jo = 0);

	void setCustomContext(jobject context);

	/*!
	 * Returns custom context if it's set, or null if it's not set.
	 * \param env, jo are not used and only needed so the function could be set as native
	 *  method in Java object and called from there over JNI.*
	 */
	jobject JNICALL getCustomContext(JNIEnv * env = 0, jobject jo = 0);

	bool customContextSet();

	/*!
	 * Returns custom context if it's set, or activity if it's not set.
	 * \param env, jo are not used and only needed so the function could be set as native
	 *  method in Java object and called from there over JNI.*
	 */
	jobject JNICALL getCurrentContext(JNIEnv * env = 0, jobject jo = 0);

	jobject JNICALL getCurrentContextNoThrow(JNIEnv * env = 0, jobject jo = 0);

	class Context: public QJniObject
	{
	public:
		Context();
	private:
		Q_DISABLE_COPY(Context)
	};

	/*!
	 * This function should be called from main thread to pre-load Java class, so
	 * objects of the class could later be instantiated from threads not created in Java.
	 */
	void preloadJavaClass(const char * class_name);

	void preloadJavaClasses();

	int apiLevel();

} // namespace QAndroidQPAPluginGap

