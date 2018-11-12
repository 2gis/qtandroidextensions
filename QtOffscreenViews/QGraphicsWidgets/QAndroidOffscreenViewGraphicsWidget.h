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
#include <QGraphicsWidget>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsItem>
#include <QGraphicsSceneMoveEvent>
#include "QAndroidOffscreenView.h"

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
	virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);

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

