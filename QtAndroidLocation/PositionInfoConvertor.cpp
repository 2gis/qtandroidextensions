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


#include "PositionInfoConvertor.h"



static void setPositionAttributeFloat(QGeoPositionInfo & info, QGeoPositionInfo::Attribute attr,
                                      QJniObject & location, const char * szCheck, const char * szGet)
{
	if (location.callBool(szCheck))
	{
		jfloat val = location.callFloat(szGet);
		info.setAttribute(attr, val);
	}
}


QGeoPositionInfo positionInfoFromJavaLocation(const jobject jlocation)
{
	QGeoPositionInfo info;
	QJniObject location(jlocation, false);

	if (!location)
	{
		qWarning() << "null location";
		return QGeoPositionInfo();
	}

	jdouble latitude = location.callDouble("getLatitude");
	jdouble longitude = location.callDouble("getLongitude");
	QGeoCoordinate coordinate(latitude, longitude);

	if (location.callBool("hasAltitude"))
	{
		jdouble value = location.callDouble("getAltitude");
		coordinate.setAltitude(value);
	}

	info.setCoordinate(coordinate);

	jlong timestamp = location.callLong("getTime");
	info.setTimestamp(QDateTime::fromMSecsSinceEpoch(timestamp));

	setPositionAttributeFloat(info, QGeoPositionInfo::HorizontalAccuracy,	location, "hasAccuracy",	"getAccuracy");
	setPositionAttributeFloat(info, QGeoPositionInfo::GroundSpeed,			location, "hasSpeed",		"getSpeed");
	setPositionAttributeFloat(info, QGeoPositionInfo::Direction,			location, "hasBearing",		"getBearing");

	return info;
}
