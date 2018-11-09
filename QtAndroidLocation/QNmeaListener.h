/*
  Offscreen Android Views library for Qt

  Author:
  Artöm R. Khvosch <a.khvosch@2gis.ru>

  Distrbuted under The BSD License

  Copyright (c) 2016, DoubleGIS, LLC.
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


class QNmeaListener : public QObject
{
	Q_OBJECT
	JNI_LINKER_DECL(QNmeaListener)

public:
	QNmeaListener(QObject * parent);
	virtual ~QNmeaListener();

signals:
	/*!
	 * \signal nmeaMessage
	 * \param timestamp - timestamp of NMEA message
	 * \param nmea - string with NMEA message
	 */
	void nmeaMessage(qint64 timestamp, QString nmea);

private:
	friend void JNICALL Java_NmeaListener_OnNmeaReceivedNative(JNIEnv * env, jobject, jlong param, jlong timestamp, jstring str);
};
