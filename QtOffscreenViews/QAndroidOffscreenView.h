/*
  Offscreen Android Views library for Qt

  Author:
  Sergey A. Galin <sergey.galin@gmail.com>

  Distrbuted under The BSD License

  Copyright (c) 2014, DoubleGIS, LLC.
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.
  * Neither the name of the DoubleGIS, LLC nor the names of its contributors
    may be used to endorse or promote products derived from this software
    without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
  BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
  THE POSSIBILITY OF SUCH DAMAGE.
*/

#pragma once
#include <QColor>
#include <QSize>
#include <QRect>
#include <QScopedPointer>
#include <QMutex>
#include <JclassPtr.h>
#include <jcGeneric.h>
#include <JniEnvPtr.h>
#include "QOpenGLTextureHolder.h"

#if QT_VERSION < 0x050000
	#define QANDROIDOFFSCREENVIEW_ALLOWWAIT
#endif

/*!
 * A general wrapper for Android offscreen views.
 * It can be used to create a QWidget / QGLWidget / QGraphicsWidget / QML component
 * which displays Android view.
 */
class QAndroidOffscreenView: public QObject
{
	Q_OBJECT
	Q_PROPERTY(QSize size READ size WRITE resize)
	Q_PROPERTY(QColor fillColor READ fillColor WRITE setFillColor)
	Q_PROPERTY(bool synchronizedTextureUpdate READ synchronizedTextureUpdate WRITE setSynchronizedTextureUpdate)
	Q_PROPERTY(bool visible READ visible WRITE setVisible)
protected:
	/*!
	 * \param classname - name of Java class of the View wrapper.
	 * \param objectname - set for QObject and also passed to Java side, to identify the object in logs and etc.
	 * \param defsize - initial size of the view.
	 * \param parent - passed to QObject constructor.
	 */
	QAndroidOffscreenView(const QString & classname, const QString & objectname, bool create_view, const QSize & defsize, QObject * parent = 0);

	//! \todo Use return result!
	bool createView();

public:
	virtual ~QAndroidOffscreenView();

	static const QString & getDefaultJavaClassPath();

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
	virtual void paintGL(int l, int b, int w, int h, bool reverse_y);

	//! Check if Android View already exists
	bool isCreated() const;

#if defined(QANDROIDOFFSCREENVIEW_ALLOWWAIT)
	/*!
	 * Wait until Android View will be actually created.
	 * This function should be called after initializeGL().
	 * If the View is already created or initializeGL() has not been called the function
	 * returns immediately.
	 * \note This function is not made availble for Qt 5 because in typical use cases Java UI thread
	 *  may be blocked during call for waitForViewCreation() and your application will deadlock.
	 */
	bool waitForViewCreation();
#endif

	//! Check if Android View has already painted something
	virtual bool hasValidImage() const;

	//! Tell Android View to repaint.
	virtual void invalidate();

	QSize size() const { return size_; }
	virtual void resize(const QSize & size);
	QColor fillColor() const { return fill_color_; }
	virtual void setFillColor(const QColor & color);

	/*!
	 * When we want to get new texture, and the View is doing its drawing, do we wait until
	 * the will finish (synced mode), or just use previous texture (not synced)?
	 * In the non-sync mode Qt UI gets more green light but updates of the View
	 * will be ignored until Qt's UI thread is busy.
	 * The default mode is synced as it typically provides better user experience.
	 */
	bool synchronizedTextureUpdate() const { return synchronized_texture_update_; }
	void setSynchronizedTextureUpdate(bool synced) { synchronized_texture_update_ = synced; }

	/*!
	 * Does the view thinks it's visible? When view is invisible, the texture may
	 * contain an empty or outdated image.
	 */
	bool visible() const { return is_visible_; }

	/*!
	 * Set visibility flag. It informs the view if it's visible or not.
	 * When view is invisible, it may save resources by not doing any timer-based work,
	 * for example. For simple widgets, it is not necessary to set them invisible
	 * because it is enough to hide the control displaying the texture. For Wev Views
	 * with active contents (e.g. JavaScript with timers, GIF images) it is important
	 * to stop them when content updates are not visible to users to save CPU power
	 * and battery.
	 * Warning: unlike for widgets, visibility also should be explicitly set to false
	 * when application goes to background. Cases when offscreen view should be marked
	 * hidden are different depending on use-cases, graphical systems and QPA plugins
	 * so proper reacting to application activation/deactivation cannot be handled
	 * on this level.
	 */
	void setVisible(bool visible);

	//
	// Handling of user input events
	//
	static const int
		ANDROID_MOTIONEVENT_ACTION_DOWN = 0,
		ANDROID_MOTIONEVENT_ACTION_UP = 1,
		ANDROID_MOTIONEVENT_ACTION_MOVE = 2;

	/*!
	 * Single-touch / mouse support.
	 * Typically, this function is called from Qt mouse event handlers.
	 * \param android_action can be ANDROID_MOTIONEVENT_ACTION_DOWN, ANDROID_MOTIONEVENT_ACTION_UP,
	 *  ANDROID_MOTIONEVENT_ACTION_MOVE.
	 */
	void mouse(int android_action, int x, int y);

	//! \todo Add keyboard and multi-touch support

signals:
	/*!
	 * Emitted when texture has finished updating on Java side and the new image
	 * can be displayed in our Qt app UI (via paintGL()).
	 */
	void updated();
	void viewCreated();

private slots:
	void javaUpdate();
	void javaViewCreated();

protected:
	bool updateTexture();
	jcGeneric * offscreenView() { return offscreen_view_.data(); }
	jcGeneric * getView();

private:
	QString view_class_name_;
	QString view_object_name_;
	QOpenGLTextureHolder tex_;
	QScopedPointer<jcGeneric> offscreen_view_;
	QSize size_;
	QColor fill_color_;
	bool need_update_texture_;
	bool view_painted_;
	bool texture_received_;
	bool synchronized_texture_update_;
	bool view_creation_requested_;
	bool is_visible_;
	volatile mutable bool view_created_; //!< Cache for isCreated()

	// Keeping threads attached to Java (peformance issue).
	QScopedPointer<JniEnvPtr> initial_thread_attacher_;
	QScopedPointer<JniEnvPtr> jni_gl_thread_attacher_;
private:
	Q_DISABLE_COPY(QAndroidOffscreenView)
	friend void JNICALL Java_OffscreenView_nativeUpdate(JNIEnv * env, jobject jo, jlong param);
};

