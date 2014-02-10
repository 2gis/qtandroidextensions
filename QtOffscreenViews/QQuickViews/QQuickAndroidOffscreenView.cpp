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

#include <QtGui/QOpenGLFramebufferObject>
#include <QtQuick/QQuickWindow>
#include "QQuickAndroidOffscreenView.h"

QQuickAndroidOffscreenView::QQuickAndroidOffscreenView(QAndroidOffscreenView * aview)
	: aview_(aview)
	, is_interactive_(true) // TODO
	, mouse_tracking_(false)
{
	connect(aview_.data(), SIGNAL(updated()), this, SLOT(onTextureUpdated()));
	connect(this, SIGNAL(xChanged()), this, SLOT(updateAndroidViewPosition()));
	connect(this, SIGNAL(yChanged()), this, SLOT(updateAndroidViewPosition()));
	connect(this, SIGNAL(enabledChanged()), this, SLOT(updateAndroidEnabled()));
	connect(this, SIGNAL(visibleChanged()), this, SLOT(updateAndroidViewVisibility()));

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
		// We must take focus here, or interactive View will not work properly.
		// Note that this happens only if the Quick item is interactive.
		setFocus(true);
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

void QQuickAndroidOffscreenView::itemChange(QQuickItem::ItemChange change, const QQuickItem::ItemChangeData & value)
{
	/*
	We would like to do this and setFlag(ItemSendsScenePositionChanges, true) in constructor,
	but it's not available in Qt Quick 2.
	if (change == QQuickItem::ItemScenePositionHasChanged && scene())
	{
		QMetaObject::invokeMethod(this, "updateAndroidViewPosition", Qt::QueuedConnection);
	}*/
	return QQuickFramebufferObject::itemChange(change, value);
}

void QQuickAndroidOffscreenView::updateAndroidViewVisibility()
{
	bool vis = isVisible() && opacity() > 0;
	qDebug()<<__PRETTY_FUNCTION__<<"Visible:"<<vis<<"***************************************";
	aview_->setVisible(vis);
}

void QQuickAndroidOffscreenView::updateAndroidViewPosition()
{
	qDebug()<<__PRETTY_FUNCTION__<<x()<<y();
	QPointF scenepos = mapToScene(QPointF(x(), y()));
	aview_->setPosition(qRound(scenepos.x()), qRound(scenepos.y()));
}

void QQuickAndroidOffscreenView::updateAndroidEnabled()
{
	qDebug()<<__PRETTY_FUNCTION__<<isEnabled();
	aview_->setEnabled(isEnabled());
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

QOpenGLFramebufferObject * QAndroidOffscreenViewRenderer::createFramebufferObject(const QSize & size)
{
	qDebug()<<__FUNCTION__<<"Creating new surface, size ="<<size;
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
