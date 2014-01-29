#pragma once
#include <QSharedPointer>
#include <QFocusEvent>
#include <QtQuick/QQuickFramebufferObject>
#include <QAndroidOffscreenWebView.h>

/*!
 * Base class for any Android offscreen view.
 */
class QQuickAndroidOffscreenView : public QQuickFramebufferObject
{
    Q_OBJECT
public:
	//!\ todo pass QAndroidOffscreenView
	QQuickAndroidOffscreenView();
	Renderer * createRenderer() const;

	virtual void setVisibe(bool visible);

protected:
	virtual void focusInEvent(QFocusEvent * event);
	virtual void focusOutEvent(QFocusEvent * event);
	virtual void mouseMoveEvent(QMouseEvent * event);
	virtual void mousePressEvent(QMouseEvent * event);
	virtual void mouseReleaseEvent(QMouseEvent * event);

public slots:
	void updateAndroidViewVisibility();

signals:
	void textureUpdated();

protected slots:
	void onTextureUpdated();

protected:
	//!\ todo pointer to QAndroidOffscreenView!
	QSharedPointer<QAndroidOffscreenWebView> aview_;
	bool is_interactive_;
	bool mouse_tracking_;
};

/*!
 * Base class for renderer of any Android offscreen view.
 */
class QAndroidOffscreenViewRenderer : public QObject, public QQuickFramebufferObject::Renderer
{
	Q_OBJECT
public:
	QAndroidOffscreenViewRenderer(QSharedPointer<QAndroidOffscreenWebView> aview);
	void render();
	QOpenGLFramebufferObject * createFramebufferObject(const QSize &size);
protected slots:
	void onTextureUpdated();
protected:
	QSharedPointer<QAndroidOffscreenWebView> aview_;
};

