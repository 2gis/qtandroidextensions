#pragma once
#include <QtOpenGL/QGLWidget>
#include <QtGui/QGraphicsWidget>
#include <QPaintEvent>

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
