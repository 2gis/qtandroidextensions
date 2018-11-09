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

#include <QtCore/QObject>
#include <QJniHelpers/IJniObjectLinker.h>


/*!
 * A class for notification on global relayout on Java side.
 * NOTE: this class doesn't work without Activity, i.e. from a non-GUI app / Service and etc.
 */
class QAndroidScreenLayoutHandler: public QObject
{
	Q_OBJECT
	JNI_LINKER_DECL(QAndroidScreenLayoutHandler)

public:
	QAndroidScreenLayoutHandler(QObject * parent = 0);
	virtual ~QAndroidScreenLayoutHandler();

	//! Subscribe this object to global layout events (by default obect is subscribed)
	void subscribeToLayoutEvents();
	//! Unsubscribe this object from global layout events
	void unsubscribeFromLayoutEvents();

signals:
	/*!
	 * A notification on global relayout on Java side.
	 * Basically it wraps ViewTreeObserver.OnGlobalLayoutListener::onGlobalLayout() signal.
	 * It is emitted in many situations: when screen is rotated, when software keyboard
	 * opens and closes, and also when layout of items inside of the View changes, for example,
	 * for EditText it is emitted as user types text.
	 * A common use case is to connect it to requestVisibleRect(), catch visibleRectReceived()
	 * and scroll page to make sure current text editor is positioned correctly.
	 */
	void globalLayoutChanged();

	void scrollChanged();
	void keyboardHeightChanged(int height);

private:
	void javaGlobalLayoutChanged();
	void javaScrollChanged();
	void javaKeyboardHeightChanged(int height);

private:
	Q_DISABLE_COPY(QAndroidScreenLayoutHandler)
	friend void JNICALL Java_ScreenLayoutHandler_globalLayoutChanged(JNIEnv *, jobject, jlong param);
	friend void JNICALL Java_ScreenLayoutHandler_scrollChanged(JNIEnv *, jobject, jlong param);
	friend void JNICALL Java_ScreenLayoutHandler_keyboardHeightChanged(JNIEnv *, jobject, jlong param, jint height);
};
