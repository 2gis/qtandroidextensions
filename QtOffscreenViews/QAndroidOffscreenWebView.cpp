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

#include <QAndroidQPAPluginGap.h>
#include "QAndroidOffscreenWebView.h"

static inline QAndroidOffscreenWebView * AOWW(jlong nativeptr)
{
	if (nativeptr)
	{
		QAndroidOffscreenWebView * ret = qobject_cast<QAndroidOffscreenWebView *>(reinterpret_cast<QAndroidOffscreenView*>(reinterpret_cast<void*>(nativeptr)));
		if (ret)
		{
			return ret;
		}
	}
	qWarning()<<"QAndroidOffscreenWebView: Zero native ptr received.";
	return 0;
}

// public native void doUpdateVisitedHistory(long nativeptr, String url, boolean isReload);
Q_DECL_EXPORT void JNICALL Java_doUpdateVisitedHistory(JNIEnv * env, jobject jo, jlong nativeptr, jobject url, jboolean isReload)
{
	if (QAndroidOffscreenWebView * wv = AOWW(nativeptr))
	{
		wv->doUpdateVisitedHistory(env, jo, url, isReload);
	}
}

// public native void onFormResubmission(long nativeptr, Message dontResend, Message resend);
Q_DECL_EXPORT void JNICALL Java_onFormResubmission(JNIEnv * env, jobject jo, jlong nativeptr, jobject dontResend, jobject resend)
{
	if (QAndroidOffscreenWebView * wv = AOWW(nativeptr))
	{
		wv->onFormResubmission(env, jo, dontResend, resend);
	}
}

// public native void onLoadResource(long nativeptr, String url);
Q_DECL_EXPORT void JNICALL Java_onLoadResource(JNIEnv * env, jobject jo, jlong nativeptr, jobject url)
{
	if (QAndroidOffscreenWebView * wv = AOWW(nativeptr))
	{
		wv->onLoadResource(env, jo, url);
	}
}

// public native void onPageFinished(long nativeptr, String url);
Q_DECL_EXPORT void JNICALL Java_onPageFinished(JNIEnv * env, jobject jo, jlong nativeptr, jobject url)
{
	if (QAndroidOffscreenWebView * wv = AOWW(nativeptr))
	{
		wv->onPageFinished(env, jo, url);
	}
}

// public native void onPageStarted(long nativeptr, String url, Bitmap favicon);
Q_DECL_EXPORT void JNICALL Java_onPageStarted(JNIEnv * env, jobject jo, jlong nativeptr, jobject url, jobject favicon)
{
	if (QAndroidOffscreenWebView * wv = AOWW(nativeptr))
	{
		wv->onPageStarted(env, jo, url, favicon);
	}
}

// public native void onReceivedError(long nativeptr, int errorCode, String description, String failingUrl);
Q_DECL_EXPORT void JNICALL Java_onReceivedError(JNIEnv * env, jobject jo, jlong nativeptr, jint errorCode, jobject description, jobject failingUrl)
{
	if (QAndroidOffscreenWebView * wv = AOWW(nativeptr))
	{
		wv->onReceivedError(env, jo, errorCode, description, failingUrl);
	}
}

// public native void onReceivedHttpAuthRequest(long nativeptr, HttpAuthHandler handler, String host, String realm);
Q_DECL_EXPORT void JNICALL Java_onReceivedHttpAuthRequest(JNIEnv * env, jobject jo, jlong nativeptr, jobject handler, jobject host, jobject realm)
{
	if (QAndroidOffscreenWebView * wv = AOWW(nativeptr))
	{
		wv->onReceivedHttpAuthRequest(env, jo, handler, host, realm);
	}
}

// public native void onReceivedLoginRequest(long nativeptr, String realm, String account, String args);
Q_DECL_EXPORT void JNICALL Java_onReceivedLoginRequest(JNIEnv * env, jobject jo, jlong nativeptr, jobject realm, jobject account, jobject args)
{
	if (QAndroidOffscreenWebView * wv = AOWW(nativeptr))
	{
		wv->onReceivedLoginRequest(env, jo, realm, account, args);
	}
}

// public native void onReceivedSslError(long nativeptr, SslErrorHandler handler, SslError error);
Q_DECL_EXPORT void JNICALL Java_onReceivedSslError(JNIEnv * env, jobject jo, jlong nativeptr, jobject handler, jobject error)
{
	if (QAndroidOffscreenWebView * wv = AOWW(nativeptr))
	{
		wv->onReceivedSslError(env, jo, handler, error);
	}
}

// public native void onScaleChanged(long nativeptr, float oldScale, float newScale);
Q_DECL_EXPORT void JNICALL Java_onScaleChanged(JNIEnv * env, jobject jo, jlong nativeptr, jfloat oldScale, jfloat newScale)
{
	if (QAndroidOffscreenWebView * wv = AOWW(nativeptr))
	{
		wv->onScaleChanged(env, jo, oldScale, newScale);
	}
}

// public native void onTooManyRedirects(long nativeptr, Message cancelMsg, Message continueMsg);
Q_DECL_EXPORT void JNICALL Java_onTooManyRedirects(JNIEnv * env, jobject jo, jlong nativeptr, jobject cancelMsg, jobject continueMsg)
{
	if (QAndroidOffscreenWebView * wv = AOWW(nativeptr))
	{
		wv->onTooManyRedirects(env, jo, cancelMsg, continueMsg);
	}
}

// public native void onUnhandledKeyEvent(long nativeptr, KeyEvent event);
Q_DECL_EXPORT void JNICALL Java_onUnhandledKeyEvent(JNIEnv * env, jobject jo, jlong nativeptr, jobject event)
{
	if (QAndroidOffscreenWebView * wv = AOWW(nativeptr))
	{
		wv->onUnhandledKeyEvent(env, jo, event);
	}
}

// public native WebResourceResponse shouldInterceptRequest(long nativeptr, String url);
Q_DECL_EXPORT jobject JNICALL Java_shouldInterceptRequest(JNIEnv * env, jobject jo, jlong nativeptr, jobject url)
{
	if (QAndroidOffscreenWebView * wv = AOWW(nativeptr))
	{
		return wv->shouldInterceptRequest(env, jo, url);
	}
	return 0;
}

// public native boolean shouldOverrideKeyEvent(long nativeptr, KeyEvent event);
Q_DECL_EXPORT jboolean JNICALL Java_shouldOverrideKeyEvent(JNIEnv * env, jobject jo, jlong nativeptr, jobject event)
{
	if (QAndroidOffscreenWebView * wv = AOWW(nativeptr))
	{
		return wv->shouldOverrideKeyEvent(env, jo, event);
	}
	return 0;
}

// public native boolean shouldOverrideUrlLoading(long nativeptr, String url);
Q_DECL_EXPORT jboolean JNICALL Java_shouldOverrideUrlLoading(JNIEnv * env, jobject jo, jlong nativeptr, jobject url)
{
	if (QAndroidOffscreenWebView * wv = AOWW(nativeptr))
	{
		wv->shouldOverrideUrlLoading(env, jo, url);
	}
	return 0;
}

Q_DECL_EXPORT void JNICALL Java_onContentHeightReceived(JNIEnv *, jobject, jlong nativeptr, jint height)
{
	if (QAndroidOffscreenWebView * wv = AOWW(nativeptr))
	{
		wv->onContentHeightReceived(height);
	}
}

QAndroidOffscreenWebView::QAndroidOffscreenWebView(const QString & object_name, const QSize & def_size, QObject * parent)
	: QAndroidOffscreenView(QLatin1String("OffscreenWebView"), object_name, false, def_size, parent)
{
	static const JNINativeMethod methods[] = {
		//
		// WebViewClient Methods
		//
		{"doUpdateVisitedHistory", "(JLjava/lang/String;Z)V", (void*)Java_doUpdateVisitedHistory},
		{"onFormResubmission", "(JLandroid/os/Message;Landroid/os/Message;)V", (void*)Java_onFormResubmission},
		{"onLoadResource", "(JLjava/lang/String;)V", (void*)Java_onLoadResource},
		{"onPageFinished", "(JLjava/lang/String;)V", (void*)Java_onPageFinished},
		{"onPageStarted", "(JLjava/lang/String;Landroid/graphics/Bitmap;)V", (void*)Java_onPageStarted},
		{"onReceivedError", "(JILjava/lang/String;Ljava/lang/String;)V", (void*)Java_onReceivedError},
		{"onReceivedHttpAuthRequest", "(JLandroid/webkit/HttpAuthHandler;Ljava/lang/String;Ljava/lang/String;)V", (void*)Java_onReceivedHttpAuthRequest},
		{"onReceivedLoginRequest", "(JLjava/lang/String;Ljava/lang/String;Ljava/lang/String;)V", (void*)Java_onReceivedLoginRequest},
		{"onReceivedSslError", "(JLandroid/webkit/SslErrorHandler;Landroid/net/http/SslError;)V", (void*)Java_onReceivedSslError},
		{"onScaleChanged", "(JFF)V", (void*)Java_onScaleChanged},
		{"onTooManyRedirects", "(JLandroid/os/Message;Landroid/os/Message;)V", (void*)Java_onTooManyRedirects},
		{"onUnhandledKeyEvent", "(JLandroid/view/KeyEvent;)V", (void*)Java_onUnhandledKeyEvent},
		{"shouldInterceptRequest", "(JLjava/lang/String;)Landroid/webkit/WebResourceResponse;", (void*)Java_shouldInterceptRequest},
		{"shouldOverrideKeyEvent", "(JLandroid/view/KeyEvent;)Z", (void*)Java_shouldOverrideKeyEvent},
		{"shouldOverrideUrlLoading", "(JLjava/lang/String;)Z", (void*)Java_shouldOverrideUrlLoading},

		//
		// Own callbacks
		//
		{"onContentHeightReceived", "(JI)V", (void*)Java_onContentHeightReceived}
	};
	if (jcGeneric * ov = offscreenView())
	{
		qDebug()<<__PRETTY_FUNCTION__<<"Registering"<<sizeof(methods)/sizeof(JNINativeMethod)<<"JNI methods for WebView";
		bool ok = ov->RegisterNativeMethods(methods, sizeof(methods));
		if (!ok)
		{
			qCritical()<<"Failed to register native methods!";
		}
		else
		{
			qDebug()<<"QAndroidOffscreenWebView successfully registered native methods.";
		}
	}
	else
	{
		qCritical("Failed to register native methods of QAndroidOffscreenWebView because Java object pointer is null.");
	}

	// Creating the view
	createView();
}

QAndroidOffscreenWebView::~QAndroidOffscreenWebView()
{
}

void QAndroidOffscreenWebView::preloadJavaClass()
{
	QString path = getDefaultJavaClassPath() + QLatin1String("OffscreenWebView");
	QAndroidQPAPluginGap::preloadJavaClass(path.toLatin1());
}

bool QAndroidOffscreenWebView::loadUrl(const QString & url)
{
	if (!isCreated())
	{
		qWarning("QAndroidOffscreenWebView: Attempt to loadUrl when View is not ready yet.");
		return false;
	}
	jcGeneric * view = offscreenView();
	if (view)
	{
		view->CallVoid("loadUrl", url);
		return true;
	}
	qWarning("QAndroidOffscreenWebView: Attempt to load URL when View is null.");
	return false;
}

bool QAndroidOffscreenWebView::loadUrl(const QString & url, const QMap<QString, QString> & additionalHttpHeaders)
{
	if (!isCreated())
	{
		qWarning("QAndroidOffscreenWebView: Attempt to loadUrl when View is not ready yet.");
		return false;
	}
	jcGeneric * view = offscreenView();
	if (view)
	{
		QString strheaders;
		for(QMap<QString, QString>::const_iterator it = additionalHttpHeaders.begin(); it != additionalHttpHeaders.end(); ++it)
		{
			strheaders += it.key();
			strheaders += QChar('\n');
			strheaders += it.value();
			strheaders += QChar('\n');
		}
		view->CallVoid("loadUrl", url, strheaders);
		return true;
	}
	qWarning("QAndroidOffscreenWebView: Attempt to load URL when View is null.");
	return false;
}

bool QAndroidOffscreenWebView::loadData(const QString & text, const QString & mime, const QString & encoding)
{
	if (!isCreated())
	{
		qWarning("QAndroidOffscreenWebView: Attempt to loadData when View is not ready yet.");
		return false;
	}
	jcGeneric * view = offscreenView();
	if (view)
	{
		view->CallVoid("loadData", text, mime, encoding);
		return true;
	}
	qWarning("QAndroidOffscreenWebView: Attempt to insert URL when View is null.");
	return false;
}

bool QAndroidOffscreenWebView::loadDataWithBaseURL(const QString & baseUrl, const QString & data, const QString & mimeType, const QString & encoding, const QString & historyUrl)
{
	if (!isCreated())
	{
		qWarning("QAndroidOffscreenWebView: Attempt to loadDataWithBaseURL when View is not ready yet.");
		return false;
	}
	jcGeneric * view = offscreenView();
	if (view)
	{
		view->CallVoid("loadDataWithBaseURL", baseUrl, data, mimeType, encoding, historyUrl);
		return true;
	}
	qWarning("QAndroidOffscreenWebView: Attempt to loadDataWithBaseURL when View is null.");
	return false;
}

bool QAndroidOffscreenWebView::requestContentHeight()
{
	if (!isCreated())
	{
		qWarning("QAndroidOffscreenWebView: Attempt to requestContentHeight when View is not ready yet.");
		return 0;
	}
	jcGeneric * view = offscreenView();
	if (view)
	{
		return view->CallBool("requestContentHeight");
	}
	qWarning("QAndroidOffscreenWebView: Attempt to requestContentHeight when View is null.");
	return 0;
}


/////////////////////////////////////////////////////////////////////////////
// WebViewClient
/////////////////////////////////////////////////////////////////////////////

void QAndroidOffscreenWebView::doUpdateVisitedHistory(JNIEnv *, jobject, jobject url, jboolean isReload)
{
	Q_UNUSED(url);
	Q_UNUSED(isReload);
}

void QAndroidOffscreenWebView::onFormResubmission(JNIEnv *, jobject, jobject dontResend, jobject resend)
{
	Q_UNUSED(resend);
	jcGeneric(dontResend).CallVoid("sendToTarget");
}

void QAndroidOffscreenWebView::onLoadResource(JNIEnv *, jobject, jobject url)
{
	Q_UNUSED(url);
}

void QAndroidOffscreenWebView::onPageFinished(JNIEnv *, jobject, jobject url)
{
	Q_UNUSED(url);
	emit pageFinished();
}

void QAndroidOffscreenWebView::onPageStarted(JNIEnv *, jobject, jobject url, jobject favicon)
{
	Q_UNUSED(url);
	Q_UNUSED(favicon);
	emit pageStarted();
}

void QAndroidOffscreenWebView::onReceivedError(JNIEnv *, jobject, int errorCode, jobject description, jobject failingUrl)
{
	Q_UNUSED(errorCode);
	Q_UNUSED(description);
	Q_UNUSED(failingUrl);
}

void QAndroidOffscreenWebView::onReceivedHttpAuthRequest(JNIEnv *, jobject, jobject handler, jobject host, jobject realm)
{
	Q_UNUSED(host);
	Q_UNUSED(realm);
	jcGeneric(handler).CallVoid("cancel");
}

void QAndroidOffscreenWebView::onReceivedLoginRequest(JNIEnv *, jobject, jobject realm, jobject account, jobject args)
{
	Q_UNUSED(realm);
	Q_UNUSED(account);
	Q_UNUSED(args);
}

void QAndroidOffscreenWebView::onReceivedSslError(JNIEnv *, jobject, jobject handler, jobject error)
{
	Q_UNUSED(error);
	jcGeneric(handler).CallVoid("cancel");
}

void QAndroidOffscreenWebView::onScaleChanged(JNIEnv *, jobject, float oldScale, float newScale)
{
	Q_UNUSED(oldScale);
	Q_UNUSED(newScale);
}

void QAndroidOffscreenWebView::onTooManyRedirects(JNIEnv *, jobject, jobject cancelMsg, jobject continueMsg)
{
	Q_UNUSED(continueMsg);
	jcGeneric(cancelMsg).CallVoid("sendToTarget");
}

void QAndroidOffscreenWebView::onUnhandledKeyEvent(JNIEnv *, jobject, jobject event)
{
	Q_UNUSED(event);
}

jobject QAndroidOffscreenWebView::shouldInterceptRequest(JNIEnv *, jobject, jobject url)
{
	Q_UNUSED(url);
	return 0;
}

jboolean QAndroidOffscreenWebView::shouldOverrideKeyEvent(JNIEnv *, jobject, jobject event)
{
	Q_UNUSED(event);
	return 0;
}

jboolean QAndroidOffscreenWebView::shouldOverrideUrlLoading(JNIEnv *, jobject, jobject url)
{
	// Doing OffscreenWebView.loadUrl(url).
	// This should always be done for Chrome to avoid opening links in external browser.
	jcGeneric * aview = QAndroidOffscreenView::getView();
	if (aview)
	{
		aview->CallParamVoid("loadUrl", "Ljava/lang/String;", url);
	}
	return 0;
}


/////////////////////////////////////////////////////////////////////////////
// Own callbacks
/////////////////////////////////////////////////////////////////////////////

void QAndroidOffscreenWebView::onContentHeightReceived(int height)
{
	emit contentHeightReceived(height);
}
