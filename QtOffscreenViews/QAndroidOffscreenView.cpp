/*
  Offscreen Android Views library for Qt

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

#include <unistd.h>
#include <stdlib.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <QThread>
#include <QTimer>
#include <QMutexLocker>
#include <QCoreApplication>
#include <QAndroidQPAPluginGap.h>
#include <QAndroidJniImagePair.h>
#include "QAndroidOffscreenView.h"

static const QString c_class_path_(QLatin1String("ru/dublgis/offscreenview/"));

Q_DECL_EXPORT void JNICALL Java_OffscreenView_nativeUpdate(JNIEnv *, jobject, jlong param)
{
	if (param)
	{
		void * vp = reinterpret_cast<void*>(param);
		QAndroidOffscreenView * proxy = reinterpret_cast<QAndroidOffscreenView*>(vp);
		if (proxy)
		{
			proxy->javaUpdate();
			return;
		}
	}
	qWarning()<<__FUNCTION__<<"Zero param!";
}

Q_DECL_EXPORT void JNICALL Java_OffscreenView_nativeViewCreated(JNIEnv *, jobject, jlong param)
{
	if (param)
	{
		void * vp = reinterpret_cast<void*>(param);
		QAndroidOffscreenView * proxy = reinterpret_cast<QAndroidOffscreenView*>(vp);
		if (proxy)
		{
			proxy->javaViewCreated();
			return;
		}
	}
	qWarning()<<__FUNCTION__<<"Zero param!";
}

QAndroidOffscreenView::QAndroidOffscreenView(
	const QString & classname
	, const QString & objectname
	, bool create_view
	, const QSize & defsize
	, QObject * parent)
	: QObject(parent)
	, view_class_name_(classname)
	, view_object_name_(objectname)
	, tex_()
	, android_to_qt_buffer_()
	, last_qt_buffer_(-1)
	, bitmap_a_(32)
	, bitmap_b_(32)
	, bitmaps_mutex_(QMutex::Recursive)
	, size_(defsize)
	, fill_color_(Qt::white)
	, need_update_texture_(false)
	, view_painted_(false)
	, texture_received_(false)
	, synchronized_texture_update_(true)
	, view_creation_requested_(false)
	, is_visible_(false)
	, is_enabled_(true)
	, view_created_(false)
	, last_texture_width_(0)
	, last_texture_height_(0)
{
	setObjectName(objectname);

	// Expand like: OffscreenWebView => ru/dublgis/offscreenview/OffscreenWebView
	if (!view_class_name_.contains('/'))
	{
		view_class_name_.prepend(c_class_path_);
		qDebug()<<__PRETTY_FUNCTION__<<"Class name"<<classname<<"expanded to"<<view_class_name_;
	}

	// qDebug()<<__PRETTY_FUNCTION__<<"Connecting to java.lang.System...";
	// system_class_.reset(new QJniObject("java/lang/System", false));

	qDebug()<<__PRETTY_FUNCTION__<<"Creating object of"<<view_class_name_<<"tid"<<gettid();
	offscreen_view_.reset(new QJniObject(
		view_class_name_.toLatin1()
		, true // Keep ownership
	));

	if (offscreen_view_ && offscreen_view_->jObject())
	{
		offscreen_view_->registerNativeMethod("nativeUpdate", "(J)V", (void*)Java_OffscreenView_nativeUpdate);
		offscreen_view_->registerNativeMethod("nativeViewCreated", "(J)V", (void*)Java_OffscreenView_nativeViewCreated);
		offscreen_view_->registerNativeMethod("nativeGetActivity", "()Landroid/app/Activity;", (void*)QAndroidQPAPluginGap::getActivity);
		offscreen_view_->callVoid("SetObjectName", view_object_name_);
		offscreen_view_->callParamVoid("SetNativePtr", "J", jlong(reinterpret_cast<void*>(this)));
		offscreen_view_->callParamVoid("setFillColor", "IIII",
			jint(fill_color_.alpha()), jint(fill_color_.red()), jint(fill_color_.green()), jint(fill_color_.blue()));

		// Invoke creation of the view, so its functions will be available
		// before initialization of GL part.
		if (create_view)
		{
			createView();
		}
	}
	else
	{
		qCritical()<<"Failed to create View:"<<view_class_name_<<"/"<<view_object_name_
			<<"Please make sure that all Java classes are present in the project, and also that the Java class is pre-loaded.";
		offscreen_view_.reset();
		return;
	}
}

bool QAndroidOffscreenView::createView()
{
	if (offscreen_view_ && !view_creation_requested_)
	{
		bool result = offscreen_view_->callBool("createView");
		if (result)
		{
			view_creation_requested_ = true;
			return true;
		}
		else
		{
			qCritical()<<"Call to createView failed!";
		}
	}
	else
	{
		qWarning()<<"Attempted to call QAndroidOffscreenView::createView() with offscreen view:"
			<<((offscreen_view_)?"not null":"null")<<"and creation_requested ="<<view_creation_requested_;
	}
	return false;
}

QAndroidOffscreenView::~QAndroidOffscreenView()
{
	deinitialize();
}

const QString & QAndroidOffscreenView::getDefaultJavaClassPath()
{
	return c_class_path_;
}

void QAndroidOffscreenView::preloadJavaClasses()
{
	QAndroidQPAPluginGap::preloadJavaClass("ru/dublgis/offscreenview/OffscreenView");
	QAndroidJniImagePair::preloadJavaClasses();
}

static int getApiLevel()
{
	try
	{
		return QJniObject("ru/dublgis/offscreenview/OffscreenView", false).callStaticInt("getApiLevel");
	}
	catch(QJniBaseException & e)
	{
		qCritical()<<"openGlTextureSupported exception:"<<e.what();
		return 0;
	}
}

bool QAndroidOffscreenView::openGlTextureSupported()
{
	return getApiLevel() >= 15; // Android 4.0.3+
}

bool QAndroidOffscreenView::nonAttachingModeSupported()
{
	return getApiLevel() >= 11; // Android 3.0+
}

void QAndroidOffscreenView::initializeGL()
{
	if (tex_.isAllocated() || bitmap_a_.isAllocated())
	{
		// qDebug("QAndroidOffscreenView GL is already initialized.");
		return;
	}

	QOpenGLTextureHolder::initializeGL();

	if (!openGlTextureSupported())
	{
		qDebug()<<__PRETTY_FUNCTION__<<"OpenGL mode is not supported on this device, will initialize for internal Bitmap mode.";
		initializeBitmap();
		return;
	}

	qDebug()<<__PRETTY_FUNCTION__;

	tex_.allocateTexture(GL_TEXTURE_EXTERNAL_OES);

	if (!offscreen_view_)
	{
		qWarning("Cannot initialize QAndroidOffscreenView because OffscreenView object was not created!");
		return;
	}

	qDebug()<<__PRETTY_FUNCTION__;

	// Check for max texture size
	GLint maxdims[2];
	GLint maxtextsz;
	glGetIntegerv(GL_MAX_VIEWPORT_DIMS, maxdims);
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxtextsz);
	int max_x = qMin(maxdims[0], maxtextsz);
	int max_y = qMin(maxdims[1], maxtextsz);
	QSize texture_size = QSize(qMin(max_x, size_.width()), qMin(max_y, size_.height()));
	tex_.setTextureSize(texture_size);
	qDebug()<<__PRETTY_FUNCTION__<<"GL_MAX_VIEWPORT_DIMS"<<maxdims[0]<<maxdims[1]
		<<"GL_MAX_TEXTURE_SIZE"<<maxtextsz
		<<"My size:"<<size_.width()<<"x"<<size_.height()
		<<"Resulting size:"<<texture_size.width()<<"x"<<texture_size.height();

	size_ = texture_size;

	offscreen_view_->callParamVoid("SetTexture", "I", jint(tex_.getTexture()));
	offscreen_view_->callParamVoid("SetInitialWidth", "I", jint(size_.width()));
	offscreen_view_->callParamVoid("SetInitialHeight", "I", jint(size_.height()));
	offscreen_view_->callVoid("initializeGL");
}

void QAndroidOffscreenView::initializeBitmap()
{
	QMutexLocker locker(&bitmaps_mutex_);
	if (tex_.isAllocated() || bitmap_a_.isAllocated())
	{
		return;
	}
	if (!offscreen_view_)
	{
		qWarning("Cannot initialize QAndroidOffscreenView because OffscreenView object was not created!");
		return;
	}
	qDebug()<<__PRETTY_FUNCTION__;
	bitmap_a_.resize(size_);
	bitmap_b_.resize(size_);
	last_qt_buffer_ = -1;
	offscreen_view_->callParamVoid("SetInitialWidth", "I", jint(size_.width()));
	offscreen_view_->callParamVoid("SetInitialHeight", "I", jint(size_.height()));
	offscreen_view_->callParamVoid("initializeBitmap",
		"Landroid/graphics/Bitmap;Landroid/graphics/Bitmap;",
		bitmap_a_.jbitmap(), bitmap_b_.jbitmap());
}

void QAndroidOffscreenView::deleteAndroidView()
{
	if (offscreen_view_)
	{
		offscreen_view_->callVoid("cppDestroyed");
		offscreen_view_.reset();
	}
}

void QAndroidOffscreenView::deinitialize()
{
	QMutexLocker locker(&bitmaps_mutex_);
	deleteAndroidView();
	tex_.deallocateTexture();
	bitmap_a_.dispose();
	bitmap_b_.dispose();
	last_qt_buffer_ = -1;
	android_to_qt_buffer_ = QImage();
}

static inline void clearGlRect(int l, int b, int w, int h, const QColor & fill_color_)
{
	glEnable(GL_SCISSOR_TEST);
	glScissor(l, b, w, h);
	glClearColor(fill_color_.redF(), fill_color_.greenF(), fill_color_.blueF(), fill_color_.alphaF());
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_SCISSOR_TEST);
}

void QAndroidOffscreenView::paintGL(int l, int b, int w, int h, bool reverse_y)
{
	glViewport(l, b, w, h);

	//
	// GL texture + GL in Qt
	//
	if (tex_.isAllocated() && view_painted_ && !bitmap_a_.isAllocated())
	{
		if (need_update_texture_)
		{
			bool texture_updated_ok = updateGlTexture();
			if (!texture_updated_ok && !texture_received_)
			{
				clearGlRect(l, b, w, h, fill_color_);
				return;
			}
		}
		if (tex_.getTextureSize().width() != last_texture_width_
			|| tex_.getTextureSize().height() != last_texture_height_)
		{
			// qDebug()<<__FUNCTION__<<"Last painted texture size doesn't match expected texture size.";
			clearGlRect(l, b, w, h, fill_color_);
			return;
		}
		tex_.blitTexture(
			QRect(QPoint(0, 0), QSize(w, h)) // target rect (relatively to viewport)
			, QRect(QPoint(0, 0), QSize(w, h)) // source rect (in texture)
			, reverse_y);
		return;
	}

	//
	// Bitmap texture + GL in Qt
	//
	if (bitmap_a_.isAllocated())
	{
		if (updateBitmapToGlTexture())
		{
			// We don't have to check for texture size match because Bitmaps are
			// destroyed and re-created on resize. (Also Java side checks for the match.)
			tex_.blitTexture(
				QRect(QPoint(0, 0), QSize(w, h)) // target rect (relatively to viewport)
				, QRect(QPoint(0, 0), QSize(w, h)) // source rect (in texture)
				, reverse_y);
			return;
		}
	}

	// View is not ready, just fill the area with the fill color.
	clearGlRect(l, b, w, h, fill_color_);
}

// Public version
const QImage * QAndroidOffscreenView::getBitmapBuffer(bool * out_texture_updated)
{
	return getBitmapBuffer(out_texture_updated, true);
}

const QImage * QAndroidOffscreenView::getPreviousBitmapBuffer(bool convert_from_android_format)
{
	if (!view_painted_)
	{
		return 0;
	}
	const QImage * result = 0;
	if (convert_from_android_format)
	{
		result = &android_to_qt_buffer_;
	}
	else
	{
		const QAndroidJniImagePair & pair = (last_qt_buffer_ == 0)? bitmap_a_: bitmap_b_;
		if (!pair.isAllocated())
		{
			return 0;
		}
		result = &pair.qImage();
	}
	if (result->isNull())
	{
		return 0;
	}
	return result;
}

// Protected version with some low-level functionality
const QImage * QAndroidOffscreenView::getBitmapBuffer(bool * out_texture_updated, bool convert_from_android_format)
{
	QMutexLocker locker(&bitmaps_mutex_);
	if (out_texture_updated)
	{
		*out_texture_updated = false;
	}
	if (bitmap_a_.isAllocated() && bitmap_b_.isAllocated()
		&& view_painted_ && offscreen_view_ && offscreen_view_->jObject())
	{
		if (need_update_texture_ ||
			(convert_from_android_format && (android_to_qt_buffer_.isNull() || android_to_qt_buffer_.size() != size())) ||
			(!convert_from_android_format && last_qt_buffer_ < 0))
		{
			need_update_texture_ = false;
			int buffer_index = offscreen_view_->callInt("getQtPaintingTexture");
			if (buffer_index < 0)
			{
				return getPreviousBitmapBuffer(convert_from_android_format);
			}
			if (out_texture_updated)
			{
				*out_texture_updated = true;
			}
			last_texture_width_ = offscreen_view_->callInt("getLastTextureWidth");
			last_texture_height_ = offscreen_view_->callInt("getLastTextureHeight");

			// Updating texture
			if (convert_from_android_format)
			{
				const QAndroidJniImagePair & pair = (buffer_index == 0)? bitmap_a_: bitmap_b_;
				pair.convert32BitImageFromAndroidToQt(android_to_qt_buffer_);
				return &android_to_qt_buffer_;
			}
			else
			{
				last_qt_buffer_ = buffer_index;
				return (buffer_index == 0)? &bitmap_a_.qImage(): &bitmap_b_.qImage();
			}
		}
		else
		{
			return getPreviousBitmapBuffer(convert_from_android_format);
		}
	}
	else
	{
		// qDebug()<<__PRETTY_FUNCTION__<<"Returning 0!";
		return 0;
	}
}

bool QAndroidOffscreenView::updateBitmapToGlTexture()
{
	QMutexLocker locker(&bitmaps_mutex_);
	bool updated_texture = true;
	const QImage * qtbuffer = getBitmapBuffer(&updated_texture, false);
	if (qtbuffer && !qtbuffer->isNull())
	{
		if (updated_texture || !tex_.isAllocated())
		{
			bool can_avoid_gl_conversion = (qtbuffer->format() == QImage::Format_ARGB32_Premultiplied);
			tex_.allocateTexture(*qtbuffer, can_avoid_gl_conversion, GL_RGBA, GL_TEXTURE_2D);
			if (can_avoid_gl_conversion)
			{
				// Fixing Y axis by setting this texture transformation.
				tex_.setTransformation(
					1.0f,  0.0f,
					0.0f, -1.0f,
					0, 0);
			}
		}
		return true; // Texture is correct
	}
	return false; // Texture contains no valid data
}

bool QAndroidOffscreenView::isCreated() const
{
	if (view_created_)
	{
		return true;
	}
	if (offscreen_view_)
	{
		bool result = offscreen_view_->callBool("isViewCreated");
		if (result)
		{
			view_created_ = true;
		}
		return result;
	}
	return false;
}

#if defined(QANDROIDOFFSCREENVIEW_ALLOWWAIT)
bool QAndroidOffscreenView::waitForViewCreation()
{
	if (isCreated())
	{
		return true;
	}
	if (!offscreen_view_)
	{
		qWarning("QAndroidOffscreenView: will not wait for View creation because OffscreenView is not initialized (yet?)");
		return false;
	}
	if (!view_creation_requested_)
	{
		qWarning("QAndroidOffscreenView: will not wait for View creation because the creation was not requested (yet?)");
		return false;
	}
	//! \todo: Use semaphore-based wait?
	qDebug()<<"QAndroidOffscreenView::waitForViewCreation"<<view_class_name_<<view_object_name_<<"tid ="<<gettid()<<">>>>>>";
	while (!isCreated())
	{
		usleep(5000); // 5 ms
		// QThread::yieldCurrentThread();		
	}
	qDebug()<<"QAndroidOffscreenView::waitForViewCreation"<<view_class_name_<<view_object_name_<<"tid ="<<gettid()<<"<<<<<<";
	return true;
}
#endif

bool QAndroidOffscreenView::hasValidImage() const
{
	return view_painted_ && texture_received_;
}

void QAndroidOffscreenView::invalidate()
{
	if (offscreen_view_)
	{
		offscreen_view_->callVoid("invalidateOffscreenView");
	}
}

void QAndroidOffscreenView::setFillColor(const QColor & color)
{
	if (color != fill_color_)
	{
		fill_color_ = color;
		if (offscreen_view_)
		{
			offscreen_view_->callParamVoid("setFillColor", "IIII",
				jint(fill_color_.alpha()),
				jint(fill_color_.red()),
				jint(fill_color_.green()),
				jint(fill_color_.blue()));
		}
		{
			QMutexLocker locker(&bitmaps_mutex_);
			if (bitmap_a_.isAllocated() && bitmap_b_.isAllocated())
			{
				bitmap_a_.fill(fill_color_, true);
				bitmap_b_.fill(fill_color_, true);
				need_update_texture_ = true;
				invalidate();
			}
		}
		if (!hasValidImage())
		{
			emit updated();
		}
	}
}

void QAndroidOffscreenView::setVisible(bool visible)
{
	if (visible != is_visible_)
	{
		is_visible_ = visible;
		if (offscreen_view_)
		{
			offscreen_view_->callVoid("setVisible", jboolean(is_visible_));
		}
	}
}

void QAndroidOffscreenView::setEnabled(bool enabled)
{
	if (enabled != is_enabled_)
	{
		is_enabled_ = enabled;
		if (offscreen_view_)
		{
			offscreen_view_->callVoid("setEnabled", jboolean(is_enabled_));
		}
	}
}

void QAndroidOffscreenView::setAttachingMode(bool attaching)
{
	if (offscreen_view_ && nonAttachingModeSupported())
	{
		offscreen_view_->callVoid("setAttachingMode", jboolean(attaching));
	}
}

/*bool QAndroidOffscreenView::isFocused() const
{
	if (offscreen_view_)
	{
		return offscreen_view_->callBool("isFocused");
	}
	return false;
}*/

void QAndroidOffscreenView::setFocused(bool focused)
{
	if (offscreen_view_)
	{
		return offscreen_view_->callVoid("setFocused", jboolean(focused));
	}
}

void QAndroidOffscreenView::setPosition(int left, int top)
{
	if (offscreen_view_)
	{
		return offscreen_view_->callParamVoid("setPosition", "II", jint(left), jint(top));
	}
}

void QAndroidOffscreenView::javaUpdate()
{
	// qDebug()<<__PRETTY_FUNCTION__<<view_object_name_;
	need_update_texture_ = true;
	view_painted_ = true;
	emit updated();
}

void QAndroidOffscreenView::javaViewCreated()
{
	view_created_ = true;
	emit viewCreated();
}

bool QAndroidOffscreenView::updateGlTexture()
{
	if (offscreen_view_)
	{
		// Get last View image into the texture.
		// If synchronized_texture_update_ is set, this will cause a wait on mutex if the view
		// is being painted. If synchronized_texture_update_ is false, then the texture update
		// will be skipped (we'll receive an update from Java later, after the frame is painted).
		bool success = offscreen_view_->callBool("updateTexture", synchronized_texture_update_);
		if (!success)
		{
			return false;
		}

		// Transform matrix
		float a11 = offscreen_view_->callFloat("getTextureTransformMatrix", 0);
		float a21 = offscreen_view_->callFloat("getTextureTransformMatrix", 1);
		float a12 = offscreen_view_->callFloat("getTextureTransformMatrix", 4);
		float a22 = offscreen_view_->callFloat("getTextureTransformMatrix", 5);
		float b1 = offscreen_view_->callFloat("getTextureTransformMatrix", 12);
		float b2 = offscreen_view_->callFloat("getTextureTransformMatrix", 13);
		tex_.setTransformation(a11, a12, a21, a22, b1, b2);

		// Last texture size
		last_texture_width_ = offscreen_view_->callInt("getLastTextureWidth");
		last_texture_height_ = offscreen_view_->callInt("getLastTextureHeight");

		/*
		qDebug()<<__PRETTY_FUNCTION__<<"Transform Matrix:\n"<<
				  __PRETTY_FUNCTION__<<"("<<a11<<a12<<") +"<<b1<<"\n"<<
				  __PRETTY_FUNCTION__<<"("<<a21<<a22<<") +"<<b2;
		*/

		need_update_texture_ = false;
		texture_received_ = true;
		return true;
	}
	else
	{
		qDebug()<<__PRETTY_FUNCTION__<<"Offscreen view does not exist (yet?)";
		return false;
	}
}

QJniObject * QAndroidOffscreenView::getView()
{
	if (offscreen_view_)
	{
		return offscreen_view_->callObject("getView", "android/view/View");
	}
	return 0;
}

void QAndroidOffscreenView::mouse(int android_action, int x, int y, long long timestamp_uptime_millis)
{
	if (offscreen_view_)
	{
		offscreen_view_->callParamVoid("ProcessMouseEvent", "IIIJ", jint(android_action), jint(x), jint(y), jlong(timestamp_uptime_millis));
	}
}

void QAndroidOffscreenView::resize(const QSize & size)
{
	if (size_ != size)
	{
		qDebug()<<__PRETTY_FUNCTION__<<"Old size:"<<size_<<"New size:"<<size;
		size_ = size;
		{
			QMutexLocker locker(&bitmaps_mutex_);
			if (bitmap_a_.isAllocated())
			{
				bitmap_a_.resize(size_);
				bitmap_b_.resize(size_);
				bitmap_a_.fill(fill_color_, true);
				bitmap_b_.fill(fill_color_, true);
				last_qt_buffer_ = -1;
				if (offscreen_view_)
				{
					offscreen_view_->callParamVoid("setBitmaps",
						"Landroid/graphics/Bitmap;Landroid/graphics/Bitmap;",
						bitmap_a_.jbitmap(), bitmap_b_.jbitmap());
					//QMetaObject::invokeMethod(this, "invalidate", Qt::QueuedConnection);
				}
			}
		}
		if (offscreen_view_)
		{
			offscreen_view_->callParamVoid("resizeOffscreenView", "II", jint(size.width()), jint(size.height()));
		}
		tex_.setTextureSize(size);
	}
}

