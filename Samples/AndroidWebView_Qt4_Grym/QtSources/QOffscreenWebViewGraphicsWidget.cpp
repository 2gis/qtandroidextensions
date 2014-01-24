#include <jni.h>
#include <unistd.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <stdlib.h>
#include <QtOpenGL>
#include <QGLWidget>
#include "QOffscreenWebViewGraphicsWidget.h"

// #define ANDROIDVIEWGRAPHICSPROXY_CLEARALL

QOffscreenWebViewGraphicsWidget::QOffscreenWebViewGraphicsWidget(QGraphicsItem *parent, Qt::WindowFlags wFlags)
	: QGraphicsWidget(parent, wFlags)
	, aview_("WebView1", QSize(512, 512))
	, mouse_tracking_(false)
{
	setAcceptedMouseButtons(Qt::LeftButton);
	connect(&aview_, SIGNAL(updated()), this, SLOT(onOffscreenUpdated()));
	connect(&aview_, SIGNAL(pageFinished()), this, SLOT(onPageFinished()));
	connect(&aview_, SIGNAL(contentHeightReceived(int)), this, SLOT(onContentHeightReceived(int)));

	// This is not necessary anymore, as the view will schedule any actions for execution when the
	// view is ready:
	// aview_.waitForViewCreation();

	aview_.loadUrl("http://www.android.com/intl/en/about/");
}

QOffscreenWebViewGraphicsWidget::~QOffscreenWebViewGraphicsWidget()
{
}

void QOffscreenWebViewGraphicsWidget::onOffscreenUpdated()
{
	update();
}

void QOffscreenWebViewGraphicsWidget::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
	Q_UNUSED(option);
	Q_UNUSED(widget);
	Q_ASSERT(painter->paintEngine()->type() == QPaintEngine::OpenGL2);

	// We don't have any function like initializeGL in QGraphicsWidget, so let's just
	// do the initialization during the first paint.
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

	l += widget->pos().x();
	b += widget->pos().y();
	//! \todo Take into account position of the view within the window
	w++;
	h++;

	glViewport(l, b, w, h);

	#if defined(ANDROIDVIEWGRAPHICSPROXY_CLEARALL)
		glEnable(GL_SCISSOR_TEST);
		glScissor(l, b, w, h);

		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	#endif

	// Finally, we can draw the texture using these GL coordinates
	aview_.paintGL(l, b, w, h, false);

	#if defined(ANDROIDVIEWGRAPHICSPROXY_CLEARALL)
		glScissor(0, 0, static_cast<GLsizei>(device->width()), static_cast<GLsizei>(device->height()));
		glDisable(GL_SCISSOR_TEST);
	#endif

	// Reset viewport (is it necessary?)
	glViewport(0, 0, static_cast<GLsizei>(device->width()), static_cast<GLsizei>(device->height()));
	painter->endNativePainting();
}

void QOffscreenWebViewGraphicsWidget::mouseMoveEvent(QGraphicsSceneMouseEvent * event)
{
	if (mouse_tracking_)
	{
		QPoint pos = event->pos().toPoint();
		aview_.mouse(QAndroidOffscreenView::ANDROID_MOTIONEVENT_ACTION_MOVE, pos.x(), pos.y());
		event->accept();
	}
}

void QOffscreenWebViewGraphicsWidget::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
	if (event->button() == Qt::LeftButton)
	{
		QPoint pos = event->pos().toPoint();
		aview_.mouse(QAndroidOffscreenView::ANDROID_MOTIONEVENT_ACTION_DOWN, pos.x(), pos.y());
		mouse_tracking_ = true;
		event->accept();
	}
}

void QOffscreenWebViewGraphicsWidget::mouseReleaseEvent(QGraphicsSceneMouseEvent * event)
{
	if (event->button() == Qt::LeftButton)
	{
		QPoint pos = event->pos().toPoint();
		aview_.mouse(QAndroidOffscreenView::ANDROID_MOTIONEVENT_ACTION_UP, pos.x(), pos.y());
		mouse_tracking_ = false;
		event->accept();
	}
}

void QOffscreenWebViewGraphicsWidget::resizeEvent(QGraphicsSceneResizeEvent *event)
{
	qDebug()<<__PRETTY_FUNCTION__<<event->newSize().toSize();
	QGraphicsWidget::resizeEvent(event);
	aview_.resize(event->newSize().toSize());
}

void QOffscreenWebViewGraphicsWidget::onPageFinished()
{
	qDebug()<<__PRETTY_FUNCTION__;
	aview_.requestContentHeight();
}

void QOffscreenWebViewGraphicsWidget::onContentHeightReceived(int height)
{
	qDebug()<<__PRETTY_FUNCTION__<<"Page height:"<<height;
}

