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

#pragma once
#include <QtOffscreenViews/QAndroidOffscreenWebView.h>
#include "QQuickAndroidOffscreenView.h"

class QQuickAndroidOffscreenWebView: public QQuickAndroidOffscreenView
{
	Q_OBJECT
	Q_PROPERTY(bool ignoreSslErrors READ getIgnoreSslErrors WRITE setIgnoreSslErrors)
public:
	QQuickAndroidOffscreenWebView();

	bool getIgnoreSslErrors() const { return androidWebView()->getIgnoreSslErrors(); }
	void setIgnoreSslErrors(bool ignore) { androidWebView()->setIgnoreSslErrors(ignore); }

protected:
	QAndroidOffscreenWebView * androidWebView() { return static_cast<QAndroidOffscreenWebView*>(androidView()); }
	const QAndroidOffscreenWebView * androidWebView() const { return static_cast<const QAndroidOffscreenWebView*>(androidView()); }

public slots:
	void loadUrl(QString url) { androidWebView()->loadUrl(url); }
	void loadData(const QString & text, const QString & mime, const QString & encoding = QString::null) { androidWebView()->loadData(text, mime, encoding); }
	void loadData(const QString & text) { androidWebView()->loadData(text); }
	void loadDataWithBaseURL(const QString & baseUrl, const QString & data, const QString & mimeType, const QString & encoding, const QString & historyUrl) { androidWebView()->loadDataWithBaseURL(baseUrl, data, mimeType, encoding, historyUrl); }
	void requestContentHeight() { androidWebView()->requestContentHeight(); }

	void requestCanGoBack() { androidWebView()->requestCanGoBack(); }
	void goBack() { androidWebView()->goBack(); }
	void requestCanGoForward() { androidWebView()->requestCanGoForward(); }
	void goForward() { androidWebView()->goForward(); }
	void requestCanGoBackOrForward(int steps) { androidWebView()->requestCanGoBackOrForward(steps); }
	void goBackOrForward(int steps) { androidWebView()->goBackOrForward(steps); }
	void setWebContentsDebuggingEnabled(bool enabled){androidWebView()->setWebContentsDebuggingEnabled(enabled);}

signals:
	void pageStarted(const QString & url);
	void pageFinished(const QString & url);
	void receivedError(int errorCode, const QString & description, const QString & failingUrl);
	void receivedSslError(int primaryError, const QString & failingUrl);
	void contentHeightReceived(int height);
	void canGoBackReceived(bool can);
	void canGoForwardReceived(bool can);
	void canGoBackOrForwardReceived(bool can, int steps);
	void progressChanged(int progress);

protected slots:
	virtual void wwPageStarted(const QString & url);
	virtual void wwPageFinished(const QString & url);
	virtual void wwReceivedError(int errorCode, const QString & description, const QString & failingUrl);
	virtual void wwReceivedSslError(int primaryError, const QString & failingUrl);
	virtual void wwContentHeightReceived(int height);
	virtual void wwCanGoBackReceived(bool can);
	virtual void wwCanGoForwardReceived(bool can);
	virtual void wwCanBackOrForwardReceived(bool can, int steps);
	virtual void wwProgressChanged(int progress);
};

