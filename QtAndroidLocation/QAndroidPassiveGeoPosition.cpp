/*
  Simple access to Android passive location.

  Author:
  Sergey A. Galin <sergey.galin@gmail.com>

  Distrbuted under The BSD License

  Copyright (c) 2018, DoubleGIS, LLC.
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

#include "QAndroidPassiveGeoPosition.h"
#include <QJniHelpers/QAndroidQPAPluginGap.h>
#include <QtAndroidHelpers/QAndroidDesktopUtils.h>


namespace QAndroidPassiveGeoPosition {

QGeoPositionInfo get()
{
	QGeoPositionInfo result;
	try
	{
		if (QAndroidDesktopUtils::checkSelfPermission("android.permission.ACCESS_COARSE_LOCATION")
			|| QAndroidDesktopUtils::checkSelfPermission("android.permission.ACCESS_FINE_LOCATION"))
		{
			QAndroidQPAPluginGap::Context jcontext;
			if (!jcontext.jObject())
			{
				qCritical() << "No Android Context!";
				return result;
			}
			const QScopedPointer<QJniObject> location_manager(jcontext.callParamObject(
				"getSystemService"
				, "java/lang/Object" // "android/location/LocationManager"
				, "Ljava/lang/String;"
				, QJniLocalRef(QStringLiteral("location")).jObject()));
			if (!location_manager || !location_manager->jObject())
			{
				qWarning() << "No LocationManager!";
				return result;
			}
			const QScopedPointer<QJniObject> location(location_manager->callParamObject(
				"getLastKnownLocation"
				, "android/location/Location"
				, "Ljava/lang/String;"
				, QJniLocalRef(QStringLiteral("passive")).jObject()));
			if (location && location->jObject())
			{
				const jdouble lon = location->callDouble("getLongitude");
				const jdouble lat = location->callDouble("getLatitude");
				const jfloat accuracy = location->callFloat("getAccuracy");
				const jlong time  = location->callLong("getTime");
				result.setCoordinate(QGeoCoordinate(static_cast<double>(lat), static_cast<double>(lon)));
				result.setTimestamp(QDateTime::fromMSecsSinceEpoch(static_cast<qint64>(time), Qt::TimeSpec::UTC));
				result.setAttribute(QGeoPositionInfo::Attribute::HorizontalAccuracy, static_cast<qreal>(accuracy));
			}
		}
	}
	catch (const std::exception & e)
	{
		qCritical() << "JNI exception while reading last location: " << e.what();
	}
	return result;
}

} // namespace QAndroidPassiveGeoPosition
