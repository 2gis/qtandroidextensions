/*
  Lightweight access to various Android APIs for Qt

  Author:
  Vyacheslav O. Koscheev <vok1980@gmail.com>

  Distrbuted under The BSD License

  Copyright (c) 2020, DoubleGIS, LLC.
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
#include <QJniHelpers/QJniHelpers.h>
#include <QJniHelpers/IJniObjectLinker.h>


class QAndroidVibrator : public QObject
{
	Q_OBJECT
	JNI_LINKER_DECL(QAndroidVibrator)

public:
	enum class Effect
	{
		Click = 0,       // VibrationEffect.EFFECT_CLICK,
		DoubleClick = 1, // VibrationEffect.EFFECT_DOUBLE_CLICK,
		HeavyClick = 5,  // VibrationEffect.EFFECT_HEAVY_CLICK,
		Tick = 2,        // VibrationEffect.EFFECT_TICK,
	};

public:
	QAndroidVibrator(QObject * parent = 0);
	~QAndroidVibrator();

	typedef std::vector<int64_t> Timings_t;

public slots:
	// Predefined vibration effects available on Android 8+.
	// On Android < 8 always starts 50ms vibration, regardless of effect value.
	void vibrate(Effect effect);
	void vibrate(Timings_t::value_type duration);
	void vibrate(Timings_t timings);

	bool hasAmplitudeControl();
};
