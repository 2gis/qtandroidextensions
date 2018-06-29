/*
	Offscreen Android Views library for Qt

	Authors:
		Vyacheslav O. Koscheev <vok1980@gmail.com>
		Ivan Avdeev marflon@gmail.com
		Sergey A. Galin sergey.galin@gmail.com

	Distrbuted under The BSD License

	Copyright (c) 2015, DoubleGIS, LLC.
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
#include <QtCore/QReadWriteLock>
#include <QJniHelpers.h>
#include "IJniObjectLinker.h"
#include "CellData.h"


namespace Mobility {

class QAndroidCellDataProvider : public QObject
{
	Q_OBJECT
	JNI_LINKER_DECL(QAndroidCellDataProvider)

public:
	QAndroidCellDataProvider(QObject * parent = 0);
	virtual ~QAndroidCellDataProvider();

public:
	CellDataPtr getLastData();

private:
	friend void JNICALL Java_CellListener_cellUpdate(JNIEnv *, jobject, jlong native_ptr, jstring type, jint cid, jint lac, jint mcc, jint mnc, jint rssi, jint ta);
	friend void JNICALL JNICALL Java_CellListener_onSignalChanged(JNIEnv *, jobject, jlong native_ptr);
	void cellUpdate(jstring type, jint cid, jint lac, jint mcc, jint mnc, jint rssi, jint ta);
	void onSignalChanged();

public slots:
	void start();
	void stop();
	void requestData();

signals:
	void updated();
	void dataReady();

private:
	CellDataPtr last_data_;
	CellDataPtr current_data_;
	QReadWriteLock lock_data_;
};

}
