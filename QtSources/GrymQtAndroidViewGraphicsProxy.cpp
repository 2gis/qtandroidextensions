#include <jni.h>
#include <unistd.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <stdlib.h>
#include <QtOpenGL>
#include <QGLWidget>
#include "GrymQtAndroidViewGraphicsProxy.h"

#define ANDROIDVIEWGRAPHICSPROXY_CLEARALL
// #define ANDROIDVIEWGRAPHICSPROXY_TEST_TEXTURE
#define ANDROIDVIEWGRAPHICSPROXY_GREEN_FILL
#define ANDROIDVIEWGRAPHICSPROXY_READ_ONLY_XY_TRANSFORM

static const QString c_class_path_ = QLatin1String("ru/dublgis/offscreenview");

Q_DECL_EXPORT void JNICALL Java_OffscreenView_nativeUpdate(JNIEnv *, jobject, jlong param)
{
	if (param)
	{
		void * vp = reinterpret_cast<void*>(param);
		GrymQtAndroidViewGraphicsProxy * proxy = reinterpret_cast<GrymQtAndroidViewGraphicsProxy*>(vp);
		if (proxy)
		{
			QMetaObject::invokeMethod(proxy, "javaUpdate", Qt::QueuedConnection);
			return;
		}
	}
	qWarning()<<__FUNCTION__<<"Zero param!";
}

GrymQtAndroidViewGraphicsProxy::GrymQtAndroidViewGraphicsProxy(QGraphicsItem *parent, Qt::WindowFlags wFlags)
	: QGraphicsWidget(parent, wFlags)
	, need_update_texture_(false)
	, mouse_tracking_(false)
{
	setAcceptedMouseButtons(Qt::LeftButton);
	qDebug()<<__PRETTY_FUNCTION__<<"tid"<<gettid();
	offscreen_view_factory_.reset(new jcGeneric((c_class_path_+"/OffscreenViewFactory").toAscii(), true));
	qDebug()<<__PRETTY_FUNCTION__<<"Connected to OffscreenViewFactory.";
	qDebug()<<"Done"<<__PRETTY_FUNCTION__;
}

GrymQtAndroidViewGraphicsProxy::~GrymQtAndroidViewGraphicsProxy()
{
	// Разрушим Java-объекты первым делом
	if (offscreen_view_)
	{
		offscreen_view_->CallVoid("cppDestroyed");
		offscreen_view_.reset();
	}
	offscreen_view_factory_.reset();
	tex_.deallocateTexture();
}

void GrymQtAndroidViewGraphicsProxy::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
	Q_UNUSED(option);
	Q_UNUSED(widget);
	Q_ASSERT(painter->paintEngine()->type() == QPaintEngine::OpenGL2);

	// Создадим текстуру
	initTexture();
	if (need_update_texture_)
	{
		if (!updateTexture())
		{
			// verbose on start qDebug()<<__PRETTY_FUNCTION__<<"Texture is not ready ret.";
			painter->fillRect(rect(), Qt::white);
			return;
		}
	}

	#if defined(ANDROIDVIEWGRAPHICSPROXY_CLEARALL)
		painter->fillRect(rect(), Qt::green);
	#endif

	painter->beginNativePainting();

	QPaintDevice *device = painter->device();
	QTransform combined_transform = painter->combinedTransform();

	Q_ASSERT(
		(combined_transform.type() == QTransform::TxNone) ||
		(combined_transform.type() == QTransform::TxTranslate) ||
		(combined_transform.type() == QTransform::TxScale));

	QRectF widget_geometry = combined_transform.mapRect(QRectF(QPointF(0, 0), size()));

	GLint l = static_cast<GLint>(widget_geometry.left() + 0.5);
	GLint b = static_cast<GLint>(device->height() - static_cast<int>(widget_geometry.bottom() + 0.5));
	GLsizei w = static_cast<GLsizei>(widget_geometry.width()+0.5);
	GLsizei h = static_cast<GLsizei>(widget_geometry.height()+0.5);

	QPoint viewpos = this->scene()->views().at(0)->pos();
	Q_UNUSED(viewpos);

#if 0
	qDebug()
			<<__PRETTY_FUNCTION__
			<<"tid"<<gettid()
			<<"widget_geometry:"<<widget_geometry.left()<<widget_geometry.top()
			<<"("<<widget_geometry.width()<<"x"<<widget_geometry.height()<<")"
			<<"right"<<widget_geometry.right()<<"bottom"<<widget_geometry.bottom()
			<<"//////// l"<<l<<"b"<<b<<"w"<<w<<"h"<<h
			<<"widget_pos"<<widget->pos().x()<<widget->pos().y()
			<<"viewpos"<<viewpos.x()<<viewpos.y();
#endif

	// SGEXP
	l += widget->pos().x();
	b += widget->pos().y();
	//! \todo Учесть положение вьюшки внутри окна!
	w++;
	h++;

	glViewport(l, b, w, h);

	#if defined(ANDROIDVIEWGRAPHICSPROXY_CLEARALL)
		glEnable(GL_SCISSOR_TEST);
		glScissor(l, b, w, h);

		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	#endif

	// Теперь можно просто нарисовать нашу текстурку вот в этих вот GL-координатах.
	doGLPainting(l, b, w, h);

	#if defined(ANDROIDVIEWGRAPHICSPROXY_CLEARALL)
		glScissor(0, 0, static_cast<GLsizei>(device->width()), static_cast<GLsizei>(device->height()));
		glDisable(GL_SCISSOR_TEST);
	#endif

	// Возвращаем viewport в исходное состояние
	glViewport(0, 0, static_cast<GLsizei>(device->width()), static_cast<GLsizei>(device->height()));
	painter->endNativePainting();
}

void GrymQtAndroidViewGraphicsProxy::javaUpdate()
{
	qDebug()<<__PRETTY_FUNCTION__;
	need_update_texture_ = true;
	update();
}

/*!
 * \param Координаты передаются в терминах OpenGL.
 */
void GrymQtAndroidViewGraphicsProxy::doGLPainting(int x, int y, int w, int h)
{
	// (GLuint texture, const QSize &viewport, const QSize &texSize, const QRect &targetRect, const QRect &sourceRect)
	QSize draw_size = getDrawableSize();
	// Пока считаем, что w, h у нас совпадают с размерами виджета, значит, в getDrawableSize всё учтено
	Q_UNUSED(w);
	Q_UNUSED(h);
	// Прямоугольник в координатах OpenGL
	QRect target_rect(
		x
		, y + h - draw_size.height()
		, draw_size.width()
		, draw_size.height());
	// verbose qDebug()<<"tid"<<gettid()<<"Mapped to parent:"<<target_rect.left()<<","<<target_rect.top()
	// <<"("<<target_rect.width()<<","<<target_rect.height()<<")";
	glViewport(target_rect.x(), target_rect.y(), target_rect.width(), target_rect.height());

	// Debug
	#if 0 // !defined(QT_OPENGL_ES_2)
		GLuint width = 0, height = 0;
		glBindTexture(texture_type_, texture_id_);
		glGetTexLevelParameteriv(texture_type_, 0, GL_TEXTURE_WIDTH, &width);
		glGetTexLevelParameteriv(texture_type_, 0, GL_TEXTURE_HEIGHT, &height);
		glBindTexture(texture_type_, 0);
		qDebug<<__PRETTY_FUNCTION__<<"TRUE TEXTURE SIZE:"<<width<<height;
	#endif

	tex_.blitTexture(
		QRect(QPoint(0, 0), draw_size) // target rect
		, QRect(QPoint(0, 0), draw_size)); // source rect
}

/*!
 * Тестовая текстура - с котиком.
 */
/*void GrymQtAndroidViewGraphicsProxy::CreateTestTexture(QSize * out_texture_size_)
{
	qDebug()<<__PRETTY_FUNCTION__<<"tid"<<gettid();
	need_update_texture_ = false;
	QImage img;
	if (!img.load(":/images/kotik.png"))
	{
		qFatal("Failed to load PNG image for test texture.");
	}
	QImage GL_formatted_image = QGLWidget::convertToGLFormat(img);
	if (GL_formatted_image.isNull())
	{
		qFatal("Result of conversion to GL format is null.");
	}
	// Создаём текстуру
	glGenTextures(1, &texture_id_);
	static const GLenum target = GL_TEXTURE_2D;
	texture_type_ = target;

	// Выберем эту текстуру для работы
	glBindTexture(GL_TEXTURE_2D, texture_id_);
	// Загрузим данные текстуры (собственно картинку)
	glTexImage2D(
		target, 0, GL_RGBA
		, GL_formatted_image.width(), GL_formatted_image.height()
		, 0, GL_RGBA, GL_UNSIGNED_BYTE, GL_formatted_image.bits());
	// Параметры текстуры
	glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(target, 0);

	*out_texture_size_ = GL_formatted_image.size();
	qDebug()<<"Done"<<__PRETTY_FUNCTION__<<"Texture size:"<<GL_formatted_image.width()<<"x"<<GL_formatted_image.height();
}*/

void GrymQtAndroidViewGraphicsProxy::initTexture()
{
	//qDebug()<<__PRETTY_FUNCTION__<<"tid"<<gettid();
	if (!tex_.isAllocated())
	{
		qDebug()<<__PRETTY_FUNCTION__<<"tid"<<gettid()<<"Have to create a texture...";

		//! \todo Клипнуть запрошенные размеры по максимальным?

		tex_.allocateTexture(GL_TEXTURE_EXTERNAL_OES);

		// Определяем максимальные размеры текстуры
		GLint maxdims[2];
		GLint maxtextsz;
		glGetIntegerv(GL_MAX_VIEWPORT_DIMS, maxdims);
		glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxtextsz);
		int max_x = qMin(maxdims[0], maxtextsz);
		int max_y = qMin(maxdims[1], maxtextsz);
		QSize mysize = size().toSize();
		QSize texture_size = QSize(qMin(max_x, mysize.width()), qMin(max_y, mysize.height()));
		tex_.setTextureSize(texture_size);
		// texture_size_ = QSize(512, 512);
		qDebug()<<__PRETTY_FUNCTION__<<"GL_MAX_VIEWPORT_DIMS"<<maxdims[0]<<maxdims[1]
			<<"GL_MAX_TEXTURE_SIZE"<<maxtextsz
			<<"My size:"<<mysize.width()<<"x"<<mysize.height()
			<<"Resuling size:"<<texture_size.width()<<"x"<<texture_size.height();

		// Мы можем создать Java-вьюху только имея текстуру, а текстуру мы можем создать
		// только в OpenGL-контексте, поэтому конструирование сделаем при первой отрисовке.
		// Если вьюха тяжёлая, то её создание можно отложить на стороне Java.
		if (offscreen_view_factory_)
		{
			//! \todo Мы задаём размер вьюшки, а не текстур!
			offscreen_view_factory_->CallParamVoid("SetTexture", "I", jint(tex_.getTexture()));
			offscreen_view_factory_->CallParamVoid("SetTextureWidth",	"I", jint(texture_size.width()));
			offscreen_view_factory_->CallParamVoid("SetTextureHeight", "I", jint(texture_size.height()));
			offscreen_view_factory_->CallParamVoid("SetNativePtr", "J", jlong(reinterpret_cast<void*>(this)));
			offscreen_view_.reset(
				offscreen_view_factory_->CallObject("DoCreateView"
				, (c_class_path_+"/OffscreenView").toAscii()));
			offscreen_view_->RegisterNativeMethod(
				"nativeUpdate"
				, "(J)V"
				, (void*)Java_OffscreenView_nativeUpdate);
		}
	}
}

bool GrymQtAndroidViewGraphicsProxy::updateTexture()
{
	if (offscreen_view_)
	{
		// Заберём нарисованное изображение в текстуру.
		// Если прямо сейчас идёт отрисовка WebView, то придётся постоять на синхронизации в Java.
		bool success = offscreen_view_->CallBool("updateTexture");
		if (!success)
		{
			return false;
		}

		// Заберём матрицу преобразований текстуры
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
		qDebug()<<__PRETTY_FUNCTION__<<"Offscreen view does not exist (yet?(";
		return false;
	}
}

QSize GrymQtAndroidViewGraphicsProxy::getDrawableSize() const
{
	QSize mysize = size().toSize();
	return QSize(qMin(mysize.width(), tex_.getTextureSize().width()), qMin(mysize.height(), tex_.getTextureSize().height()));
}

// http://developer.android.com/reference/android/view/MotionEvent.htm
static const int
	ANDROID_MOTIONEVENT_ACTION_DOWN = 0,
	ANDROID_MOTIONEVENT_ACTION_UP = 1,
	ANDROID_MOTIONEVENT_ACTION_MOVE = 2;

void GrymQtAndroidViewGraphicsProxy::mouseMoveEvent(QGraphicsSceneMouseEvent * event)
{
	if (mouse_tracking_ && tex_.isAllocated() && offscreen_view_)
	{
		QPoint pos = event->pos().toPoint();
		offscreen_view_->CallParamVoid("ProcessMouseEvent", "III", jint(ANDROID_MOTIONEVENT_ACTION_MOVE), jint(pos.x()), jint(pos.y()));
		event->accept();
	}
}

void GrymQtAndroidViewGraphicsProxy::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
	if (tex_.isAllocated() && offscreen_view_ && event->button() == Qt::LeftButton)
	{
		QPoint pos = event->pos().toPoint();
		offscreen_view_->CallParamVoid("ProcessMouseEvent", "III", jint(ANDROID_MOTIONEVENT_ACTION_DOWN), jint(pos.x()), jint(pos.y()));
		// grabMouse();
		mouse_tracking_ = true;
		event->accept();
	}
}

void GrymQtAndroidViewGraphicsProxy::mouseReleaseEvent(QGraphicsSceneMouseEvent * event)
{
	if (tex_.isAllocated() && offscreen_view_ && event->button() == Qt::LeftButton)
	{
		QPoint pos = event->pos().toPoint();
		offscreen_view_->CallParamVoid("ProcessMouseEvent", "III", jint(ANDROID_MOTIONEVENT_ACTION_UP), jint(pos.x()), jint(pos.y()));
		// ungrabMouse();
		mouse_tracking_ = false;
		event->accept();
	}
}

