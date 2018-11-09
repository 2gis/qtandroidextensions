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
#include <QtGui/QColor>
#include <QtCore/QSize>
#include <QtCore/QRect>
#include <QtCore/QScopedPointer>
#include <QtCore/QMutex>
#include <QJniHelpers/QJniHelpers.h>
#include "QAndroidJniImagePair.h"
#include "QOpenGLTextureHolder.h"
#include "QApplicationActivityObserver.h"

/*!
 * A general wrapper for Android offscreen views.f
 * It can be used to create a QWidget / QGLWidget / QGraphicsWidget / QML component
 * which displays Android view.
 */
class QAndroidOffscreenView: public QObject
{
	Q_OBJECT
	Q_PROPERTY(QSize size READ size WRITE resize)
	Q_PROPERTY(QColor fillColor READ fillColor WRITE setFillColor)
	Q_PROPERTY(bool visible READ visible WRITE setVisible)
	Q_PROPERTY(bool enabled READ enabled WRITE setEnabled)
protected:
	/*!
	 * \param classname - name of Java class of the View wrapper.
	 * \param objectname - set for QObject and also passed to Java side, to identify the object in logs and etc.
	 * \param defsize - initial size of the view.
	 * \param parent - passed to QObject constructor.
	 * \note The function invokes createView() through Qt::QueuedConnection so descenant constructor still
	 *       has time to register View-specific native functions and do some other initialization before
	 *       the construction of the View will start.
	 */
	QAndroidOffscreenView(const QString & classname, const QString & objectname, const QSize & defsize, QObject * parent = 0);

protected slots:
	void createView();

public:
	virtual ~QAndroidOffscreenView();

	static const QString & getDefaultJavaClassPath();
	static void preloadJavaClasses();

	//
	// Functions to check for available configuration
	//

	static bool openGlTextureSupportedOnJavaSide();
	static bool nonAttachingModeSupported();

	const QString & viewObjectName() const { return view_object_name_; }
	const QString & viewClassName() const { return view_class_name_; }

	/*!
	 * Initialize OpenGL rendering surface. This function should be called within active
	 * proper GL context as it will want to create an OpenGL texture.
	 * When View is initialized using this function it may be rendered on screen only using
	 * paintGL(). Call to getBitmapBuffer() may return 0.
	 * If GL mode is not supported on Android side this function will call initializeBitmap()
	 * instead of creating GL texture surface; paintGL() can still be used to draw the View.
	 * It is safe to call this function multiple times. All subsequent calls are ignored.
	 */
	virtual void initializeGL();

	/*!
	 * Initialize non-OpenGL (Bitmap-based) rendering surface. The View can be rendered on screen
	 * either using paintGL() or by getting QImage via getBitmapBuffer() and drawing it using
	 * some other method, e.g. QPainter::drawImage().
	 * This function should be used when Qt is working in unaccelerated UI mode.
	 * It is safe to call this function multiple times. All subsequent calls are ignored.
	 */
	virtual void initializeBitmap();

	/*!
	 * Returns true if initializeGL() has been called.
	 */
	virtual bool isIntialized() const { return tex_.isAllocated() || bitmap_a_.isAllocated(); }

	/*!
	 * Delete associated Android View and its rendering infrastructure. The texture continues
	 * to exist and can be painted, but updates, resizes and so on will not work anymore.
	 */
	virtual void deleteAndroidView();

	/*!
	 * Draw GL texture or Bitmap (depending on the rendering surface on the Java side) using OpenGL.
	 * targetRect is the output rectangle in OpenGL terms. If View image is not ready, the rectangle
	 * is filled with fillColor.
	 * Note: to draw the View when Qt is working in unaccelerated mode, intialize it
	 * via initializeBitmap() and use QImage returned by getBitmapBuffer().
	 */
	virtual void paintGL(int l, int b, int w, int h, bool reverse_y);

	/*!
	 * Makes sure that the GL texture holder contains actual image, if possible.
	 * This function should only be called if \ref getGLTextureHolder() is used to access
	 * the texure directly. It is not needed to call it when using paintGL().
	 * Note: in Bitmap+GL mode this function may cause re-allocation of GL texture.
	 */
	bool updateGLTextureInHolder();

	/*!
	 * Access \ref QOpenGLTextureHolder for reading texture id or other properties
	 * for direct painting without using \ref paintGL().
	 * \ref updateGLTextureInHolder() should be called to make sure that the texture
	 * contains actual image.
	 */
	const QOpenGLTextureHolder & getGLTextureHolder() const { return tex_; }

	/*!
	 * Used in Bitmap mode. Instead of paintGL(), get current bitmap buffer
	 * using getBitmapBuffer() and paint it by yourself.
	 * The image buffer is guaranteed to be unmodified until the next call to getBitmapBuffer().
	 * \note This function is not guaranteed to return expected image when
	 *  initialized through initializeGL(). For example, returned image may be padded to
	 *  have power-of-two size.
	 * \param out_texture_updated: the bool value is set to true/false to indicate
	 *  that the image has been actually changed since the last call to getBitmapBuffer().
	 *  This can be used, for example, to avoid reloading the bitmap into GL texture.
	 */
	const QImage * getBitmapBuffer(bool * out_texture_updated = 0);

	//! Check if Android View already exists.
	bool isCreated() const;

	//! Check if Android View has already painted something.
	virtual bool hasValidImage() const;

	QSize size() const { return size_; }
	virtual void resize(const QSize & newsize);
	QColor fillColor() const { return fill_color_; }
	virtual void setFillColor(const QColor & color);

	/*!
	 * Does the view thinks it's visible? When view is invisible, the texture may
	 * contain an empty or outdated image. Also it stops all animations and etc.
	 */
	bool visible() const { return is_visible_; }

	/*!
	 * Set visibility flag. It informs the view if it's visible or not.
	 * By default, view is invisible.
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

	bool enabled() const { return is_enabled_; }

	void setEnabled(bool enabled);

	/*!
	 * Control attaching View to the main activity View.
	 * It is typically called one time after constructing QAndroidOffscreenView.
	 * Unattached views will not receive keyboard events when focused and
	 * can't use SIP and popup menus.
	 * Non-attached mode is not supported on API < 11 (\see nonAttachingModeSupported()).
	 */
	void setAttachingMode(bool attaching);

	void reattachView();

	/* !
	 * Returns true if View is currently focused. Please note that this View takes and
	 * and releases focus in another thread so this function may e.g. return false if
	 * called immediately after setFocused(true).
	 */
	//bool isFocused() const;

	/*!
	 * Set or remove focusing from the View. This should be called to keep Qt focus
	 * in sync with Android focus.
	 */
	void setFocused(bool focused = true);

	/*!
	 * Inform Android about real screen coordinates of the View.
	 * This is important only if the View works with SIP (e.g.: EditText, interactive WebView)
	 * because input method may want to display selection markers and need to know the
	 * real position of the view. For non-interactive views calling this function is
	 * not necessary.
	 */
	void setPosition(int left, int top);

	//! Make sure software keyboard is hidden for this control.
	void hideKeyboard();

	//! Make sure software keyboard is shown for this control.
	void showKeyboard();

	/*!
	 * Set/cleaer the flag to hide software keyboard when focus is lost.
	 * By default, the mode is enabled and keyboard slides away when focus goes from the View.
	 * If it is disabled, SIP won't hide when user changes focus to any pure Qt control
	 * because main Qt window always accepts text input.
	 * Disable the mode if you want user to be able to switch between various form
	 * fields without hiding and re-opening keyboard. In this case, you have to call
	 * hideKeyboard() from your C++/Qt code to make sure the keyboard is hidden when focus
	 * goes to something which doesn't need it.
	 */
	void setHideKeyboardOnFocusLoss(bool hide);

	/*!
	 * Set/cleaer the flag to show software keyboard on focused.
	 * By default, the mode is disabled.
	 */
	void setShowKeyboardOnFocusIn(bool show);

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
	 * \param timestamp_uptime_mills - this should be set either as a System.uptimeMillis() of the
	 *  time when the event has been initially generated or 0. Unfortunately, stock Qt currently
	 *  doesn't pass the right timestamp in mouse events. So we can pass 0 here and the Java side
	 *  will enable a workaround of getting the current timestamp. It will cause errors in determination
	 *  of the movement speed because the initial Andoid UI event queue and the Qt UI queue run at
	 *  uneven speed. So movement animations will not work good (e.g. WebView scrolling).
	 */
	void mouse(int android_action, int x, int y, long long timestamp_uptime_millis = 0);

	//! Return the scrolled left position of this view.
	int getScrollX();

	//! Return the scrolled top position of this view.
	int getScrollY();

	void setScrollX(int x);
	void setScrollY(int y);

	//! Return the width measurement information for this view as computed by the most recent call to measure(int, int).
	int getMeasuredWidth();

	//! Return the height measurement information for this view as computed by the most recent call to measure(int, int).
	int getMeasuredHeight();

	//! \todo Add multi-touch support!

	void setSoftInputModeResize();
	void setSoftInputModeAdjustPan();
	void setSoftInputModeAdjustNothing(); // API 11+

	//! Test function for lib developers, don't use it
	void testFunction();

public slots:
	/*!
	 * Free Android view and OpenGL resources. The object will be unusable after that.
	 */
	virtual void deinitialize();

	//! Tell Android View to repaint.
	virtual void invalidate();

	//! Catch application activity status changes from the app activity observer.
	virtual void applicationActivityStatusChanged();

	//! Update Android View visibility from visibility of the Qt View and activity of Qt application.
	virtual void updateAndroidViewVisibility();

	/*!
	 * Request currently visible rectangle of the view.
	 * The result will be sent via visibleRectReceived() signal.
	 * This function returns size of rectangle not covered by software keyboard
	 * and may be used to scroll contents to have a text edit located in
	 * the visible part of the screen.
	 */
	void requestVisibleRect();


signals:
	/*!
	 * Emitted when texture has finished updating on Java side and the new image
	 * can be displayed in our Qt app UI (via paintGL()).
	 */
	void updated();

	/*!
	 * Emitted when view has been actually created.
	 */
	void viewCreated();

	/*!
	 * Emitted when visible screen rect requested via requestVisibleRect() is receieved.
	 */
	void visibleRectReceived(int width, int height);

private slots:
	void javaUpdate();
	void javaViewCreated();
	void javaVisibleRectReceived(int left, int top, int right, int bottom);

private:
	const QImage * getPreviousBitmapBuffer(bool convert_from_android_format);

protected:
	const QImage * getBitmapBuffer(bool * out_texture_updated, bool convert_from_android_format);
	bool updateGlTexture();
	bool updateBitmapToGlTexture();
	QJniObject * offscreenView() { return offscreen_view_.data(); }
	const QJniObject * offscreenView() const { return offscreen_view_.data(); }
	QJniObject * getView();

private:
	QString view_class_name_;
	QString view_object_name_;

	//! Keeps OpenGL texture when painting goes on in GL.
	QOpenGLTextureHolder tex_;

	//! Intermediate buffer used in Bitmap mode to convert Android's BGR to RGB.
	QImage android_to_qt_buffer_;
	int last_qt_buffer_;

	//! Double buffer for Bitmap mode.
	QAndroidJniImagePair bitmap_a_, bitmap_b_;

	//! Used to lock bitmap_a_/bitmap_b_ access.
	QMutex bitmaps_mutex_;

	QScopedPointer<QJniObject> offscreen_view_;
	QSize size_;
	QColor fill_color_;
	volatile bool need_update_texture_;
	volatile bool view_painted_;
	bool texture_received_;
	bool view_creation_requested_;
	bool is_visible_;
	bool is_enabled_;
	volatile mutable bool view_created_; //!< Cache for isCreated()
	int last_texture_width_, last_texture_height_;
private:
	Q_DISABLE_COPY(QAndroidOffscreenView)
	friend void JNICALL Java_OffscreenView_nativeUpdate(JNIEnv * env, jobject jo, jlong param);
	friend void JNICALL Java_OffscreenView_nativeViewCreated(JNIEnv *, jobject, jlong param);
	friend void JNICALL Java_OffscreenView_onVisibleRect(JNIEnv *, jobject, jlong param, int left, int top, int right, int bottom);
};

int QColorToAndroidColor(const QColor & color);


