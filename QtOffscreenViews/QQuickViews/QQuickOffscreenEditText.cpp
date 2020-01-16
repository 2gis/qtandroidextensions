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

#include "QQuickOffscreenEditText.h"

QQuickAndroidOffscreenEditText::QQuickAndroidOffscreenEditText()
	: QQuickAndroidOffscreenView(new QAndroidOffscreenEditText("EditTextInQuick", QSize(512, 64)))
{
	connect(androidEditText(), SIGNAL(onTextChanged(QString,int,int,int)), this, SLOT(etTextChanged(QString,int,int,int)));
	connect(androidEditText(), SIGNAL(selectionChanged()), this, SIGNAL(selectionChanged()));
	connect(androidEditText(), SIGNAL(onKeyBack(bool)), this, SLOT(etKeyBack(bool)));
	connect(androidEditText(), SIGNAL(onEnterOrPositiveAction()), this, SLOT(etEnter()));
	connect(androidEditText(), SIGNAL(contentHeightChanged(int)), this, SIGNAL(contentHeightChanged(int)));
}

void QQuickAndroidOffscreenEditText::etTextChanged(QString text, int start, int before, int count)
{
	Q_UNUSED(start);
	Q_UNUSED(before);
	Q_UNUSED(count);
	emit onTextChanged(text);
}

void QQuickAndroidOffscreenEditText::etEnter()
{
	emit returnPressed();
}

void QQuickAndroidOffscreenEditText::etKeyBack(bool down)
{
	if (down)
	{
		emit backPressed();
	}
	else
	{
		emit backReleased();
	}
}

void QQuickAndroidOffscreenEditText::setEnableNativeSuggestions(bool enable)
{
	// Note: changing just TYPE_TEXT_FLAG_NO_SUGGESTIONS does not have effect on some
	// software keyboards (e.g. HTC Sense).
	if (enable)
	{
		// Turn off TYPE_TEXT_FLAG_NO_SUGGESTIONS, turn on INPUTTYPE_TYPE_CLASS_TEXT
		setInputTypeAndOr(
			(~QAndroidOffscreenEditText::ANDROID_INPUTTYPE_TYPE_TEXT_FLAG_NO_SUGGESTIONS) & 0x7fffffff,
			QAndroidOffscreenEditText::ANDROID_INPUTTYPE_TYPE_CLASS_TEXT);
	}
	else
	{
		// Turn on TYPE_TEXT_FLAG_NO_SUGGESTIONS, turn off INPUTTYPE_TYPE_CLASS_TEXT
		setInputTypeAndOr(
			0x7fffffff & ~QAndroidOffscreenEditText::ANDROID_INPUTTYPE_TYPE_CLASS_TEXT,
			QAndroidOffscreenEditText::ANDROID_INPUTTYPE_TYPE_TEXT_FLAG_NO_SUGGESTIONS);
	}
}


