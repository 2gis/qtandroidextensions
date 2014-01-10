#pragma once
#include <QtOpenGL/QGLWidget>
#include <QTimer>
#include <QtGui/QGraphicsWidget>
#include <QPaintEvent>
#include <EGL/egl.h>
#include <JclassPtr.h>
#include <jcGeneric.h>
#include "QAndroidOffscreenView.h"

class GrymQtAndroidViewGraphicsProxy
	: public QGraphicsWidget
{
	Q_OBJECT
public:
	GrymQtAndroidViewGraphicsProxy(QGraphicsItem *parent = 0, Qt::WindowFlags wFlags = 0);
	virtual ~GrymQtAndroidViewGraphicsProxy();

protected:
	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	virtual void mouseMoveEvent(QGraphicsSceneMouseEvent * event);
	virtual void mousePressEvent(QGraphicsSceneMouseEvent * event);
	virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent * event);
	virtual void resizeEvent(QGraphicsSceneResizeEvent *event);

private slots:
	void onOffscreenUpdated();

private:
	//void CreateTestTexture(QSize * out_texture_size_);

private:
	QAndroidOffscreenView aview_;
	bool mouse_tracking_;
};
