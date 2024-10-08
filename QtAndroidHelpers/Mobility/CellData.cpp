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

#include "CellData.h"


namespace Mobility {


const qint32 CellData::java_integer_max_value = std::numeric_limits<qint32>::max();
const qint64 CellData::java_long_max_value = std::numeric_limits<qint64>::max();


CellData::Data::Data(int32_t cell_id)
	: cell_id_(cell_id)
	, location_area_code_(java_integer_max_value)
	, mobile_country_code_(java_integer_max_value)
	, mobile_network_code_(java_integer_max_value)
	, signal_strength_(java_integer_max_value)
	, timing_advance_(java_integer_max_value)
	, last_seen_ms_(java_long_max_value)
{
}


void CellData::Data::accept(DataOperation & operation) const
{
	operation.execute(QStringLiteral("cell_id"), cell_id_);
	operation.execute(QStringLiteral("location_area_code"), location_area_code_);
	operation.execute(QStringLiteral("mobile_country_code"), mobile_country_code_);
	operation.execute(QStringLiteral("mobile_network_code"), mobile_network_code_);
	operation.execute(QStringLiteral("signal_strength"), signal_strength_);
	operation.execute(QStringLiteral("timing_advance"), timing_advance_);
	operation.execute(QStringLiteral("radio_type"), radio_type_);
	operation.execute(QStringLiteral("last_seen_ms"), last_seen_ms_);
}

} // namespace Mobility
