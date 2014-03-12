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

#include "QAndroidOffscreenViewGraphicsWidget.h"

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
	// qDebug()<<__PRETTY_FUNCTION__<<<<aview_->viewObjectName();
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
		if (use_gl)
		{
			aview_->initializeGL();
		}
		else
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

#if 1 // (Enable/disable texture-based drawing)
	if (use_gl)
	{
		painter->beginNativePainting();
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

		// Finally, we can draw the texture using these GL coordinates
		aview_->paintGL(l, b, w, h, false);

		// Reset viewport (is it necessary?)
		glViewport(0, 0, static_cast<GLsizei>(device->width()), static_cast<GLsizei>(device->height()));
		painter->endNativePainting();
	}
	else
#endif
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
	if (scene())
	{
		QList<QGraphicsView*> views = scene()->views();
		if (views.size() >= 1)
		{
			if (views.size() > 1)
			{
				qWarning()<<__PRETTY_FUNCTION__<<"The scene has multiple views, the View may be positioned improperly!";
			}
			QGraphicsView * view = views.at(0);
			QPointF scenepos = this->scenePos();
			QPointF inviewpos = view->mapFromScene(scenepos);
			QPoint abspos = view->mapToGlobal(inviewpos.toPoint());
			// qDebug()<<__PRETTY_FUNCTION__<<"ScenePos:"<<scenepos<<"InViewPos:"<<inviewpos<<"AbsPos:"<<abspos;
			return abspos;
		}
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

