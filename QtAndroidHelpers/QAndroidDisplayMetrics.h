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

#include <QtCore/QString>
#include <QtCore/QObject>

class QAndroidDisplayMetrics: public QObject
{
	Q_OBJECT
	Q_PROPERTY(float density READ density)
	Q_PROPERTY(int densityDpi READ densityDpi)
	Q_PROPERTY(int heightPixels READ heightPixels)
	Q_PROPERTY(float scaledDensity READ scaledDensity)
	Q_PROPERTY(int widthPixels READ widthPixels)
	Q_PROPERTY(float xdpi READ xdpi)
	Q_PROPERTY(float ydpi READ ydpi)
public:
	QAndroidDisplayMetrics(QObject * parent = 0);
	static void preloadJavaClasses();

	float density() const { return density_;  }
	int densityDpi() const { return densityDpi_; }
	int heightPixels() const { return density_; }
	float scaledDensity() const { return scaledDensity_; }
	int widthPixels() const { return widthPixels_; }
	float xdpi() const { return xdpi_; }
	float ydpi() const { return ydpi_; }

private:
	float density_;
	int densityDpi_;
	int heightPixels_;
	float scaledDensity_;
	int widthPixels_;
	float xdpi_;
	float ydpi_;
};
