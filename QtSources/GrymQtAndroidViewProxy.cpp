#include <QtOpenGL>
#include <EGL/egl.h>
#include "GrymQtAndroidViewProxy.h"

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
	setAttribute(Qt::WA_PaintOnScreen);
	setAttribute(Qt::WA_NoSystemBackground);
	setAutoBufferSwap(false);
}


void GrymQtAndroidViewProxy::initializeGL()
{
	qDebug()<<__PRETTY_FUNCTION__;
	QGLWidget::initializeGL();

	glClearColor(0.1, 1.0, 0.1, 1.0);
}

void GrymQtAndroidViewProxy::resizeGL(int width, int height)
{
	qDebug()<<__PRETTY_FUNCTION__;
	QGLWidget::resizeGL(width, height);
}

void GrymQtAndroidViewProxy::paintGL()
{
	qDebug()<<__PRETTY_FUNCTION__;
	// QGLWidget::paintGL();

	QPainter painter;
	painter.begin(this);
	painter.beginNativePainting();

	glClearColor(0.1f, 1.0f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	painter.endNativePainting();
	painter.end();
	swapBuffers();
}

