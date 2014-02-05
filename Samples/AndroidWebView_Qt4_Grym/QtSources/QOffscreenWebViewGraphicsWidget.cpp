#include <jni.h>
#include <unistd.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <stdlib.h>
#include <QtOpenGL>
#include <QGLWidget>
#include "QOffscreenWebViewGraphicsWidget.h"

// #define ANDROIDVIEWGRAPHICSPROXY_CLEARALL

QAndroidOffscreenViewGraphicsWidget::QAndroidOffscreenViewGraphicsWidget(QAndroidOffscreenView * view, bool interactive, QGraphicsItem *parent, Qt::WindowFlags wFlags)
	: QGraphicsWidget(parent, wFlags)
	, aview_(view)
	, mouse_tracking_(false)
	, last_updated_position_(0, 0)
	, initial_visibilty_set_(false)
	, is_interactive_(interactive)
{
	aview_->setAttachingMode(interactive);
	setAcceptedMouseButtons(Qt::LeftButton);
	setFocusPolicy(Qt::StrongFocus);
	connect(aview_.data(), SIGNAL(updated()), this, SLOT(onOffscreenUpdated()));
}

QAndroidOffscreenViewGraphicsWidget::~QAndroidOffscreenViewGraphicsWidget()
{
}

void QAndroidOffscreenViewGraphicsWidget::setVisible(bool visible)
{
	//! \todo Also take into account if the app is minimized or not
	aview_->setVisible(visible);
	QGraphicsWidget::setVisible(visible);
}

void QAndroidOffscreenViewGraphicsWidget::setEnabled(bool enabled)
{
	aview_->setEnabled(enabled);
	QGraphicsWidget::setEnabled(enabled);
}

void QAndroidOffscreenViewGraphicsWidget::onOffscreenUpdated()
{
	update();
}

void QAndroidOffscreenViewGraphicsWidget::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
	Q_UNUSED(option);
	Q_UNUSED(widget);
	bool use_gl = painter->paintEngine()->type() == QPaintEngine::OpenGL2;

	// We don't have any function like initializeGL in QGraphicsWidget, so let's just
	// do the initialization during the first paint.
	if (!aview_->isIntialized())
	{
#if 0
		if (aview_->openGlTextureSupported() && use_gl)
		{
			aview_->initializeGL();
		}
		else
#endif
		{
			aview_->initializeBitmap();
		}
	}

	if (!initial_visibilty_set_)
	{
		//! \todo Also take into account if the app is minimized or not
		aview_->setVisible(isVisible());
		initial_visibilty_set_ = true;
	}

	#if defined(ANDROIDVIEWGRAPHICSPROXY_CLEARALL)
		painter->fillRect(rect(), Qt::green);
	#endif

	painter->beginNativePainting();

	if (use_gl)
	{
		QPaintDevice *device = painter->device();
		QTransform combined_transform = painter->combinedTransform();

		Q_ASSERT(
			(combined_transform.type() == QTransform::TxNone) ||
			(combined_transform.type() == QTransform::TxTranslate) ||
			(combined_transform.type() == QTransform::TxScale));

		QRectF widget_geometry = combined_transform.mapRect(QRectF(QPointF(0, 0), size()));

		GLint l = static_cast<GLint>(widget_geometry.left() + 0.5);
		GLint b = static_cast<GLint>(device->height() - static_cast<int>(widget_geometry.bottom() + 0.5));
		GLsizei w = static_cast<GLsizei>(widget_geometry.width()+0.5);
		GLsizei h = static_cast<GLsizei>(widget_geometry.height()+0.5);

		QPoint viewpos = this->scene()->views().at(0)->pos();
		Q_UNUSED(viewpos);

		#if 0
			qDebug()
					<<__PRETTY_FUNCTION__
					<<"tid"<<gettid()
					<<"widget_geometry:"<<widget_geometry.left()<<widget_geometry.top()
					<<"("<<widget_geometry.width()<<"x"<<widget_geometry.height()<<")"
					<<"right"<<widget_geometry.right()<<"bottom"<<widget_geometry.bottom()
					<<"//////// l"<<l<<"b"<<b<<"w"<<w<<"h"<<h
					<<"widget_pos"<<widget->pos().x()<<widget->pos().y()
					<<"viewpos"<<viewpos.x()<<viewpos.y();
		#endif

		l += widget->pos().x();
		b += widget->pos().y();
		//! \todo Take into account position of the view within the window
		w++;
		h++;

		glViewport(l, b, w, h);

		#if defined(ANDROIDVIEWGRAPHICSPROXY_CLEARALL)
			glEnable(GL_SCISSOR_TEST);
			glScissor(l, b, w, h);

			glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		#endif

		// Finally, we can draw the texture using these GL coordinates
		aview_->paintGL(l, b, w, h, false);

		#if defined(ANDROIDVIEWGRAPHICSPROXY_CLEARALL)
			glScissor(0, 0, static_cast<GLsizei>(device->width()), static_cast<GLsizei>(device->height()));
			glDisable(GL_SCISSOR_TEST);
		#endif

		// Reset viewport (is it necessary?)
		glViewport(0, 0, static_cast<GLsizei>(device->width()), static_cast<GLsizei>(device->height()));
		painter->endNativePainting();
	}
	else
	{
		const QImage * buffer = aview_->getBitmapBuffer();
		if (buffer)
		{
			painter->drawImage(0, 0, *buffer);
		}
		else
		{
			painter->fillRect(0, 0, size().width(), size().height(), aview_->fillColor());
		}
	}

	if (last_updated_position_ != absolutePosition())
	{
		updateViewPosition();
	}
}

void QAndroidOffscreenViewGraphicsWidget::mouseMoveEvent(QGraphicsSceneMouseEvent * event)
{
	if (is_interactive_ && mouse_tracking_)
	{
		QPoint pos = event->pos().toPoint();
		aview_->mouse(QAndroidOffscreenView::ANDROID_MOTIONEVENT_ACTION_MOVE, pos.x(), pos.y());
		event->accept();
	}
}

void QAndroidOffscreenViewGraphicsWidget::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
	if (is_interactive_ && event->button() == Qt::LeftButton)
	{
		QPoint pos = event->pos().toPoint();
		aview_->mouse(QAndroidOffscreenView::ANDROID_MOTIONEVENT_ACTION_DOWN, pos.x(), pos.y());
		mouse_tracking_ = true;
		event->accept();
	}
}

void QAndroidOffscreenViewGraphicsWidget::mouseReleaseEvent(QGraphicsSceneMouseEvent * event)
{
	if (is_interactive_ && event->button() == Qt::LeftButton)
	{
		QPoint pos = event->pos().toPoint();
		aview_->mouse(QAndroidOffscreenView::ANDROID_MOTIONEVENT_ACTION_UP, pos.x(), pos.y());
		mouse_tracking_ = false;
		event->accept();
	}
}

void QAndroidOffscreenViewGraphicsWidget::resizeEvent(QGraphicsSceneResizeEvent *event)
{
	qDebug()<<__PRETTY_FUNCTION__<<event->newSize().toSize();
	QGraphicsWidget::resizeEvent(event);
	aview_->resize(event->newSize().toSize());
}

void QAndroidOffscreenViewGraphicsWidget::moveEvent(QGraphicsSceneMoveEvent * event)
{
	qDebug()<<__PRETTY_FUNCTION__<<event->newPos();
	QGraphicsWidget::moveEvent(event);
	updateViewPosition();
}

QPoint QAndroidOffscreenViewGraphicsWidget::absolutePosition() const
{
	QList<QGraphicsView*> views = scene()->views();
	if (views.size() >= 1)
	{
		if (views.size() > 1)
		{
			qWarning()<<__PRETTY_FUNCTION__<<"The scene has multiple views, the View may be positioned improperly!";
		}
		QGraphicsView * view = views.at(0);
		QPointF scenepos = this->pos();
		QPointF inviewpos = view->mapFromScene(scenepos);
		QPoint abspos = view->mapToGlobal(inviewpos.toPoint());
		// qDebug()<<__PRETTY_FUNCTION__<<"ScenePos:"<<scenepos<<"InViewPos:"<<inviewpos<<"AbsPos:"<<abspos;
		return abspos;
	}
	return last_updated_position_;
}

void QAndroidOffscreenViewGraphicsWidget::updateViewPosition()
{
	QPoint abspos = absolutePosition();
	aview_->setPosition(abspos.x(), abspos.y());
	last_updated_position_ = abspos;
}

void QAndroidOffscreenViewGraphicsWidget::focusInEvent (QFocusEvent * event)
{
	qDebug()<<__PRETTY_FUNCTION__;
	aview_->setFocused(true);
	QGraphicsWidget::focusInEvent(event);
}

void QAndroidOffscreenViewGraphicsWidget::focusOutEvent (QFocusEvent * event)
{
	qDebug()<<__PRETTY_FUNCTION__;
	aview_->setFocused(false);
	QGraphicsWidget::focusOutEvent(event);
}






QOffscreenWebViewGraphicsWidget::QOffscreenWebViewGraphicsWidget(const QString & objectname, bool interactive, const QSize & def_size, QGraphicsItem *parent, Qt::WindowFlags wFlags)
	: QAndroidOffscreenViewGraphicsWidget(new QAndroidOffscreenWebView(objectname, def_size), interactive, parent, wFlags)

{
	connect(androidOffscreenView(), SIGNAL(pageFinished()), this, SLOT(onPageFinished()));
	connect(androidOffscreenView(), SIGNAL(contentHeightReceived(int)), this, SLOT(onContentHeightReceived(int)));

	// This is not necessary anymore, as the view will schedule any actions for execution when the
	// view is ready:
	// aview_->waitForViewCreation();
	// androidOffscreenWebView()->loadUrl("http://www.android.com/intl/en/about/");
	androidOffscreenWebView()->loadData(
		"<html>"
		"<body>"
		"<h1>Hello WebView</h1>"
		"<p>Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.</p>"
		"<p>Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.</p>"
		"<p>Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.</p>"
		"<p>Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.</p>"
		"<p>Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.</p>"
		"<p>Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.</p>"
		"<p>Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.</p>"
		"</body>"
		"</html>"
	);
}

void QOffscreenWebViewGraphicsWidget::onPageFinished()
{
	qDebug()<<__PRETTY_FUNCTION__;
	androidOffscreenWebView()->requestContentHeight();
}

void QOffscreenWebViewGraphicsWidget::onContentHeightReceived(int height)
{
	qDebug()<<__PRETTY_FUNCTION__<<"Page height:"<<height;
}




QOffscreenEditTextGraphicsWidget::QOffscreenEditTextGraphicsWidget(const QString objectname, const QSize & def_size, QGraphicsItem *parent, Qt::WindowFlags wFlags)
	: QAndroidOffscreenViewGraphicsWidget(new QAndroidOffscreenEditText(objectname, def_size), true, parent, wFlags)
{
}
