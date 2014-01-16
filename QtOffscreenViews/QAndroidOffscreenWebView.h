/*
  Offscreen Android Views library for Qt

  Author:
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

#include "QAndroidOffscreenView.h"

class QAndroidOffscreenWebView
	: public QAndroidOffscreenView
{
	Q_OBJECT
public:
	QAndroidOffscreenWebView(const QString & object_name, bool waitforcreation, const QSize & def_size, QObject * parent = 0);
	virtual ~QAndroidOffscreenWebView();

	bool loadUrl(const QString & url);
	bool loadData(const QString & text, const QString & mime = QLatin1String("text/html"));

protected:
	// WebViewClient
	virtual void doUpdateVisitedHistory(JNIEnv *, jobject, jobject url, jboolean isReload);
	virtual void onFormResubmission(JNIEnv *, jobject, jobject dontResend, jobject resend);
	virtual void onLoadResource(JNIEnv *, jobject, jobject url);
	virtual void onPageFinished(JNIEnv *, jobject, jobject url);
	virtual void onPageStarted(JNIEnv *, jobject, jobject url, jobject favicon);
	virtual void onReceivedError(JNIEnv *, jobject, int errorCode, jobject description, jobject failingUrl);
	virtual void onReceivedHttpAuthRequest(JNIEnv *, jobject, jobject handler, jobject host, jobject realm);
	virtual void onReceivedLoginRequest(JNIEnv *, jobject, jobject realm, jobject account, jobject args);
	virtual void onReceivedSslError(JNIEnv *, jobject, jobject handler, jobject error);
	virtual void onScaleChanged(JNIEnv *, jobject, float oldScale, float newScale);
	virtual void onTooManyRedirects(JNIEnv *, jobject, jobject cancelMsg, jobject continueMsg);
	virtual void onUnhandledKeyEvent(JNIEnv *, jobject, jobject event);
	virtual jobject shouldInterceptRequest(JNIEnv *, jobject, jobject url);
	virtual jboolean shouldOverrideKeyEvent(JNIEnv *, jobject, jobject event);
	virtual jboolean shouldOverrideUrlLoading(JNIEnv *, jobject, jobject url);

	friend Q_DECL_EXPORT void JNICALL Java_doUpdateVisitedHistory(JNIEnv * env, jobject jo, jlong nativeptr, jobject url, jboolean isReload);
	friend Q_DECL_EXPORT void JNICALL Java_onFormResubmission(JNIEnv * env, jobject jo, jlong nativeptr, jobject dontResend, jobject resend);
	friend Q_DECL_EXPORT void JNICALL Java_onLoadResource(JNIEnv * env, jobject jo, jlong nativeptr, jobject url);
	friend Q_DECL_EXPORT void JNICALL Java_onPageFinished(JNIEnv * env, jobject jo, jlong nativeptr, jobject url);
	friend Q_DECL_EXPORT void JNICALL Java_onPageStarted(JNIEnv * env, jobject jo, jlong nativeptr, jobject url, jobject favicon);
	friend Q_DECL_EXPORT void JNICALL Java_onReceivedError(JNIEnv * env, jobject jo, jlong nativeptr, jint errorCode, jobject description, jobject failingUrl);
	friend Q_DECL_EXPORT void JNICALL Java_onReceivedHttpAuthRequest(JNIEnv * env, jobject jo, jlong nativeptr, jobject handler, jobject host, jobject realm);
	friend Q_DECL_EXPORT void JNICALL Java_onReceivedLoginRequest(JNIEnv * env, jobject jo, jlong nativeptr, jobject realm, jobject account, jobject args);
	friend Q_DECL_EXPORT void JNICALL Java_onReceivedSslError(JNIEnv * env, jobject jo, jlong nativeptr, jobject handler, jobject error);
	friend Q_DECL_EXPORT void JNICALL Java_onScaleChanged(JNIEnv * env, jobject jo, jlong nativeptr, jfloat oldScale, jfloat newScale);
	friend Q_DECL_EXPORT void JNICALL Java_onTooManyRedirects(JNIEnv * env, jobject jo, jlong nativeptr, jobject cancelMsg, jobject continueMsg);
	friend Q_DECL_EXPORT void JNICALL Java_onUnhandledKeyEvent(JNIEnv * env, jobject j, jlong nativeptr, jobject event);
	friend Q_DECL_EXPORT jobject JNICALL Java_shouldInterceptRequest(JNIEnv * env, jobject jo, jlong nativeptr, jobject url);
	friend Q_DECL_EXPORT jboolean JNICALL Java_shouldOverrideKeyEvent(JNIEnv * env, jobject jo, jlong nativeptr, jobject event);
	friend Q_DECL_EXPORT jboolean JNICALL Java_shouldOverrideUrlLoading(JNIEnv * env, jobject jo, jlong nativeptr, jobject url);


};
