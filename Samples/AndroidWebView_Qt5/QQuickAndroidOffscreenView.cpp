#include <QtGui/QOpenGLFramebufferObject>
#include <QtQuick/QQuickWindow>
#include "QQuickAndroidOffscreenView.h"

QQuickAndroidOffscreenView::QQuickAndroidOffscreenView(QAndroidOffscreenView * aview)
	: aview_(aview)
	, is_interactive_(true) // TODO
	, mouse_tracking_(false)
{
	connect(aview_.data(), SIGNAL(updated()), this, SLOT(onTextureUpdated()));
	setAcceptedMouseButtons(Qt::LeftButton);

	aview_->setAttachingMode(is_interactive_);
}

QQuickFramebufferObject::Renderer * QQuickAndroidOffscreenView::createRenderer() const
{
	QAndroidOffscreenViewRenderer * renderer = new QAndroidOffscreenViewRenderer(aview_);
	connect(this, SIGNAL(textureUpdated()), renderer, SLOT(onTextureUpdated()));

	const_cast<QQuickAndroidOffscreenView*>(this)->updateAndroidViewVisibility();

	return renderer;
}

void QQuickAndroidOffscreenView::onTextureUpdated()
{
	emit textureUpdated();
}

void QQuickAndroidOffscreenView::setVisibe(bool visible)
{
	QQuickFramebufferObject::setVisible(visible);
	updateAndroidViewVisibility();
}

void QQuickAndroidOffscreenView::focusInEvent(QFocusEvent * event)
{
	QQuickFramebufferObject::focusInEvent(event);
	aview_->setFocused(true);
}

void QQuickAndroidOffscreenView::focusOutEvent(QFocusEvent * event)
{
	QQuickFramebufferObject::focusOutEvent(event);
	aview_->setFocused(false);
}

void QQuickAndroidOffscreenView::mouseMoveEvent(QMouseEvent * event)
{
	qDebug()<<__PRETTY_FUNCTION__<<"***************************************";
	if (is_interactive_ && mouse_tracking_)
	{
		QPoint pos = event->pos();
		aview_->mouse(QAndroidOffscreenView::ANDROID_MOTIONEVENT_ACTION_MOVE, pos.x(), pos.y());
		event->accept();
	}
}

void QQuickAndroidOffscreenView::mousePressEvent(QMouseEvent * event)
{
	if (is_interactive_ && event->button() == Qt::LeftButton)
	{
		QPoint pos = event->pos();
		aview_->mouse(QAndroidOffscreenView::ANDROID_MOTIONEVENT_ACTION_DOWN, pos.x(), pos.y());
		mouse_tracking_ = true;
		event->accept();
	}
}

void QQuickAndroidOffscreenView::mouseReleaseEvent(QMouseEvent * event)
{
	if (is_interactive_ && event->button() == Qt::LeftButton)
	{
		QPoint pos = event->pos();
		aview_->mouse(QAndroidOffscreenView::ANDROID_MOTIONEVENT_ACTION_UP, pos.x(), pos.y());
		mouse_tracking_ = false;
		event->accept();
	}
}

void QQuickAndroidOffscreenView::updateAndroidViewVisibility()
{
	aview_->setVisible(isVisible() && opacity() > 0);
}



QAndroidOffscreenViewRenderer::QAndroidOffscreenViewRenderer(QSharedPointer<QAndroidOffscreenView> aview)
	: aview_(aview)
{
}

void QAndroidOffscreenViewRenderer::render()
{
	// We can't use the texture in aview_ directly because it uses GL shader extension
	// and custom transformation matrix, but we can draw the texture on the FBO texture.
	// Fortunately, this is an extremely small operation comparing to the everything else
	// we have to do.
	aview_->paintGL(0, 0, aview_->size().width(), aview_->size().height(), true);
	update();
}

QOpenGLFramebufferObject * QAndroidOffscreenViewRenderer::createFramebufferObject(const QSize &size)
{
	aview_->initializeGL();
	aview_->resize(size);
	QOpenGLFramebufferObjectFormat format;
	format.setSamples(4);
	return new QOpenGLFramebufferObject(size, format);
}

void QAndroidOffscreenViewRenderer::onTextureUpdated()
{
	// qDebug()<<__FUNCTION__<<"!!!!!!!!!!!!! ***** !!!!!!!!!!!!! ***** !!!!!!!!!!!!!! **** !!!!!!!!!!!!!!";
	invalidateFramebufferObject();
}







QQuickAndroidOffscreenWebView::QQuickAndroidOffscreenWebView()
	: QQuickAndroidOffscreenView(new QAndroidOffscreenWebView("WebViewInQuick", QSize(512, 512)))
{
	// SGEXP
	androidWebView()->loadUrl("http://www.android.com/intl/en/about/");
}


QQuickAndroidOffscreenEditText::QQuickAndroidOffscreenEditText()
	: QQuickAndroidOffscreenView(new QAndroidOffscreenEditText("EditTextInQuick", QSize(512, 64)))
{
	// SGEXP
	androidView()->setFillColor(Qt::yellow);
}








