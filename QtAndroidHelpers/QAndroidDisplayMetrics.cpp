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

#include <QJniHelpers.h>
#include <QAndroidQPAPluginGap.h>
#include "QAndroidDisplayMetrics.h"

QAndroidDisplayMetrics::QAndroidDisplayMetrics(QObject * parent)
	: QObject(parent)
	, density_(0)
	, densityDpi_(0)
	, heightPixels_(0)
	, scaledDensity_(0)
	, widthPixels_(0)
	, xdpi_(0)
	, ydpi_(0)
{
	QJniObject metrics("android/util/DisplayMetrics", true);
	{
		QJniObject activity(QAndroidQPAPluginGap::getActivity(), true);
		QScopedPointer<QJniObject> windowmanager(activity.callObject("getWindowManager", "android/view/WindowManager"));
		QScopedPointer<QJniObject> defaultdisplay(windowmanager->callObject("getDefaultDisplay", "android/view/Display"));
		defaultdisplay->callParamVoid("getMetrics", "Landroid/util/DisplayMetrics;", metrics.jObject());
	}
	density_ = metrics.getFloatField("density");
	densityDpi_ = metrics.getIntField("densityDpi");
	heightPixels_ = metrics.getIntField("heightPixels");
	scaledDensity_ = metrics.getFloatField("scaledDensity");
	widthPixels_ = metrics.getIntField("widthPixels");
	xdpi_ = metrics.getFloatField("xdpi");
	ydpi_ = metrics.getFloatField("ydpi");
	qDebug()<<"QAndroidDisplayMetrics: density ="<<density_<<"/ densityDpi ="<<densityDpi_
		   <<"/ scaledDensity ="<<scaledDensity_
		   <<"/ xdpi ="<<xdpi_<<"/ ydpi ="<<ydpi_
		   <<"/ widthPixels ="<<widthPixels_<<"/ heightPixels ="<<heightPixels_;
}

void QAndroidDisplayMetrics::preloadJavaClasses()
{
	QAndroidQPAPluginGap::preloadJavaClass("android/util/DisplayMetrics");
}
