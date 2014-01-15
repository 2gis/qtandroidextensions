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
#include <QMutexLocker>
#include "QAndroidQPAPluginGap.h"
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
			QMetaObject::invokeMethod(proxy, "javaUpdate", Qt::QueuedConnection);
			return;
		}
	}
	qWarning()<<__FUNCTION__<<"Zero param!";
}

QAndroidOffscreenView::QAndroidOffscreenView(const QString & classname, const QString & objectname, bool waitforcreation, const QSize & defsize, QObject * parent)
	: QObject(parent)
	, view_class_name_(classname)
	, view_object_name_(objectname)
	, size_(defsize)
	, fill_color_(Qt::white)
	, need_update_texture_(false)
	, view_painted_(false)
	, texture_received_(false)
	, synchronized_texture_update_(true)
{
	setObjectName(objectname);

	// Expand OffscreenWebView => ru/dublgis/offscreenview/OffscreenWebView
	if (!view_class_name_.contains('/'))
	{
		view_class_name_.prepend(c_class_path_);
	}

	qDebug()<<__PRETTY_FUNCTION__<<"Creating object of"<<view_class_name_;
	offscreen_view_.reset(new jcGeneric(
		view_class_name_.toAscii()
		, true // Keep ownership
	));

	if (offscreen_view_ && offscreen_view_->jObject())
	{
		offscreen_view_->CallVoid("SetObjectName", view_object_name_);
		offscreen_view_->CallParamVoid("SetNativePtr", "J", jlong(reinterpret_cast<void*>(this)));

		offscreen_view_->RegisterNativeMethod("nativeUpdate", "(J)V", (void*)Java_OffscreenView_nativeUpdate);
		offscreen_view_->RegisterNativeMethod("nativeGetActivity", "()Landroid/app/Activity;", (void*)QAndroidQPAPluginGap::getActivity);

		// Invoke creation of the view, so its functions will be available
		// before initialization of GL part.
		offscreen_view_->CallVoid("createView");
	}
	else
	{
		qCritical()<<"Failed to create View:"<<view_class_name_<<"/"<<view_object_name_;
		offscreen_view_.reset();
		return;
	}

	if (waitforcreation)
	{
		waitForViewCreation();
	}
}

QAndroidOffscreenView::~QAndroidOffscreenView()
{
	deinitialize();
}

void QAndroidOffscreenView::initializeGL()
{
	qDebug()<<__PRETTY_FUNCTION__;
	if (tex_.isAllocated())
	{
		qWarning("Attempting to initialize QAndroidOffscreenView second time!");
		return;
	}

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

	qDebug()<<"waitForViewCreation >>";
	waitForViewCreation();
	qDebug()<<"<< waitForViewCreation";

	offscreen_view_->CallParamVoid("SetTexture", "I", jint(tex_.getTexture()));
	offscreen_view_->CallParamVoid("SetInitialWidth", "I", jint(size_.width()));
	offscreen_view_->CallParamVoid("SetInitialHeight", "I", jint(size_.height()));
	offscreen_view_->CallVoid("initializeGL");
}

void QAndroidOffscreenView::deleteAndroidView()
{
	if (offscreen_view_)
	{
		offscreen_view_->CallVoid("cppDestroyed");
		offscreen_view_.reset();
	}
}

void QAndroidOffscreenView::deinitialize()
{
	deleteAndroidView();
	tex_.deallocateTexture();
}

static inline void clearGlRect(int l, int b, int w, int h, const QColor & fill_color_)
{
	glEnable(GL_SCISSOR_TEST);
	glScissor(l, b, w, h);
	glClearColor(fill_color_.redF(), fill_color_.greenF(), fill_color_.blueF(), fill_color_.alphaF());
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_SCISSOR_TEST);
}

void QAndroidOffscreenView::paintGL(int l, int b, int w, int h)
{
	glViewport(l, b, w, h);
	if (tex_.isAllocated() && view_painted_)
	{
		if (need_update_texture_)
		{
			bool texture_updated_ok = updateTexture();
			if (!texture_updated_ok && !texture_received_)
			{
				clearGlRect(l, b, w, h, fill_color_);
				return;
			}
		}
		tex_.blitTexture(
			QRect(QPoint(0, 0), QSize(w, h)) // target rect (relatively to viewport)
			, QRect(QPoint(0, 0), QSize(w, h))); // source rect (in texture)
		return;
	}
	clearGlRect(l, b, w, h, fill_color_);
}

bool QAndroidOffscreenView::isCreated() const
{
	if (offscreen_view_)
	{
		return offscreen_view_->CallBool("isViewCreated");
	}
	return false;
}

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
	//! \todo: Use semaphore-based wait?
	while (!isCreated())
	{
		usleep(5000); // 5 ms
		// QThread::yieldCurrentThread();
	}
	return true;
}

bool QAndroidOffscreenView::hasValidImage() const
{
	return view_painted_ && texture_received_;
}

void QAndroidOffscreenView::invalidate()
{
	if (offscreen_view_)
	{
		offscreen_view_->CallVoid("invalidateOffscreenView");
	}
}

void QAndroidOffscreenView::setFillColor(const QColor & color)
{
	if (color != fill_color_)
	{
		fill_color_ = color;
		if (!hasValidImage())
		{
			emit updated();
		}
	}
}

void QAndroidOffscreenView::javaUpdate()
{
	qDebug()<<__PRETTY_FUNCTION__;
	need_update_texture_ = true;
	view_painted_ = true;
	emit updated();
}

bool QAndroidOffscreenView::updateTexture()
{
	if (offscreen_view_)
	{
		// Get last View image into the texture.
		// If synchronized_texture_update_ is set, this will cause a wait on mutex if the view
		// is being painted. If synchronized_texture_update_ is false, then the texture update
		// will be skipped (we'll receive an update from Java later, after the frame is painted).
		bool success = offscreen_view_->CallBool("updateTexture", synchronized_texture_update_);
		if (!success)
		{
			return false;
		}

		// Transform matrix
		float a11 = offscreen_view_->CallFloat("getTextureTransformMatrix", 0);
		float a21 = offscreen_view_->CallFloat("getTextureTransformMatrix", 1);
		float a12 = offscreen_view_->CallFloat("getTextureTransformMatrix", 4);
		float a22 = offscreen_view_->CallFloat("getTextureTransformMatrix", 5);
		float b1 = offscreen_view_->CallFloat("getTextureTransformMatrix", 12);
		float b2 = offscreen_view_->CallFloat("getTextureTransformMatrix", 13);
		tex_.setTransformation(a11, a12, a21, a22, b1, b2);

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

void QAndroidOffscreenView::mouse(int android_action, int x, int y)
{
	if (offscreen_view_)
	{
		offscreen_view_->CallParamVoid("ProcessMouseEvent", "III", jint(android_action), jint(x), jint(y));
	}
}

void QAndroidOffscreenView::resize(const QSize & size)
{
	if (size_ != size)
	{
		qDebug()<<__PRETTY_FUNCTION__<<"Old size:"<<size_<<"New size:"<<size_;
		size_ = size;
		if (offscreen_view_)
		{
			offscreen_view_->CallParamVoid("resizeOffscreenView", "II", jint(size.width()), jint(size.height()));
			tex_.setTextureSize(size);
		}
	}
}

