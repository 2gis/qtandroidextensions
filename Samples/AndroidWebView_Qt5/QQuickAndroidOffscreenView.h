#pragma once
#include <QSharedPointer>
#include <QFocusEvent>
#include <QtQuick/QQuickFramebufferObject>

#include <QAndroidOffscreenWebView.h>
#include <QAndroidOffscreenEditText.h>

/*!
 * Base class for any Android offscreen view.
 */
class QQuickAndroidOffscreenView : public QQuickFramebufferObject
{
    Q_OBJECT
public:
	//!\ todo pass QAndroidOffscreenView
	QQuickAndroidOffscreenView(QAndroidOffscreenView * aview);
	Renderer * createRenderer() const;

protected:
	QAndroidOffscreenView * androidView() { return aview_.data(); }
	const QAndroidOffscreenView * androidView() const { return aview_.data(); }

	virtual void focusInEvent(QFocusEvent * event);
	virtual void focusOutEvent(QFocusEvent * event);
	virtual void mouseMoveEvent(QMouseEvent * event);
	virtual void mousePressEvent(QMouseEvent * event);
	virtual void mouseReleaseEvent(QMouseEvent * event);
	virtual void itemChange(ItemChange change, const ItemChangeData & value);

public slots:
	void updateAndroidViewVisibility();
	void updateAndroidViewPosition();
	void updateAndroidEnabled();

signals:
	void textureUpdated();

protected slots:
	void onTextureUpdated();

private:
	QSharedPointer<QAndroidOffscreenView> aview_;
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
	QAndroidOffscreenViewRenderer(QSharedPointer<QAndroidOffscreenView> aview);
	void render();
	QOpenGLFramebufferObject * createFramebufferObject(const QSize & size);
protected slots:
	void onTextureUpdated();
protected:
	QSharedPointer<QAndroidOffscreenView> aview_;
};



class QQuickAndroidOffscreenWebView: public QQuickAndroidOffscreenView
{
	Q_OBJECT
public:
	QQuickAndroidOffscreenWebView();

	QAndroidOffscreenWebView * androidWebView() { return static_cast<QAndroidOffscreenWebView*>(androidView()); }
	const QAndroidOffscreenWebView * androidWebView() const { return static_cast<const QAndroidOffscreenWebView*>(androidView()); }
};


class QQuickAndroidOffscreenEditText: public QQuickAndroidOffscreenView
{
	Q_OBJECT
public:
	QQuickAndroidOffscreenEditText();

	QAndroidOffscreenEditText * androidEditText() { return static_cast<QAndroidOffscreenEditText*>(androidView()); }
	const QAndroidOffscreenEditText * androidEditText() const { return static_cast<const QAndroidOffscreenEditText*>(androidView()); }
};

