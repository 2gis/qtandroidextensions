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
#include <QtCore/QThread>
#include <QtCore/QTimer>
#include <QtCore/QMutexLocker>
#include <QtCore/QCoreApplication>
#include <QJniHelpers/QAndroidQPAPluginGap.h>
#include "QAndroidJniImagePair.h"
#include "QAndroidOffscreenView.h"

// Set in QAndroidOffsceenView::initializeGL.
static bool s_have_to_adjust_size_to_pot = true;
static QSize s_max_gl_size;

//! Calculate smallest power of 2 which is greater than x.
static int potSize(int x, int max_possible)
{
	int p = 1;
	while (p < x)
	{
		p <<= 1;
		if (p >= max_possible)
		{
			return max_possible;
		}
	}
	return p;
}

//! Calculate smallest power of 2 size which is greater than the size.
static QSize potSize(const QSize & size, const QSize & maxsize)
{
	QSize result;
	if (!maxsize.isEmpty())
	{
		result = QSize(potSize(size.width(), maxsize.width()), potSize(size.height(), maxsize.height()));
	}
	else
	{
		result = QSize(potSize(size.width(), 0x40000000), potSize(size.height(), 0x40000000));
	}
	// qDebug()<<__FUNCTION__<<"In:"<<size<<"Max:"<<maxsize<<"Result:"<<result;
	return result;
}

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

Q_DECL_EXPORT void JNICALL Java_OffscreenView_onVisibleRect(JNIEnv *, jobject, jlong param, int left, int top, int right, int bottom)
{
	if (param)
	{
		void * vp = reinterpret_cast<void*>(param);
		QAndroidOffscreenView * proxy = reinterpret_cast<QAndroidOffscreenView*>(vp);
		if (proxy)
		{
			proxy->javaVisibleRectReceived(left, top, right, bottom);
			return;
		}
	}
	qWarning()<<__FUNCTION__<<"Zero param!";
}


QAndroidOffscreenView::QAndroidOffscreenView(
	const QString & classname
	, const QString & objectname
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
	, view_creation_requested_(false)
	, is_visible_(false)
	, is_enabled_(true)
	, view_created_(false)
	, last_texture_width_(0)
	, last_texture_height_(0)
{
	connect(
		QApplicationActivityObserver::instance(),
		SIGNAL(applicationActiveStateChanged()),
		this,
		SLOT(applicationActivityStatusChanged()),
		Qt::DirectConnection);

	preloadJavaClasses();

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
	offscreen_view_.reset(new QJniObject(view_class_name_.toLatin1().data(), ""));

	if (offscreen_view_ && offscreen_view_->jObject())
	{
		offscreen_view_->callVoid("SetObjectName", view_object_name_);
		offscreen_view_->callParamVoid("SetNativePtr", "J", jlong(reinterpret_cast<void*>(this)));
		offscreen_view_->callParamVoid("setFillColor", "IIII",
			jint(fill_color_.alpha()), jint(fill_color_.red()), jint(fill_color_.green()), jint(fill_color_.blue()));

		// Our descendant constructors may want to register natives before createView is actually
		// called, so let's invoke it through the message queue rather than calling directly.
		QMetaObject::invokeMethod(this, "createView", Qt::QueuedConnection);
	}
	else
	{
		qCritical()<<"Failed to create View:"<<view_class_name_<<"/"<<view_object_name_
			<<"Please make sure that all Java classes are present in the project, and also that the Java class is pre-loaded.";
		offscreen_view_.reset();
		return;
	}
}

void QAndroidOffscreenView::createView()
{
	if (offscreen_view_ && !view_creation_requested_)
	{
		bool result = offscreen_view_->callBool("createView");
		if (result)
		{
			view_creation_requested_ = true;
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
	static volatile bool preloaded_ = false;

	if (!preloaded_)
	{
		preloaded_ = true;

		QApplicationActivityObserver::instance();

		QAndroidQPAPluginGap::preloadJavaClasses();
		QAndroidQPAPluginGap::preloadJavaClass("ru/dublgis/offscreenview/OffscreenView");
		QAndroidJniImagePair::preloadJavaClasses();

		QJniClass ov("ru/dublgis/offscreenview/OffscreenView");
		static const JNINativeMethod methods[] = {
			{"nativeUpdate", "(J)V", reinterpret_cast<void*>(Java_OffscreenView_nativeUpdate)},
			{"nativeViewCreated", "(J)V", reinterpret_cast<void*>(Java_OffscreenView_nativeViewCreated)},
			{"getActivity", "()Landroid/app/Activity;", reinterpret_cast<void*>(QAndroidQPAPluginGap::getActivityNoThrow)},
			{"nativeOnVisibleRect", "(JIIII)V", reinterpret_cast<void*>(Java_OffscreenView_onVisibleRect)},
		};
		ov.registerNativeMethods(methods, sizeof(methods));
	}
}

static int getApiLevel()
{
	try
	{
		return QJniClass("ru/dublgis/offscreenview/OffscreenView").callStaticInt("getApiLevel");
	}
	catch(QJniBaseException & e)
	{
		qCritical()<<"getApiLevel exception:"<<e.what();
		return 0; // Unknown API level, app should assume the lowest possible level.
	}
}

bool QAndroidOffscreenView::openGlTextureSupportedOnJavaSide()
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

	static bool s_capabilities_checked = false;
	static bool s_gl_texture_mode_supported = false;
	static bool s_blacklisted_renderer = false;

	if (!s_capabilities_checked)
	{
		// Check that we have enough API level
		s_gl_texture_mode_supported = openGlTextureSupportedOnJavaSide();

		// Some older devices don't support non-power of 2 OES textures.
		// (IMG textures seem to be supported by all GLES2 devices, but not OES.)
		// We have to detect that and keep that in mind.
		{
			QByteArray extensions(reinterpret_cast<const char *>(glGetString(GL_EXTENSIONS)));
			s_have_to_adjust_size_to_pot = !extensions.contains("GL_OES_texture_npot");
			qDebug()<<((!s_have_to_adjust_size_to_pot)?
				"GL_OES_texture_npot is available." :
				"GL_OES_texture_npot extension not found, cannot use full GL mode.");
		}

		// Checking blacklisted video drivers
		const char * renderer_string = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
		if (renderer_string)
		{
			static const char * const bad[] =
			{
				"Adreno 200",
				"Adreno (TM) 200",
				"Adreno 203",
				"Adreno (TM) 203",
				"Adreno 205",
				"Adreno (TM) 205",
				"VideoCore IV",
				"VideoCore IV HW",
				0
			};
			for(const char * const * p = bad; *p; ++p)
			{
				if (!strcmp(renderer_string, *p))
				{
					s_blacklisted_renderer = true;
					break;
				}
			}
		}

		// Detecting max possible size of texture.
		{
			GLint maxdims[2] = {0, 0};
			GLint maxtextsz = 0;
			glGetIntegerv(GL_MAX_VIEWPORT_DIMS, maxdims);
			glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxtextsz);
			s_max_gl_size = QSize(qMin(maxdims[0], maxtextsz), qMin(maxdims[1], maxtextsz));
		}

		qDebug()<<"OffscreenView's OpenGL check result: renderer ="<<((renderer_string)?renderer_string:"!!! EMPTY !!!")
			<<"Blacklist:"<<s_blacklisted_renderer<<"API support:"<<s_gl_texture_mode_supported
			<<"Max dimensions:"<<s_max_gl_size;

		s_capabilities_checked = true;
	}

	// Good time to compile shaders. We don't need to compile GL_TEXTURE_EXTERNAL_OES
	// shaders if we are not working in full GL mode.
	QOpenGLTextureHolder::initializeGL(s_gl_texture_mode_supported);

	// Automatically fall back to Bitmap + GL mode if pure GL is not available.
	// Note: we could also support s_have_to_adjust_size_to_pot in pure GL, but it doesn't seem
	// necessary as pure GL requires API >= 15 which means newer devices and newer drivers,
	// so such situation doesn't seem probable.
	if (!s_gl_texture_mode_supported || s_have_to_adjust_size_to_pot || s_blacklisted_renderer)
	{
		qDebug()<<__PRETTY_FUNCTION__<<"OpenGL mode is not supported on this device, will initialize for internal Bitmap mode.";
		initializeBitmap();
	}
	else
	{
		tex_.allocateTexture(GL_TEXTURE_EXTERNAL_OES);

		if (!offscreen_view_)
		{
			qWarning("Cannot initialize QAndroidOffscreenView because OffscreenView object was not created!");
			return;
		}

		// Check for max texture size and limit control size
		size_ = QSize(qMin(s_max_gl_size.width(), size_.width()), qMin(s_max_gl_size.height(), size_.height()));
		tex_.setTextureSize(size_);

		offscreen_view_->callParamVoid("SetTexture", "I", jint(tex_.getTexture()));
		offscreen_view_->callParamVoid("SetInitialWidth", "I", jint(size_.width()));
		offscreen_view_->callParamVoid("SetInitialHeight", "I", jint(size_.height()));
		offscreen_view_->callVoid("initializeGL");
	}
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
	// qDebug()<<__PRETTY_FUNCTION__;
	QSize bitmapsize = (s_have_to_adjust_size_to_pot)? potSize(size_, s_max_gl_size): size_;
	bitmap_a_.resize(bitmapsize);
	bitmap_b_.resize(bitmapsize);
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
	if (updateGLTextureInHolder())
	{
		tex_.blitTexture(
			QRect(QPoint(0, 0), QSize(w, h)) // target rect (relatively to viewport)
			, QRect(QPoint(0, 0), QSize(w, h)) // source rect (in texture)
			, reverse_y);
	}
	else
	{
		// View is not ready, just fill the area with the fill color.
		clearGlRect(l, b, w, h, fill_color_);
	}
}

bool QAndroidOffscreenView::updateGLTextureInHolder()
{
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
				return false;
			}
			if (tex_.getTextureSize().width() != last_texture_width_
				|| tex_.getTextureSize().height() != last_texture_height_)
			{
				return false;
			}
		}
		return true;
	}
	//
	// Bitmap texture + GL in Qt
	//
	if (bitmap_a_.isAllocated())
	{
		return updateBitmapToGlTexture();
	}
	return false;
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
		return 0; // No previous buffer
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
			return 0; // No previous buffer
		}
		result = &pair.qImage();
	}
	if (result->isNull())
	{
		return 0; // No previous buffer
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
		return 0;  // No buffer created
	}
}

bool QAndroidOffscreenView::updateBitmapToGlTexture()
{
	QMutexLocker locker(&bitmaps_mutex_);
	bool updated_texture = true;
	// Get bitmap buffer in Android format (for 32 bits it is ABGR (in Qt) aka RGBA (in Android)).
	// We don't need conversion to Qt format because GL can hangle Android formats directly.
	const QImage * qtbuffer = getBitmapBuffer(&updated_texture, false);
	if (qtbuffer && !qtbuffer->isNull())
	{
		if (updated_texture || !tex_.isAllocated())
		{
			tex_.allocateTexture(*qtbuffer, true);
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

void QAndroidOffscreenView::applicationActivityStatusChanged()
{
	updateAndroidViewVisibility();
}

void QAndroidOffscreenView::updateAndroidViewVisibility()
{
	if (offscreen_view_)
	{
		bool vis = is_visible_ && QApplicationActivityObserver::instance()->isApplicationActive();
		// qDebug()<<__FUNCTION__<<viewObjectName()<<"Visible:"<<is_visible_<<"AppActive:"<<QApplicationActivityObserver::instance()->isApplicationActive()<<"Set:"<<vis;
		offscreen_view_->callVoid("setVisible", jboolean(vis));
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
	is_visible_ = visible;
	updateAndroidViewVisibility();
}

void QAndroidOffscreenView::setEnabled(bool enabled)
{
	// NB: we always call Java setEnabled!
	is_enabled_ = enabled;
	if (offscreen_view_)
	{
		offscreen_view_->callVoid("setEnabled", jboolean(is_enabled_));
	}
}

void QAndroidOffscreenView::setAttachingMode(bool attaching)
{
	if (!nonAttachingModeSupported())
	{
		qDebug()<<__FUNCTION__<<"Ignoring ("<<attaching<<") because non-attaching mode is not supported on this device";
		return;
	}
	if (offscreen_view_)
	{
		offscreen_view_->callVoid("setAttachingMode", jboolean(attaching));
	}
}

void QAndroidOffscreenView::reattachView()
{
	if (offscreen_view_)
	{
		offscreen_view_->callVoid("reattachView");
		setVisible(visible());
		setEnabled(enabled());
		invalidate();
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

void QAndroidOffscreenView::hideKeyboard()
{
	if (offscreen_view_)
	{
		offscreen_view_->callVoid("hideKeyboard");
	}
}

void QAndroidOffscreenView::showKeyboard()
{
	if (offscreen_view_)
	{
		offscreen_view_->callVoid("showKeyboard");
	}
}

void QAndroidOffscreenView::setHideKeyboardOnFocusLoss(bool hide)
{
	if (offscreen_view_)
	{
		offscreen_view_->callVoid("setHideKeyboardOnFocusLoss", jboolean(hide));
	}
}

void QAndroidOffscreenView::setShowKeyboardOnFocusIn(bool show)
{
	if (offscreen_view_)
	{
		offscreen_view_->callVoid("setShowKeyboardOnFocusIn", jboolean(show));
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

void QAndroidOffscreenView::javaVisibleRectReceived(int left, int top, int right, int bottom)
{
	int width = right - left, height = bottom - top;
	// qDebug()<<viewObjectName()<<__FUNCTION__<<left<<top<<right<<bottom<<"W:"<<width<<"H:"<<height;
	emit visibleRectReceived(width, height);
}

bool QAndroidOffscreenView::updateGlTexture()
{
	if (offscreen_view_)
	{
		// Get last View image into the texture.
		if (!offscreen_view_->callBool("updateTexture"))
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
	return 0; // View not created yet
}

void QAndroidOffscreenView::mouse(int android_action, int x, int y, long long timestamp_uptime_millis)
{
	if (offscreen_view_)
	{
		offscreen_view_->callParamVoid("ProcessMouseEvent", "IIIJ", jint(android_action), jint(x), jint(y), jlong(timestamp_uptime_millis));
	}
}

void QAndroidOffscreenView::requestVisibleRect()
{
	if (offscreen_view_)
	{
		offscreen_view_->callVoid("queryVisibleRect");
	}
}

int QAndroidOffscreenView::getScrollX()
{
	if (offscreen_view_)
	{
		return offscreen_view_->callInt("getScrollX");
	}
	return 0; // No scroll yet
}

int QAndroidOffscreenView::getScrollY()
{
	if (offscreen_view_)
	{
		return offscreen_view_->callInt("getScrollY");
	}
	return 0; // No scroll yet
}

void QAndroidOffscreenView::setScrollX(int x)
{
	if (offscreen_view_)
	{
		offscreen_view_->callVoid("setScrollX", static_cast<jint>(x));
	}
}

void QAndroidOffscreenView::setScrollY(int y)
{
	if (offscreen_view_)
	{
		offscreen_view_->callVoid("setScrollY", static_cast<jint>(y));
	}
}

int QAndroidOffscreenView::getMeasuredWidth()
{
	if (offscreen_view_)
	{
		return offscreen_view_->callInt("getMeasuredWidth");
	}
	return -1;
}

int QAndroidOffscreenView::getMeasuredHeight()
{
	if (offscreen_view_)
	{
		return offscreen_view_->callInt("getMeasuredHeight");
	}
	return -1;
}

void QAndroidOffscreenView::resize(const QSize & newsize)
{
	QSize size = newsize;
	if (!s_max_gl_size.isEmpty())
	{
		size = QSize(qMin(size.width(), s_max_gl_size.width()), qMin(size.height(), s_max_gl_size.height()));
	}

	if (size_ != size)
	{
		qDebug()<<__PRETTY_FUNCTION__<<"Old size:"<<size_<<"New size:"<<size;
		size_ = size;
		{
			QMutexLocker locker(&bitmaps_mutex_);
			if (bitmap_a_.isAllocated())
			{
				QSize bitmapsize = (s_have_to_adjust_size_to_pot)? potSize(size_, s_max_gl_size): size_;
				bitmap_a_.resize(bitmapsize);
				bitmap_b_.resize(bitmapsize);
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
			// The view texture is now contains wrongly sized image and should not be used
			// until the view is painted again because it will look distorted.
			view_painted_ = false;
			offscreen_view_->callParamVoid("resizeOffscreenView", "II", jint(size.width()), jint(size.height()));
		}
		tex_.setTextureSize(size);
	}
}

void QAndroidOffscreenView::setSoftInputModeResize()
{
	if (offscreen_view_)
	{
		offscreen_view_->callVoid("setSoftInputModeResize");
	}
}

void QAndroidOffscreenView::setSoftInputModeAdjustPan()
{
	if (offscreen_view_)
	{
		offscreen_view_->callVoid("setSoftInputModeAdjustPan");
	}
}

void QAndroidOffscreenView::setSoftInputModeAdjustNothing()
{
	if (offscreen_view_)
	{
		offscreen_view_->callVoid("setSoftInputModeAdjustNothing");
	}
}

void QAndroidOffscreenView::testFunction()
{
	if (offscreen_view_)
	{
		offscreen_view_->callVoid("testFunction");
	}
}


int QColorToAndroidColor(const QColor & color)
{
	// QColor to BGRA aka ARGB in Android terms
	return color.blue() | (color.green()<<8) | (color.red()<<16) | (color.alpha()<<24);
}
