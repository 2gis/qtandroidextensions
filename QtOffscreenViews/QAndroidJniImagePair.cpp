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

#include <unistd.h>
#include <android/bitmap.h>
#include <QDebug>
#include "QAndroidJniImagePair.h"

static QImage::Format AndroidBitmapFormat_to_QImageFormat(uint32_t abf)
{
	switch(abf)
	{
	case ANDROID_BITMAP_FORMAT_RGB_565:
		return QImage::Format_RGB16;

	case ANDROID_BITMAP_FORMAT_RGBA_8888:
		// "Note: Do not render into ARGB32 images using QPainter.
		// Using QImage::Format_ARGB32_Premultiplied is significantly faster." - Qt docs
		return QImage::Format_ARGB32_Premultiplied;

	case ANDROID_BITMAP_FORMAT_RGBA_4444:
		qWarning()<<"Warning: untested screen format RGBA 4444!";
		return QImage::Format_ARGB4444_Premultiplied;

	case ANDROID_BITMAP_FORMAT_A_8:
		qCritical()<<"Warning: grayscale video mode is not supported yet!";
		return QImage::Format_Invalid;

	default:
		qCritical()<<"ERROR: Invalid Android bitmap format:"<<abf;
		return QImage::Format_Invalid;
	}
}

static QImage::Format qtImageFormatForBitness(int bitness)
{
	// Now, everything is rewritten so the format mapping happens inside of
	// AndroidBitmapFormat_to_QImageFormat().
	switch(bitness)
	{
		case 32: return AndroidBitmapFormat_to_QImageFormat(ANDROID_BITMAP_FORMAT_RGBA_8888);
		case 16: return AndroidBitmapFormat_to_QImageFormat(ANDROID_BITMAP_FORMAT_RGB_565);
		case 8:  return AndroidBitmapFormat_to_QImageFormat(ANDROID_BITMAP_FORMAT_A_8);
		default:
			qCritical()<<"Invalid image bitness:"<<bitness;
			return AndroidBitmapFormat_to_QImageFormat(ANDROID_BITMAP_FORMAT_RGBA_8888);
	}
}

QAndroidJniImagePair::QAndroidJniImagePair(int bitness)
	: mBitmap()
    , mImageOnBitmap()
	, bitness_(bitness)
{
}

QAndroidJniImagePair::~QAndroidJniImagePair()
{
    deallocate();
}

void QAndroidJniImagePair::dummy()
{
	if (mImageOnBitmap.width() == 1 && mImageOnBitmap.height() == 1 && !mBitmap)
	{
        return; // Already a dummy
	}
    deallocate();
	mImageOnBitmap = QImage(1, 1, qtImageFormatForBitness(bitness_));
}

void QAndroidJniImagePair::deallocate()
{
    mImageOnBitmap = QImage(); // In case deallocate() is called not from destructor...
	mBitmap.reset();
}

bool QAndroidJniImagePair::resize(const QSize & size)
{
	qDebug()<<"QAndroidJniImagePair::resize (static) tid:"<<gettid()<<"new size:"<<size;

	if (size.width() < 1 || size.height() < 1)
	{
        qCritical()<<__FUNCTION__<<"- Supplied image dimenstions are invalid!";
        return false;
    }

	QJniEnvPtr jep;

    // We'll need a new bitmap for the new size
	QImage::Format format = qtImageFormatForBitness(bitness_);

    //
    // Create Android bitmap object by calling appropriate Java function over JNI
    //

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! SGEXP
//	aint->surfaceCreate(format, size)
	QJniLocalRef newBitmap(0);
	if (newBitmap.jObject() == 0)
    {
        qCritical("Could not create %dx%d surface", size.width(), size.height());
		dummy();
        return false;
    }

    //
    // Request image format for the bitmap created
    //
    // AndroidBitmap_getInfo() does not work on Android 1.6, so
    // on API4 we are using a stub.
    // On newer systems, we honestly ask Android bitmap for its
    // actual dimensions, format and stride.
    uint32_t bwidth, bheight, bstride;
	// Android > 1.6
	AndroidBitmapInfo binfo;
	memset(&binfo, 0, sizeof(binfo)); // Important!
	int getinforesult = AndroidBitmap_getInfo(jep.env(), newBitmap, &binfo);
	if (getinforesult != 0)
	{
		#if 1
			// On some bogus devices call to AndroidBitmap_getInfo()
			// may return error code. Zeroing binfo will cause the code below
			// to fall back to calculating it here.
			qWarning()<<"Could not get new surface info, error:"<<getinforesult;
			memset(&binfo, 0, sizeof(binfo));
		#else
			qCritical()<<"Could not get new surface info, error:"<<getinforesult;
			env->DeleteLocalRef(newBitmap);
			if (bitmap != 0)
			{
				env->DeleteGlobalRef(bitmap);
				bitmap = 0;
			}
			imageOnBitmap = QImage();
			return false;
		#endif
	}
	bwidth = binfo.width;
	bheight = binfo.height;
	bstride = binfo.stride;
	// binfo.flags is typically 0 and is not used.

	// Here, we have to make a workaround for some sorry devices.
	// For example, Huawei MediaPad returns: stride instead of width, width instead of height,
	// zero instead of stride. In such case we just try to assume that the image format is
	// standard. Worked good so far.
	if (binfo.format == 0 || bstride == 0 || bwidth == 0 || bheight == 0 || bstride < bwidth)
	{
		qCritical()<<"Invalid AndroidBitmapInfo. Will fall back to standard bitmap properties: "
					 "width:"<<bwidth<<"height:"<<bheight
				   <<"stride:"<<bstride<<"format:"<<binfo.format<<"flags:"<<binfo.flags;
		bwidth = size.width();
		bheight = size.height();
		bstride = size.width() * ((bitness_ == 32)? 4: 2);
		format = qtImageFormatForBitness(bitness_);
	}
	else
	{
		format = AndroidBitmapFormat_to_QImageFormat(binfo.format);
		if (format == QImage::Format_Invalid)
		{
			qCritical()<<"Don't know how to create bitmap of this Android bitmap format:"<<binfo.format;
			dummy();
			return false;
		}
	}
	if (uint32_t(size.width()) != bwidth || uint32_t(size.height()) != bheight)
	{
		qWarning()<<"Android bitmap size:"<<bwidth<<"x"<<bheight
				  <<"is different than the requested size:"<<size.width()<<"x"<<size.height();
	}

    qDebug()<<"AndroidBitmapInfo: width:"<<bwidth<<"height:"<<bheight
            <<"stride:"<<bstride<<"QImage::format:"<<static_cast<int>(format);

    //
    // Lock Android bitmap's pixels so we could create a QImage over it
    //
	void * ptr = 0;
	int lockpixelsresult = AndroidBitmap_lockPixels(jep.env(), newBitmap, &ptr);
	if (lockpixelsresult != 0)
	{
		qCritical()<<"Could not get new surface pointer, error:"<<lockpixelsresult;
		dummy();
        return false;
    }
	if (!ptr)
	{
        qCritical()<<"Could not get new surface pointer, null pointer returned.";
		dummy();
        return false;
    }

    //
    // Create QImage in the same memory area as the Android bitmap
    //

    // "Constructs an image with the given width, height and format,
    // that uses an existing memory buffer, data. The width and height
    // must be specified in pixels. bytesPerLine specifies the number
    // of bytes per line (stride)."
    qDebug()<<"Constructing QImage buffer:"<<bwidth<<"x"<<bheight
            <<bstride<<static_cast<int>(format);
	mImageOnBitmap = QImage(
		static_cast<uchar*>(ptr),
		bwidth,
		bheight,
		bstride,
		format);
	if (mImageOnBitmap.isNull())
	{
        qCritical()<<"Error: called QImage constructor but got null image! Memory error?";
		dummy();
		return false;
	}

	mBitmap.reset(new QJniObject(newBitmap.jObject(), false));
    return true;
}

