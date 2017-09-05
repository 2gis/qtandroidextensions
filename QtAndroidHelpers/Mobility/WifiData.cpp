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

#include "WifiData.h"
#include <QtCore/QStringList>


namespace Mobility {

// Используется в bool WifiDataList::SameAs()
class WifiDataEq : public std::unary_function<const WifiData&, bool>
{
private:
	const WifiData::MacAddr &mac_;
public:
	WifiDataEq(const WifiData::MacAddr &mac)
		: mac_(mac)
	{}
	result_type operator()(argument_type d) const
	{
		return !memcmp(d.macAddr, mac_, sizeof(WifiData::MacAddr));
	}
};


QString WifiData::MacAsString(const QString& delim) const
{
#define ARGBYTE(x) arg(ushort(macAddr[x]), 2, 16, QChar('0'))
	QString fmt = "%1";
	for (int i = 2; i <= 6; i++)
	{
		fmt += delim + "%" + QString::number(i);
	}
	return QString(fmt).ARGBYTE(0).ARGBYTE(1).ARGBYTE(2).ARGBYTE(3).ARGBYTE(4).ARGBYTE(5);
#undef ARGBYTE
}


void WifiData::StringAsMac(const QString& str, const QString& delim)
{
	if (str.length() < 17)
	{
		return;
	}
	QStringList bytes = str.split(delim);
	if (bytes.size() != MacAddrLength)
	{
		return;
	}
	for (int i = 0; i < MacAddrLength; ++i)
	{
		bool ok;
		macAddr[i] = MacAddrSign(bytes.at(i).toInt(&ok, 16));
		if (!ok)
		{
			memset(&macAddr, 0, sizeof macAddr);
			break;
		}
	}
}


bool WifiDataList::operator==(const WifiDataList &rhs) const
{
	return SameAs(rhs, true);
}


bool WifiDataList::operator!=(const WifiDataList &rhs) const
{
	return !(*this == rhs);
}


void WifiDataList::DbgPrint(const WifiDataList &another)
{
	Q_UNUSED(another)
}


bool WifiDataList::SameAs(const WifiDataList &other, bool compareSignalStrength) const
{
	if ( size() != other.size() )
	{
		return false;
	}
	
	for (WifiDataList::const_iterator myiter = begin(); myiter != end(); ++myiter)
	{
		WifiDataList::const_iterator otheriter = std::find_if(other.begin(), other.end(), WifiDataEq(myiter->macAddr));
		if (otheriter == other.end())
		{
			return false;
		}
		if( compareSignalStrength && myiter->signalStrength != otheriter->signalStrength)
		{
			return false;
		}
	}
	return true;
}

}// namespace Mobility 
