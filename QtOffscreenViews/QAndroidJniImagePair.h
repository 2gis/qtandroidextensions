/*
  Offscreen Android Views library for Qt

  Authors:
  Sergey A. Galin <sergey.galin@gmail.com>
  Ivan 'w23' Avdeev <marflon@gmail.com>

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

#pragma once
#include <jni.h>
#include <QImage>
#include <QSize>
#include <QScopedPointer>
#include <QAndroidQPAPluginGap.h>
#include <QJniHelpers.h>

class QAndroidJniImagePair
	: public QObject
{
    Q_OBJECT
public:
	QAndroidJniImagePair(int bitness = 32);
	virtual ~QAndroidJniImagePair();
	static void preloadJavaClasses();

    // Create a dummy object with 1x1 px QImage and no jbitmap.
    // jbitmap is released, if necessary.
    // This is used for some workarounds.
    void dummy();

	/*!
	 * After call of this function, imageOnBitmap and bitmap have "size" size
	 * and are located in the same memory (i.e. share the same image data).
	 * WARNING: size must be valid (non-zero and not too huge) or the function
	 * will just return false without doing anything.
	 */
	bool resize(const QSize & size);

	jobject jbitmap(){ return (mBitmap)? mBitmap->jObject(): 0; }
	QImage & qimage(){ return mImageOnBitmap; }
	const QImage & qimage() const { return mImageOnBitmap; }

	bool isOk() const { return mBitmap && mBitmap->jObject() != 0 && !mImageOnBitmap.isNull(); }

    bool hasSize(int w, int h) const
    {
        return isOk() && mImageOnBitmap.width()==w && mImageOnBitmap.height()==h;
    }

    bool ensureSize(int w, int h)
    {
		if (hasSize(w, h))
		{
            return true;
		}
        return resize(QSize(w, h));
    }

    bool ensureSize(const QSize& sz)
    {
		if (hasSize(sz.width(), sz.height()))
		{
            return true;
		}
        return resize(sz);
    }
protected:
	void deallocate();
	QJniObject * createBitmap(const QSize & size);

private:
	QJniObject qjniimagepairclass_;
	mutable QScopedPointer<QJniObject> mBitmap;
    QImage mImageOnBitmap;
	int bitness_;
};

