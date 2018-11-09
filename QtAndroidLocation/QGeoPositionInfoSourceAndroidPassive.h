/*
	Offscreen Android Views library for Qt

	Author:
	  Vyacheslav O. Koscheev <vok1980@gmail.com>

	Distrbuted under The BSD License

	Copyright (c) 2017, DoubleGIS, LLC.
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

#include <QtPositioning/QGeoPositionInfoSource>
#include <QJniHelpers/IJniObjectLinker.h>


class QGeoPositionInfoSourceAndroidPassive : public QGeoPositionInfoSource
{
	Q_OBJECT
	JNI_LINKER_DECL(QGeoPositionInfoSourceAndroidPassive)
	Q_DISABLE_COPY(QGeoPositionInfoSourceAndroidPassive)

public:
	QGeoPositionInfoSourceAndroidPassive(QObject * parent = 0);
	virtual ~QGeoPositionInfoSourceAndroidPassive();

public:
	// From QGeoPositionInfoSource
	virtual void setUpdateInterval(int msec);
	virtual QGeoPositionInfo lastKnownPosition(bool fromSatellitePositioningMethodsOnly = false) const;
	virtual PositioningMethods supportedPositioningMethods() const;
	virtual void setPreferredPositioningMethods(const PositioningMethods methods);
	virtual int minimumUpdateInterval() const;
	virtual Error error() const;

public Q_SLOTS:
	virtual void startUpdates();
	virtual void stopUpdates();
	virtual void requestUpdate(int timeout = 0);

private:
	friend void JNICALL Java_GeoPositionInfoSourceAndroidPassive_onLocation(JNIEnv * env, jobject, jlong param, jobject location);
	void onLocationRecieved(QGeoPositionInfo location);

private:
	bool started_;
	jint update_interval_msec_;
};
