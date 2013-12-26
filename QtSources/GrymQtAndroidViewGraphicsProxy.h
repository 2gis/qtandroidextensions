#pragma once
#include <QtOpenGL/QGLWidget>
#include <QtGui/QGraphicsWidget>
#include <QPaintEvent>
#include <EGL/egl.h>
#include "JNIUtils/JclassPtr.h"
#include "JNIUtils/jcGeneric.h"

class GrymQtAndroidViewGraphicsProxy
	: public QGraphicsWidget
{
	Q_OBJECT
public:
	GrymQtAndroidViewGraphicsProxy(QGraphicsItem *parent = 0, Qt::WindowFlags wFlags = 0);
	virtual ~GrymQtAndroidViewGraphicsProxy();

protected:
	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	//! \param x, y, w, h - координаты региона в терминах OpenGL
	virtual void doGLPainting(int x, int y, int w, int h);
	void initTexture();
	bool updateTexture();
	void destroyTexture();

	QSize getDrawableSize() const;

private:
	void CreateTestTexture(QSize * out_texture_size_);
	void CreateEmptyExternalTexture();

	//! \todo refactor
	void drawTexture(const QRectF &rect, const QRectF &bitmap_rect = QRectF());
	void blitTexture(const QRect &targetRect, const QRect &sourceRect);

	/*! Java вызывает эту функцию, чтобы сообщить, что дорисовалась новая текстура.
		Она оборачивает вызов javaUpdate() в нашем треде. */
	friend void JNICALL Java_OffscreenView_nativeUpdate(JNIEnv * env, jobject jo, jlong param);

private slots:
	/*! Вызывается, когда нужно инициировать перерисовку себя (есть новая текстура). */
	void javaUpdate();

private:
	GLuint texture_id_;
	GLenum texture_type_;
	bool texture_available_;
	QSize texture_size_;
	QScopedPointer<jcGeneric> offscreen_view_factory_;
	QScopedPointer<jcGeneric> offscreen_view_;
	GLfloat texture_transform_[16];
	bool texture_transform_set_;
	bool need_update_texture_;
};
