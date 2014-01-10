#pragma once
#include <QColor>
#include <QSize>
#include <QRect>
#include <QScopedPointer>
#include <JclassPtr.h>
#include <jcGeneric.h>
#include "QOpenGLTextureHolder.h"

class QAndroidOffscreenView: public QObject
{
	Q_OBJECT
	Q_PROPERTY(QSize size READ size WRITE resize)
	Q_PROPERTY(QColor fillColor READ fillColor WRITE setFillColor)
public:
	QAndroidOffscreenView(const QString & classname, const QString & objectname, const QSize & defsize, QObject * parent = 0);
	virtual ~QAndroidOffscreenView();

	/*!
	 * Initialize Android view. This function should be called within active proper GL context
	 * as it may want to create OpenGL texture.
	 */
	virtual void initializeGL();

	/*!
	 * Returns true if initializeGL() has been called.
	 */
	virtual bool isIntialized() const { return tex_.isAllocated(); }

	/*!
	 * Delete associated Android View and its rendering infrastructure. The texture continues
	 * to exist and can be painted, but updates, resizes and so on will not work anymore.
	 */
	virtual void deleteAndroidView();

	/*!
	 * Free Android view and OpenGL resources. The object will be unusable after that.
	 */
	virtual void deinitialize();

	/*!
	 * Draw the texture. targetRect is the output rectangle in OpenGL terms.
	 * If texture is not ready, the rectangle is filled with fillColor.
	 */
	virtual void paintGL(int l, int b, int w, int h);
	void paintGL(const QRect & targetRect) { paintGL(targetRect.left(), targetRect.bottom(), targetRect.width(), targetRect.height()); }
	void paintGL(const QPoint & point) { paintGL(QRect(point, size_)); }
	void paintGL(int x, int y) { paintGL(QRect(QPoint(x, y), size_)); }

	//! Check if Android View already exists
	bool isCreated() const;

	//! Check if Android View has already painted something
	virtual bool hasValidImage() const;

	//! Tell Android View to repaint.
	virtual void invalidate();

	QSize size() const { return size_; }
	virtual void resize(const QSize & size);
	QColor fillColor() const { return fill_color_; }
	virtual void setFillColor(const QColor & color);

	//
	// Handling of user input events
	//
	static const int
		ANDROID_MOTIONEVENT_ACTION_DOWN = 0,
		ANDROID_MOTIONEVENT_ACTION_UP = 1,
		ANDROID_MOTIONEVENT_ACTION_MOVE = 2;

	void mouse(int android_action, int x, int y);

	//! \todo Add keyboard and multi-touch

signals:
	/*!
	 * Emitted when texture has finished updating on Java side and the new image
	 * can be displayed in our Qt app UI (via paintGL()).
	 */
	void updated();

private slots:
	void javaUpdate();

protected:
	bool updateTexture();

protected:
	QString view_class_name_;
	QString view_object_name_;
	QOpenGLTextureHolder tex_;
	QScopedPointer<jcGeneric> offscreen_view_factory_;
	QScopedPointer<jcGeneric> offscreen_view_;
	QSize size_;
	QColor fill_color_;
	bool need_update_texture_;

private:
	Q_DISABLE_COPY(QAndroidOffscreenView)
	friend void JNICALL Java_OffscreenView_nativeUpdate(JNIEnv * env, jobject jo, jlong param);
};


