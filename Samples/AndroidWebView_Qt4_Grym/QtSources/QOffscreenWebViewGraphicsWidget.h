#pragma once
#include <QtOpenGL/QGLWidget>
#include <QTimer>
#include <QtGui/QGraphicsWidget>
#include <QPaintEvent>
#include <EGL/egl.h>
#include <QJniHelpers.h>
#include "QAndroidOffscreenWebView.h"
#include "QAndroidOffscreenEditText.h"

/*!
 * Base class for any QGraphicsWidget-based view at Android View.
 */
class QAndroidOffscreenViewGraphicsWidget
	: public QGraphicsWidget
{
	Q_OBJECT
public:
	QAndroidOffscreenViewGraphicsWidget(QAndroidOffscreenView * view, bool interactive, QGraphicsItem *parent = 0, Qt::WindowFlags wFlags = 0);
	virtual ~QAndroidOffscreenViewGraphicsWidget();

	virtual void setVisible(bool visible);
	virtual void setEnabled(bool enabled);

	QAndroidOffscreenView * androidOffscreenView() { return aview_.data(); }
	const QAndroidOffscreenView * androidOffscreenView() const { return aview_.data(); }

protected:
	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	virtual void mouseMoveEvent(QGraphicsSceneMouseEvent * event);
	virtual void mousePressEvent(QGraphicsSceneMouseEvent * event);
	virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent * event);
	virtual void resizeEvent(QGraphicsSceneResizeEvent *event);
	virtual void moveEvent(QGraphicsSceneMoveEvent * event);
	virtual void focusInEvent (QFocusEvent * event);
	virtual void focusOutEvent (QFocusEvent * event);

	QPoint absolutePosition() const;
	void updateViewPosition();

private slots:
	void onOffscreenUpdated();

private:
	QScopedPointer<QAndroidOffscreenView> aview_;
	bool mouse_tracking_;
	QPoint last_updated_position_;
	bool initial_visibilty_set_;
private:
	bool is_interactive_;
};

class QOffscreenWebViewGraphicsWidget
	: public QAndroidOffscreenViewGraphicsWidget
{
	Q_OBJECT
public:
	QOffscreenWebViewGraphicsWidget(
		const QString & objectname = QLatin1String("DefaultWebView"),
		bool interactive = true,
		const QSize & def_size = QSize(512, 512),
		QGraphicsItem *parent = 0,
		Qt::WindowFlags wFlags = 0);

	QAndroidOffscreenWebView * androidOffscreenWebView() { return static_cast<QAndroidOffscreenWebView*>(androidOffscreenView()); }
	const QAndroidOffscreenWebView * androidOffscreenWebView() const { return static_cast<const QAndroidOffscreenWebView*>(androidOffscreenView()); }

private slots:
	void onPageFinished();
	void onContentHeightReceived(int height);
};

class QOffscreenEditTextGraphicsWidget
	: public QAndroidOffscreenViewGraphicsWidget
{
	Q_OBJECT
public:
	QOffscreenEditTextGraphicsWidget(const QString objectname = QLatin1String("DefaultEditText"),
		const QSize & def_size = QSize(512, 32), QGraphicsItem *parent = 0, Qt::WindowFlags wFlags = 0);

};


