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

#include <QtGui/QtGui>
#include <QtQuick/QSGTransformNode>
#include <QtQuick/QSGSimpleTextureNode>
#include <QtQuick/QQuickWindow>
#include "QQuickAndroidOffscreenView.h"

namespace {

class TexureHolderNode
	: public QSGSimpleTextureNode
{
public:
	TexureHolderNode() {}
	QScopedPointer<QOpenGLFramebufferObject> fbo_;
};

static void ClearOpenGLState()
{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	GLint maxAttribs;
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxAttribs);
	for (GLuint i = 0; GLint(i) < maxAttribs; ++i)
	{
		glVertexAttribPointer(i, 4, GL_FLOAT, GL_FALSE, 0, 0); //-V112
		glDisableVertexAttribArray(i);
	}

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_STENCIL_TEST);
	glDisable(GL_SCISSOR_TEST);

	glDepthMask(true);
	glDepthFunc(GL_LESS);
	glClearDepthf(1);

	glStencilMask(0xff);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	glStencilFunc(GL_ALWAYS, 0, 0xff);

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);

	glUseProgram(0);
}

} // anonymous namespace




QQuickAndroidOffscreenView::QQuickAndroidOffscreenView(QAndroidOffscreenView * aview)
	: aview_(aview)
	, is_interactive_(true) // TODO
	, mouse_tracking_(false)
	, redraw_texture_needed_(true)
	, last_set_position_(0, 0) // View always at (0, 0) by default.
{
	setFlag(QQuickItem::ItemHasContents, true);
	setAcceptedMouseButtons(Qt::LeftButton);
	connect(aview_.data(), SIGNAL(updated()), this, SLOT(onTextureUpdated()));
	connect(this, SIGNAL(xChanged()), this, SLOT(updateAndroidViewPosition()));
	connect(this, SIGNAL(yChanged()), this, SLOT(updateAndroidViewPosition()));
	connect(this, SIGNAL(enabledChanged()), this, SLOT(updateAndroidEnabled()));
	connect(this, SIGNAL(visibleChanged()), this, SLOT(updateAndroidViewVisibility()));
	connect(aview_.data(), SIGNAL(visibleRectReceived(int,int)), this, SLOT(onVisibleRectReceived(int,int)));
	connect(aview_.data(), SIGNAL(viewCreated()), this, SLOT(onViewCreated()));
	aview_->setAttachingMode(is_interactive_);
}

void QQuickAndroidOffscreenView::setBackgroundColor(const QColor & color)
{
	if (color != androidView()->fillColor())
	{
		androidView()->setFillColor(color);
		emit backgroundColorChanged(color);
		redraw_texture_needed_ = true;
		update();
	}
}

void QQuickAndroidOffscreenView::onTextureUpdated()
{
	redraw_texture_needed_ = true;
	QMetaObject::invokeMethod(this, "update", Qt::AutoConnection);
}

void QQuickAndroidOffscreenView::onVisibleRectReceived(int width, int height)
{
	emit visibleRectReceived(width, height);
}

void QQuickAndroidOffscreenView::geometryChanged(const QRectF & new_geometry, const QRectF & old_geometry)
{
	QQuickItem::geometryChanged(new_geometry, old_geometry);
	if (new_geometry.size() != old_geometry.size())
	{
		aview_->resize(new_geometry.size().toSize());
	}
}

void QQuickAndroidOffscreenView::focusInEvent(QFocusEvent * event)
{
	// qDebug()<<__PRETTY_FUNCTION__;
	QQuickItem::focusInEvent(event);
	aview_->setFocused(true);
}

void QQuickAndroidOffscreenView::focusOutEvent(QFocusEvent * event)
{
	// qDebug()<<__PRETTY_FUNCTION__;
	QQuickItem::focusOutEvent(event);
	aview_->setFocused(false);
}

void QQuickAndroidOffscreenView::mouseMoveEvent(QMouseEvent * event)
{
	if (is_interactive_ && mouse_tracking_)
	{
		// qDebug()<<__PRETTY_FUNCTION__;
		QPoint pos = event->pos();
		aview_->mouse(QAndroidOffscreenView::ANDROID_MOTIONEVENT_ACTION_MOVE, pos.x(), pos.y());
		event->accept();
	}
}

void QQuickAndroidOffscreenView::mousePressEvent(QMouseEvent * event)
{
	if (is_interactive_ && event->button() == Qt::LeftButton)
	{
		// qDebug()<<__PRETTY_FUNCTION__;
		// We must take focus here, or interactive View will not work properly.
		// Note that this happens only if the Quick item is interactive.
		setFocus(true);
		forceActiveFocus(Qt::MouseFocusReason);
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
		// qDebug()<<"<<__PRETTY_FUNCTION__;
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
	return QQuickItem::itemChange(change, value);
}

QSGNode * QQuickAndroidOffscreenView::updatePaintNode(QSGNode * node, UpdatePaintNodeData * nodedata)
{
	Q_UNUSED(nodedata);

	// Initialize GL if necessary
	if (aview_ && !aview_->isIntialized())
	{
		aview_->initializeGL();
		if (aview_->isIntialized())
		{
			updateAndroidViewVisibility();
			updateAndroidViewPosition();
			aview_->resize(QSize(static_cast<int>(width()), static_cast<int>(height())));
		}
	}

	// ********************************************************************************************
	// Note: in theory, we could feed our texture right into the node, but unfortunately the Qt
	// classes don't have the functionality to use a read-only texture id.
	// So this would require writing our own implementation of textured SG node with black jack
	// and kittens from scratch.
	// As I don't have time for that right now, let's just live with drawing our texture over Qt
	// texture. It doesn't cost that much performance.
	// ********************************************************************************************

	// Create our painting node
	TexureHolderNode * n = static_cast<TexureHolderNode *>(node);
	if (!n) // Мы ещё не создавали узел
	{
		if (width() <= 0 || height() <= 0)
		{
			return nullptr;
		}
		n = new TexureHolderNode();
	}

	// Remove buffer if it has wrong size
	if (n->fbo_ && (n->fbo_->width() != width() || n->fbo_->height() != height()))
	{
		n->fbo_.reset();
	}

	// Create new buffer if necessary
	if (n->fbo_.isNull())
	{
		QSize fboSize(qMax<int>(1, int(width())), qMax<int>(1, int(height())));
		QOpenGLFramebufferObjectFormat format;
		format.setAttachment(QOpenGLFramebufferObject::NoAttachment); // CombinedDepthStencil
		n->fbo_.reset(new QOpenGLFramebufferObject(fboSize, format));
		n->setTexture(
			window()->createTextureFromId(
				n->fbo_->texture(),
				n->fbo_->size(),
				QQuickWindow::TextureHasAlphaChannel));
		n->setFiltering(QSGTexture::Nearest);
		n->setRect(0, 0, width(), height());
		redraw_texture_needed_ = true;
	}

	if (redraw_texture_needed_)
	{
		redraw_texture_needed_ = false;
		n->fbo_->bind();
		{
			glViewport(0, 0, n->fbo_->width(), n->fbo_->height());
			ClearOpenGLState();

			QColor c = getBackgroundColor();
			glClearColor(c.redF(), c.greenF(), c.blueF(), c.alphaF());
			glClear(GL_COLOR_BUFFER_BIT);

			if (aview_)
			{
				aview_->paintGL(0, 0, static_cast<int>(width()), static_cast<int>(height()), true);
			}
		}
		n->fbo_->bindDefault();
		n->markDirty(QSGNode::DirtyMaterial);
	}

	return n;
}

void QQuickAndroidOffscreenView::updateAndroidViewVisibility()
{
	bool vis = isVisible() && opacity() > 0;
	qDebug()<<__PRETTY_FUNCTION__<<"Visible:"<<vis<<"***************************************";
	aview_->setVisible(vis);
}

void QQuickAndroidOffscreenView::updateAndroidViewPosition()
{
	// Mapping item's top left corner to scene coordinates, in a hope that
	// the Quick scene covers all Android window.
	// Note: x() and y() return coordinates of the item within parent, i.e. we don't
	// need them. We only need in-scene coordinates.
	QPointF scenepos = mapToScene(QPointF(0, 0));
	QPoint scenepos_i(qRound(scenepos.x()), qRound(scenepos.y()));

	// Calling setPosition only if position has actually changed, because
	// Java side has to schedule a relatively lenghty operation to handle the
	// request.
	if (last_set_position_ != scenepos_i)
	{
		// qDebug() << __PRETTY_FUNCTION__ << scenepos_i.x() << scenepos_i.y();
		aview_->setPosition(scenepos_i.x(), scenepos_i.y());
		last_set_position_ = scenepos_i;
	}
}

void QQuickAndroidOffscreenView::requestVisibleRect()
{
	aview_->requestVisibleRect();
}

void QQuickAndroidOffscreenView::updateAndroidEnabled()
{
	// qDebug() << __PRETTY_FUNCTION__ << isEnabled();
	aview_->setEnabled(isEnabled());
}

void QQuickAndroidOffscreenView::onViewCreated()
{
	qDebug() << __PRETTY_FUNCTION__;
	emit viewCreated();
}
