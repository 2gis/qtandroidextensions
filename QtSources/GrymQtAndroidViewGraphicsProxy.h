#pragma once
#include <QtOpenGL/QGLWidget>
#include <QtGui/QGraphicsWidget>
#include <QPaintEvent>

class GrymQtAndroidViewProxy
	: public QGLWidget
{
	Q_OBJECT
public:
	explicit GrymQtAndroidViewProxy(QWidget* parent=0, const QGLWidget* shareWidget = 0, Qt::WindowFlags f=0);
	explicit GrymQtAndroidViewProxy(QGLContext *context, QWidget* parent=0, const QGLWidget* shareWidget = 0, Qt::WindowFlags f=0);
	explicit GrymQtAndroidViewProxy(const QGLFormat& format, QWidget* parent=0, const QGLWidget* shareWidget = 0, Qt::WindowFlags f=0);
	virtual ~GrymQtAndroidViewProxy();

protected:
	virtual void initializeGL();
	virtual void resizeGL(int width, int height);
	virtual void paintGL();

	virtual void paintEvent(QPaintEvent * e);

private:
	void init();

private:
	int texture_id_;
};

class GrymQtAndroidViewGraphicsProxy
	: public QGraphicsWidget
{
	Q_OBJECT
public:
	GrymQtAndroidViewGraphicsProxy(QGraphicsItem *parent = 0, Qt::WindowFlags wFlags = 0);
	virtual ~GrymQtAndroidViewGraphicsProxy();

protected:
	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	virtual void doGLPainting();

private:
	int texture_id_;
};
