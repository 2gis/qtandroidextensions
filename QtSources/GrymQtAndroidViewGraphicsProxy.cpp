#include <QtOpenGL>
#include <EGL/egl.h>
#include "GrymQtAndroidViewGraphicsProxy.h"

GrymQtAndroidViewProxy::GrymQtAndroidViewProxy(QWidget * parent, const QGLWidget * shareWidget, Qt::WindowFlags f)
	: QGLWidget(parent, shareWidget, f)
	, texture_id_(0)
{
	init();
}

GrymQtAndroidViewProxy::GrymQtAndroidViewProxy(QGLContext * context, QWidget * parent, const QGLWidget * shareWidget, Qt::WindowFlags f)
	: QGLWidget(context, parent, shareWidget, f)
	, texture_id_(0)
{
	init();
}

GrymQtAndroidViewProxy::GrymQtAndroidViewProxy(const QGLFormat & format, QWidget * parent, const QGLWidget * shareWidget, Qt::WindowFlags f)
	: QGLWidget(format, parent, shareWidget, f)
	, texture_id_(0)
{
	init();
}

GrymQtAndroidViewProxy::~GrymQtAndroidViewProxy()
{
}

void GrymQtAndroidViewProxy::init()
{
	/*glInit(); // WTF?
	setAttribute(Qt::WA_PaintOnScreen);
	setAttribute(Qt::WA_NoSystemBackground);
	setAutoBufferSwap(false);
	setMinimumSize(50, 50);*/
}

void GrymQtAndroidViewProxy::initializeGL()
{
	qDebug()<<__PRETTY_FUNCTION__;
	QGLWidget::initializeGL();
}

void GrymQtAndroidViewProxy::resizeGL(int width, int height)
{
	qDebug()<<__PRETTY_FUNCTION__;
	QGLWidget::resizeGL(width, height);
}

void GrymQtAndroidViewProxy::paintGL()
{
	qDebug()<<__PRETTY_FUNCTION__<<"Pos:"<<this->pos()<<"Size:"<<this->size();
	// QGLWidget::paintGL();





	QPainter painter;
	painter.begin(this);

	painter.fillRect(0, 0, width(), height(), Qt::green);

	/*painter.beginNativePainting();
	glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	painter.endNativePainting();(*/

	painter.end();
	swapBuffers();
}

void GrymQtAndroidViewProxy::paintEvent(QPaintEvent * e)
{
	qDebug()<<__PRETTY_FUNCTION__;
	QGLWidget::paintEvent(e);
	updateGL();
	//e->accept();

	// WTF из Carto
/*(	QPainter painter(this);
	painter.beginNativePainting();
	paintGL();
	painter.endNativePainting();*/
}




GrymQtAndroidViewGraphicsProxy::GrymQtAndroidViewGraphicsProxy(QGraphicsItem *parent, Qt::WindowFlags wFlags)
	: QGraphicsWidget(parent, wFlags)
	, texture_id_(0)
{
}

GrymQtAndroidViewGraphicsProxy::~GrymQtAndroidViewGraphicsProxy()
{
}

void GrymQtAndroidViewGraphicsProxy::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	// painter->fillRect(rect(), Qt::green);
	painter->beginNativePainting();
	{
		QPaintDevice *device = painter->device();
		{
			QTransform combined_transform = painter->combinedTransform();

			/*BOOST_ASSERT(
				(combined_transform.type() == QTransform::TxNone) ||
				(combined_transform.type() == QTransform::TxTranslate) ||
				(combined_transform.type() == QTransform::TxScale));*/

			QRectF widget_geometry = combined_transform.mapRect(QRectF(QPointF(0, 0), size()));

			GLint l = static_cast<GLint>(widget_geometry.left());
			GLint b = static_cast<GLint>(device->height() - static_cast<int>(widget_geometry.bottom()));
			GLsizei w = static_cast<GLsizei>(widget_geometry.width());
			GLsizei h = static_cast<GLsizei>(widget_geometry.height());
			glViewport(l, b, w, h);

			glEnable(GL_SCISSOR_TEST);
			glScissor(l, b, w, h);
		}

		doGLPainting();

		glScissor(0, 0, static_cast<GLsizei>(device->width()), static_cast<GLsizei>(device->height()));
		glDisable(GL_SCISSOR_TEST);

		// Возвращаем viewport в исходное состояние
		glViewport(0, 0, static_cast<GLsizei>(device->width()), static_cast<GLsizei>(device->height()));
	}
	painter->endNativePainting();
}

void GrymQtAndroidViewGraphicsProxy::doGLPainting()
{
	glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}


