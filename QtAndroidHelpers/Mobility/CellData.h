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

#include <QSharedPointer>


namespace Mobility {

struct CellData
{
	CellData();
	bool compare( const CellData& other, bool compareSignalStrength ) const;
	bool operator==(const CellData& other) const;
	bool operator!=(const CellData& other) const;
	bool isEmpty() const; // used as sign of validity
	void clear();

	// fields are described in http://code.google.com/intl/ru/apis/gears/geolocation_network_protocol.html
	// Unique identifier of the cell. (CID for GSM, BID for CDMA)
	int cellId;

	// Location Area Code (LAC for GSM, NID for CDMA)
	int locationAreaCode;

	// Mobile Country Code (MCC for GSM and CDMA)
	int mobileCountryCode;

	// Mobile Network Code (MNC for GSM, SID for CDMA)
	int mobileNetworkCode;

	// Radio signal strength measured in dBm.
	/* convert GSM asu (TS 27.007 8.5) to these as follows:
	<rssi>:
	0 -113 dBm or less
	1 -111 dBm
	2...30 -109... -53 dBm
	31 -51 dBm or greater
	99 not known or not detectable
	*/
	int signalStrength;

	// Represents the distance from the cell tower. Each unit is roughly 550 meters.
	int timingAdvance;
};


typedef QSharedPointer<CellData> CellDataPtr;

}
