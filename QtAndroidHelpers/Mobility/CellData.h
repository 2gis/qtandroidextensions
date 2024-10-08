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

#include "DataOperation.h"
#include <QtCore/QSharedPointer>


namespace Mobility {

struct CellData
{
	struct Data
	{
		Data(qint32 cell_id);
		void accept(DataOperation & operation) const;

		// required, int.
		// Cell ID (CID) for GSM
		// Base Station ID (BID) for CDMA
		// UTRAN/GERAN Cell Identity (UC-Id) for WCDMA.
		qint32 cell_id_;

		// optional, int.
		// Location Area Code (LAC) for GSM and WCDMA.
		// Network ID (NID) for CDMA сетей. 0 <= LAC <= 65535.
		qint32 location_area_code_;

		// optional, int. 0 <= MCC < 1000
		qint32 mobile_country_code_;

		// optional, int. 0 <= MNC < 1000
		qint32 mobile_network_code_;

		// optional, int. RSSI в dBm.
		qint32 signal_strength_;

		// optional, int
		qint32 timing_advance_;

		// gsm, wcdma, lte, cdma
		QString radio_type_;

		// optional, long
		qint64 last_seen_ms_;
	};

	typedef QList<Data> DataColl;
	DataColl data_;
	static const qint32 java_integer_max_value;
	static const qint64 java_long_max_value;
};


typedef QSharedPointer<CellData> CellDataPtr;

}
