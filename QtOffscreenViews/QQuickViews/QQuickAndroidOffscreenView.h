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
#include <QtCore/QSharedPointer>
#include <QtGui/QFocusEvent>
#include <QtQuick/QQuickItem>
#include <QtOffscreenViews/QAndroidOffscreenView.h>

/*!
 * Base class for any Android offscreen view.
 */
class QQuickAndroidOffscreenView : public QQuickItem
{
	Q_OBJECT
	Q_PROPERTY(QColor backgroundColor READ getBackgroundColor WRITE setBackgroundColor NOTIFY backgroundColorChanged)

public:
	QQuickAndroidOffscreenView(QAndroidOffscreenView * aview);

	QColor getBackgroundColor() const { return androidView()->fillColor(); }
	void setBackgroundColor(const QColor & color);

public slots:
	/*!
	 * This function must be called from QML after screen position has been changed
	 * to make sure that any Android's overlay controls are placed correctly.
	 * Unfortunately, Quick doesn't seem to allow to track change of absolute item
	 * coordinates - only its x/y within the parent.
	 */
	void updateAndroidViewPosition();

	void requestVisibleRect();

	//! Make sure software keyboard is hidden for this control.
	void hideKeyboard() { androidView()->hideKeyboard(); }

	//! Make sure software keyboard is shown for this control.
	void showKeyboard() { androidView()->showKeyboard(); }

	void setHideKeyboardOnFocusLoss(bool hide) { androidView()->setHideKeyboardOnFocusLoss(hide); }

	void setSoftInputModeResize() { androidView()->setSoftInputModeResize(); }
	void setSoftInputModeAdjustPan() { androidView()->setSoftInputModeAdjustPan(); }
	void setSoftInputModeAdjustNothing() { androidView()->setSoftInputModeAdjustNothing(); }

	// A workaround test function, do not use
	void reattachView() { androidView()->reattachView(); }

	// Direct control on Android widget focusing. This typically should not be called
	// because focusing is automatically done in focusInEvent()/focusOutEvent().
	void setFocused(bool focused) { androidView()->setFocused(focused); }

	void setScrollX(int x) { androidView()->setScrollX(x); }
	void setScrollY(int y) { androidView()->setScrollY(y); }

	// Test function for lib developers, don't use it
	void testFunction() { androidView()->testFunction(); }

signals:
	void viewCreated();
	void backgroundColorChanged(QColor color);
	void visibleRectReceived(int visible_width, int visible_height);

protected:
	QAndroidOffscreenView * androidView() { return aview_.data(); }
	const QAndroidOffscreenView * androidView() const { return aview_.data(); }

	virtual void geometryChanged(const QRectF & new_geometry, const QRectF & old_geometry);
	virtual void focusInEvent(QFocusEvent * event);
	virtual void focusOutEvent(QFocusEvent * event);
	virtual void mouseMoveEvent(QMouseEvent * event);
	virtual void mousePressEvent(QMouseEvent * event);
	virtual void mouseReleaseEvent(QMouseEvent * event);
	virtual void itemChange(ItemChange change, const ItemChangeData & value);

	QSGNode * updatePaintNode(QSGNode * node, UpdatePaintNodeData * nodedata);

protected slots:
	virtual void updateAndroidViewVisibility();
	virtual void updateAndroidEnabled();
	virtual void onTextureUpdated();
	virtual void onVisibleRectReceived(int width, int height);
	virtual void onViewCreated();

private:
	QSharedPointer<QAndroidOffscreenView> aview_;
	bool is_interactive_;
	bool mouse_tracking_;
	bool redraw_texture_needed_;
	QPoint last_set_position_;
};
