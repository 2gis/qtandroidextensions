#pragma once
#include <QtOpenGL/QGLWidget>
#include <QTimer>
#include <QtGui/QGraphicsWidget>
#include <QPaintEvent>
#include <EGL/egl.h>
#include <JclassPtr.h>
#include <jcGeneric.h>
#include "QOpenGLTextureHolder.h"

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

	//! \param x, y, w, h - координаты региона в терминах OpenGL
	virtual void doGLPainting(int x, int y, int w, int h);
	void initTexture();
	bool updateTexture();

	QSize getDrawableSize() const;

private:
	//void CreateTestTexture(QSize * out_texture_size_);

	/*! Java вызывает эту функцию, чтобы сообщить, что дорисовалась новая текстура.
		Она оборачивает вызов javaUpdate() в нашем треде. */
	friend void JNICALL Java_OffscreenView_nativeUpdate(JNIEnv * env, jobject jo, jlong param);

private slots:
	/*! Вызывается, когда нужно инициировать перерисовку себя (есть новая текстура). */
	void javaUpdate();

	void onRefreshTimer();

private:
	QOpenGLTextureHolder tex_;
	QScopedPointer<jcGeneric> offscreen_view_factory_;
	QScopedPointer<jcGeneric> offscreen_view_;
	bool need_update_texture_;
	bool mouse_tracking_;
	QTimer refresh_timer_;
};
