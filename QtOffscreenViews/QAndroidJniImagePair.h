/*
  Offscreen Android Views library for Qt

  Author:
  Sergey A. Galin <sergey.galin@gmail.com>

  Contributor:
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
#include <QtGui/QImage>
#include <QtCore/QSize>
#include <QtCore/QScopedPointer>
#include <QJniHelpers/QAndroidQPAPluginGap.h>
#include <QJniHelpers/QJniHelpers.h>

/*!
 * This class holds QImage and Android Bitmap sharing the same pixel buffer.
 * For 16 bit, the image can be used in Android and in Qt at the same time.
 * The 16 bit mode may have visible color errors and has no significant
 * performance benefits comparing to 32 bits.
 * For 32 bit, it is necessary to call convert32BitImageFromQtToAndroid() /
 * convert32BitImageFromAndroidToQt() to fix color plane order.
 */
class QAndroidJniImagePair
	: public QObject
{
    Q_OBJECT
	Q_PROPERTY(int bitness READ bitness)
	Q_PROPERTY(QSize size READ size WRITE resize)
public:
	/*!
	 * Create an image pair object for given color resoultion.
	 * To actually allocate the bitmap call resize().
	 * \param bitness can be 32 or 16.
	 */
	QAndroidJniImagePair(int bitness = 32);
	virtual ~QAndroidJniImagePair();

	int bitness() const { return bitness_; }

	/*!
	 * Call this from main() to make sure that Java classes will be accessible.
	 */
	static void preloadJavaClasses();

	/*!
	 * After call of this function, Java-side Bitmap is released and
	 * QImage is assigned with 1 pixel image (i.e. QImage is never a null image).
	 */
	void dispose();

	//! Global Java reference to the Java-side Bitmap.
	jobject jbitmap(){ return (mBitmap)? mBitmap->jObject(): 0; }

	//! Reference to the const QImage.
	const QImage & qImage() const { return mImageOnBitmap; }

	//! Fills image with uniform color
	void fill(const QColor & color, bool to_android_color);

	//! Swap color planes so Qt image starts to look correct on Android.
	void convert32BitImageFromQtToAndroid();

	//! Swap color planes so Qt image starts to look correct on Android.
	void convert32BitImageFromQtToAndroid(QImage & out_image) const;

	//! Swap color planes so Android image starts to look correct on Qt.
	void convert32BitImageFromAndroidToQt();

	//! Swap color planes so Android image starts to look correct on Qt.
	void convert32BitImageFromAndroidToQt(QImage & out_image) const;

	//! Returns true if shared bitmap is allocated.
	bool isAllocated() const;

	/*!
	 * Allocate bitmap of the given size. If the bitmap is already allocated it does nothing.
	 * \return true if bitmap of the requested size has been successfully allocated.
	 */
	bool resize(int w, int h);

	/*!
	 * Allocate bitmap of the given size. If the bitmap is already allocated it does nothing.
	 * \return true if bitmap of the requested size has been successfully allocated.
	 */
	bool resize(const QSize & sz);

	/*!
	 * Get size. Note that it does not always match qImage().size() because
	 * when the bitmap is not allocated it returns QSize(0, 0) while qImage().size()
	 * will give QSize(1, 1).
	 */
	QSize size() const;

	/*!
	 * Load Android resource identified by integer id.
	 * Note that the image will be in Android color order after loading; call
	 * to convert32BitImageFromAndroidToQt() to fix that.
	 * \return true if loaded successfully and false on error. Not throwing exceptions.
	 */
	bool loadResource(jint res_id);

	/*!
	 * Load Android resource identified by resource name.
	 * See comments for loadResource(jint)!
	 */
	bool loadResource(const QString & res_name, const QString & category = QLatin1String("drawable"));
protected:
	/*!
	 * Create Java-side bitmap of the given size for current bitness_.
	 * \return Returns JNI object or 0.
	 */
	QJniObject * createBitmap(const QSize & size);

	/*!
	 * After a call to  this function, imageOnBitmap and bitmap have "size" size
	 * and are located in the same memory (i.e. share the same image data).
	 * WARNING: size must be valid (non-zero and not too huge) or the function
	 * will just return false without doing anything.
	 */
	bool doResize(const QSize & size);

private:
	QScopedPointer<QJniObject> mBitmap;
	QImage mImageOnBitmap;
	int bitness_;
};

