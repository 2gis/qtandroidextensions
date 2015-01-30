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

#include "QQuickOffscreenWebView.h"

QQuickAndroidOffscreenWebView::QQuickAndroidOffscreenWebView()
	: QQuickAndroidOffscreenView(new QAndroidOffscreenWebView("WebViewInQuick", QSize(512, 512)))
{
	connect(androidWebView(), SIGNAL(pageStarted()), this, SLOT(wwPageStarted()));
	connect(androidWebView(), SIGNAL(pageFinished()), this, SLOT(wwPageFinished()));
	connect(androidWebView(), SIGNAL(contentHeightReceived(int)), this, SLOT(wwContentHeightReceived(int)));
	connect(androidWebView(), SIGNAL(canGoBackReceived(bool)), this, SLOT(wwCanGoBackReceived(bool)));
	connect(androidWebView(), SIGNAL(canGoForwardReceived(bool)), this, SLOT(wwCanGoForwardReceived(bool)));
	connect(androidWebView(), SIGNAL(canGoBackOrForwardReceived(bool,int)), this, SLOT(wwCanGoBackOrForwardReceived(bool,int)));
	connect(androidWebView(), SIGNAL(receivedError(int, const QString &, const QString &)), this, SLOT(wwReceivedError(int, const QString &, const QString &)));
	connect(androidWebView(), SIGNAL(receivedSslError(int, const QString &)), this, SLOT(wwReceivedSslError(int, const QString &)));
	connect(androidWebView(), SIGNAL(progressChanged(int)), this, SLOT(wwProgressChanged(int)));
}

void QQuickAndroidOffscreenWebView::wwPageStarted()
{
	emit pageStarted();
}

void QQuickAndroidOffscreenWebView::wwPageFinished()
{
	emit pageFinished();
}

void QQuickAndroidOffscreenWebView::wwContentHeightReceived(int height)
{
	emit contentHeightReceived(height);
}

void QQuickAndroidOffscreenWebView::wwCanGoBackReceived(bool can)
{
	emit canGoBackReceived(can);
}

void QQuickAndroidOffscreenWebView::wwCanGoForwardReceived(bool can)
{
	emit canGoForwardReceived(can);
}

void QQuickAndroidOffscreenWebView::wwCanBackOrForwardReceived(bool can, int steps)
{
	emit canGoBackOrForwardReceived(can, steps);
}

void QQuickAndroidOffscreenWebView::wwReceivedError(int errorCode, const QString & description, const QString & failingUrl)
{
	emit receivedError(errorCode, description, failingUrl);
}

void QQuickAndroidOffscreenWebView::wwReceivedSslError(int primaryError, const QString & failingUrl)
{
	emit receivedSslError(primaryError, failingUrl);
}

void QQuickAndroidOffscreenWebView::wwProgressChanged(int progress)
{
	emit progressChanged(progress);
}
