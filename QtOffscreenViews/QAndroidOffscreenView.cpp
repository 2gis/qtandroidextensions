#include <unistd.h>
#include <stdlib.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include "QAndroidOffscreenView.h"

static const QString c_class_path_(QLatin1String("ru/dublgis/offscreenview"));

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

QAndroidOffscreenView::QAndroidOffscreenView(const QString & classname, const QString & objectname, const QSize & defsize, QObject * parent)
	: QObject(parent)
	, view_class_name_(classname)
	, view_object_name_(objectname)
	, size_(defsize)
	, fill_color_(Qt::white)
	, need_update_texture_(false)
	, view_painted_(false)
{
	setObjectName(objectname);
	offscreen_view_factory_.reset(new jcGeneric((c_class_path_+"/OffscreenViewFactory").toAscii(), true));
}

QAndroidOffscreenView::~QAndroidOffscreenView()
{
	deinitialize();
}

/*!
 * Initialize Android view. This function should be called within active proper GL context
 * as it may want to create OpenGL texture.
 */
void QAndroidOffscreenView::initializeGL()
{
	if (tex_.isAllocated())
	{
		qWarning("Attempting to initialize QAndroidOffscreenView second time!");
		return;
	}

	tex_.allocateTexture(GL_TEXTURE_EXTERNAL_OES);

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

	if (offscreen_view_factory_)
	{
		offscreen_view_factory_->CallVoid("SetClassName", view_class_name_);
		offscreen_view_factory_->CallVoid("SetObjectName", view_object_name_);
		offscreen_view_factory_->CallParamVoid("SetTexture", "I", jint(tex_.getTexture()));
		offscreen_view_factory_->CallParamVoid("SetTextureWidth",	"I", jint(texture_size.width()));
		offscreen_view_factory_->CallParamVoid("SetTextureHeight", "I", jint(texture_size.height()));
		offscreen_view_factory_->CallParamVoid("SetNativePtr", "J", jlong(reinterpret_cast<void*>(this)));
		offscreen_view_.reset(
			offscreen_view_factory_->CallObject("DoCreateView"
			, (c_class_path_+"/OffscreenView").toAscii()));
		if (offscreen_view_->jObject() == 0)
		{
			qCritical()<<"Failed to create View:"<<view_class_name_<<"/"<<view_object_name_;
			offscreen_view_.reset();
			return;
		}
		offscreen_view_->RegisterNativeMethod(
			"nativeUpdate"
			, "(J)V"
			, (void*)Java_OffscreenView_nativeUpdate);
	}
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
	offscreen_view_factory_.reset();
	tex_.deallocateTexture();
}

static void clearGlRect(int l, int b, int w, int h, const QColor & fill_color_)
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
			if (!updateTexture())
			{
				clearGlRect(l, b, w, h, fill_color_);
				return;
			}
		}

		//! \todo check source rect calculations!
		tex_.blitTexture(
			QRect(QPoint(0, 0), QSize(w, h)) // target rect (relatively to viewport)
			, QRect(QPoint(0, 0), QSize(w, h))); // source rect (in texture)
		return;
	}
	clearGlRect(l, b, w, h, fill_color_);
}

bool QAndroidOffscreenView::isCreated() const
{
	//! \todo
	return true;
}

bool QAndroidOffscreenView::hasValidImage() const
{
	//! \todo
	return true;
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
		// NB: this may cause a wait on mutex if the view is being painted
		//! \todo Avoid waiting on mutex (optionally?)
		bool success = offscreen_view_->CallBool("updateTexture");
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

		need_update_texture_ = false;
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
		}
	}
}

