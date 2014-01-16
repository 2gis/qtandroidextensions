#include <jni.h>
#include <unistd.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <stdlib.h>
#include <QtOpenGL>
#include <QGLWidget>
#include "GrymQtAndroidViewGraphicsProxy.h"

// #define ANDROIDVIEWGRAPHICSPROXY_CLEARALL

GrymQtAndroidViewGraphicsProxy::GrymQtAndroidViewGraphicsProxy(QGraphicsItem *parent, Qt::WindowFlags wFlags)
	: QGraphicsWidget(parent, wFlags)
	, aview_("WebView1", true, QSize(512, 512))
	, mouse_tracking_(false)
{
	setAcceptedMouseButtons(Qt::LeftButton);
	connect(&aview_, SIGNAL(updated()), this, SLOT(onOffscreenUpdated()));
	connect(&aview_, SIGNAL(pageFinished()), this, SLOT(onPageFinished()));
	connect(&aview_, SIGNAL(contentHeightReceived(int)), this, SLOT(onContentHeightReceived(int)));

	// Since we created aview_ with "waitforcreation", we can safely start loading
	// the page right now.
	aview_.loadUrl("http://www.android.com/intl/en/about/");
}

GrymQtAndroidViewGraphicsProxy::~GrymQtAndroidViewGraphicsProxy()
{
}

void GrymQtAndroidViewGraphicsProxy::onOffscreenUpdated()
{
	update();
}

void GrymQtAndroidViewGraphicsProxy::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
	Q_UNUSED(option);
	Q_UNUSED(widget);
	Q_ASSERT(painter->paintEngine()->type() == QPaintEngine::OpenGL2);

	// Создадим текстуру
	if (!aview_.isIntialized())
	{
		aview_.initializeGL();
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
	aview_.paintGL(l, b, w, h);

	#if defined(ANDROIDVIEWGRAPHICSPROXY_CLEARALL)
		glScissor(0, 0, static_cast<GLsizei>(device->width()), static_cast<GLsizei>(device->height()));
		glDisable(GL_SCISSOR_TEST);
	#endif

	// Возвращаем viewport в исходное состояние
	glViewport(0, 0, static_cast<GLsizei>(device->width()), static_cast<GLsizei>(device->height()));
	painter->endNativePainting();
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

// http://developer.android.com/reference/android/view/MotionEvent.htm

void GrymQtAndroidViewGraphicsProxy::mouseMoveEvent(QGraphicsSceneMouseEvent * event)
{
	if (mouse_tracking_)
	{
		QPoint pos = event->pos().toPoint();
		aview_.mouse(QAndroidOffscreenView::ANDROID_MOTIONEVENT_ACTION_MOVE, pos.x(), pos.y());
		event->accept();
	}
}

void GrymQtAndroidViewGraphicsProxy::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
	if (event->button() == Qt::LeftButton)
	{
		QPoint pos = event->pos().toPoint();
		aview_.mouse(QAndroidOffscreenView::ANDROID_MOTIONEVENT_ACTION_DOWN, pos.x(), pos.y());
		mouse_tracking_ = true;
		event->accept();
	}
}

void GrymQtAndroidViewGraphicsProxy::mouseReleaseEvent(QGraphicsSceneMouseEvent * event)
{
	if (event->button() == Qt::LeftButton)
	{
		QPoint pos = event->pos().toPoint();
		aview_.mouse(QAndroidOffscreenView::ANDROID_MOTIONEVENT_ACTION_UP, pos.x(), pos.y());
		mouse_tracking_ = false;
		event->accept();
	}
}

void GrymQtAndroidViewGraphicsProxy::resizeEvent(QGraphicsSceneResizeEvent *event)
{
	qDebug()<<__PRETTY_FUNCTION__<<event->newSize().toSize();
	QGraphicsWidget::resizeEvent(event);
	aview_.resize(event->newSize().toSize());
}

void GrymQtAndroidViewGraphicsProxy::onPageFinished()
{
	qDebug()<<__PRETTY_FUNCTION__;
	aview_.requestContentHeight();
}

void GrymQtAndroidViewGraphicsProxy::onContentHeightReceived(int height)
{
	qDebug()<<__PRETTY_FUNCTION__<<"Page height:"<<height;
}

