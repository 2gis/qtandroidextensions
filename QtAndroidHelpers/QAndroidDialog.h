/*
  Lightweight access to various Android APIs for Qt

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
#include <QtCore/QString>
#include <QtCore/QObject>
#include <QJniHelpers/QJniHelpers.h>
#include <QJniHelpers/IJniObjectLinker.h>

/*

Examples:

1) Simply show a dialog:

QAndroidDialog().showMessage(...);


2) Show a dialog & invoke slot after it's closed:

QAndroidDialog * dialog = new QAndroidDialog();
connect(dialog, SIGNAL(closed()), ....)
dialog->setDeleteSelfOnClose(true);
dialog->showMessage(...);


3) C++ connection examples:

QObject::connect(
    dialog
    , (void (QAndroidDialog::*)())&QAndroidDialog::closed
    , []() {
        // some lambda here
    });

QObject::connect(
    dialog
    , (void (QAndroidDialog::*)(int))&QAndroidDialog::closed
    , [](int button) {
        // some lambda here
    });

NOTE: this class can work with or without Activity but to display dialogs from non-Activity
context one needs a permission in the AndroidManifest.xml:
<uses-permission android:name="android.permission.SYSTEM_ALERT_WINDOW" />
*/

class QAndroidDialog: public QObject
{
	Q_OBJECT
	JNI_LINKER_DECL(QAndroidDialog)
public:
	static const int
		ANDROID_DIALOGINTERFACE_BUTTON_POSITIVE = -1,
		ANDROID_DIALOGINTERFACE_BUTTON_NEGATIVE = -2,
		ANDROID_DIALOGINTERFACE_BUTTON_NEUTRAL  = -3;

	QAndroidDialog(QObject * parent = 0);
	virtual ~QAndroidDialog();

	static void setInteractiveMode(bool interactive);
	static bool isInteractiveMode();

	Q_INVOKABLE bool deleteSelfOnClose() const { return delete_self_on_close_; }
	Q_INVOKABLE void setDeleteSelfOnClose(bool del) { delete_self_on_close_ = del; }

	/*!
	 * Show basic dialog with a message and one, two or three buttons: positive, negative and neutral.
	 * \param pause - the function will not return until the dialog is closed. (Use resultButton()
	 *  to determine which button has been clicked.) Setting this to true also implies lock_rotation.
	 * \param lock_rotation - set to true to disable screen rotation while the dialog is active.
	 */
	Q_INVOKABLE void showMessage(
		const QString & title,
		const QString & explanation,
		const QString & positive_button_text,
		const QString & negative_button_text,
		const QString & neutral_button_text,
		bool pause,
		bool lock_rotation = false);

	//! This is a convenience wrapper for the full version.
	Q_INVOKABLE void showMessage(
		const QString & title,
		const QString & explanation,
		const QString & positive_button_text,
		const QString & negative_button_text,
		bool pause,
		bool lock_rotation = false);

	//! This is a convenience wrapper for the full version.
	Q_INVOKABLE void showMessage(
		const QString & title,
		const QString & explanation,
		const QString & positive_button_text,
		bool pause,
		bool lock_rotation = false);

	Q_INVOKABLE int resultButton() const { return result_button_; }

signals:
	//! Dialog has been closed by click on the positive button.
	void positiveClicked();

	//! Dialog has been closed by click on the negative button.
	void negativeClicked();

	//! Dialog has been closed by click on the neutral button.
	void neutralClicked();

	//! Dialog has been cancelled.
	void cancelled();

	/*!
	 * Dialog has been closed by click on any button or cancelling.
	 * If it has been cancelled, then button is 0.
	 */
	void closed(int button);

	//! Dialog has been closed.
	void closed();

private:
	void showMessageCallback(int button);

private:
	bool delete_self_on_close_;
	int result_button_;
	static bool interactive_;

	friend Q_DECL_EXPORT void JNICALL Java_DialogHelper_DialogHelper_showMessageCallback(JNIEnv *, jobject, jlong param, jint button);
};

