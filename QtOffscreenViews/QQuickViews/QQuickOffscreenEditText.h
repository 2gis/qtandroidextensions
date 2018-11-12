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
#include "QQuickAndroidOffscreenView.h"
#include <QtOffscreenViews/QAndroidOffscreenEditText.h>

class QQuickAndroidOffscreenEditText: public QQuickAndroidOffscreenView
{
	Q_OBJECT
	Q_PROPERTY(QString text READ getText WRITE setText NOTIFY onTextChanged)
public:
	QQuickAndroidOffscreenEditText();

	QString getText() const { return androidEditText()->getText(); }

protected:
	QAndroidOffscreenEditText * androidEditText() { return static_cast<QAndroidOffscreenEditText*>(androidView()); }
	const QAndroidOffscreenEditText * androidEditText() const { return static_cast<const QAndroidOffscreenEditText*>(androidView()); }

public slots:
	void setText(QString text) { androidEditText()->setText(text); }

	//! Set the default text size to a given unit (ANDROID_TYPEDVALUE_...) and value.
	void setTextSize(float size, int unit) { androidEditText()->setTextSize(size, unit); }

	//! Set text size in raw pixels.
	void setTextSize(float size) { androidEditText()->setTextSize(size); }

	/*! Sets the typeface and style (ANDROID_TYPEFACE_...) in which the text should be displayed.
		\param name can be "normal", "sans", "serif", "monospace". */
	void setTypeface(QString name, int style) { androidEditText()->setTypeface(name, style); }

	void setTypeface(QString name) { androidEditText()->setTypeface(name); }

	//! Creates Typeface from file and sets it.
	void setTypefaceFromFile(const QString & filename, int style) { androidEditText()->setTypefaceFromFile(filename, style); }

	void setTypefaceFromFile(const QString & filename) { androidEditText()->setTypefaceFromFile(filename); }

	//! Creates Typeface from asset file and sets it.
	void setTypefaceFromAsset(const QString & filename, int style) { androidEditText()->setTypefaceFromAsset(filename, style); }

	void setTypefaceFromAsset(const QString & filename) { androidEditText()->setTypefaceFromAsset(filename); }

	//! Set whether the cursor is visible.
	void setCursorVisible(bool visible) { androidEditText()->setCursorVisible(visible); }

	//! Set the type of the content with a constant as defined for inputType (ANDROID_INPUTTYPE_...)
	void setInputType(int type) { androidEditText()->setInputType(type); }

	//! setInputType((getInputType() & type_and) | type_or)
	void setInputTypeAndOr(int type_and, int type_or) { androidEditText()->setInputType(type_and, type_or); }

	void setEnableNativeSuggestions(bool enable);

	//! Sets how many times to repeat the marquee animation.
	void setMarqueeRepeatLimit(int marqueeLimit) { androidEditText()->setMarqueeRepeatLimit(marqueeLimit); }

	//! Makes the TextView at most this many ems wide
	void setMaxEms(int maxems) { androidEditText()->setMaxEms(maxems); }

	//! Makes the TextView at least this many ems wide
	void setMinEms(int minems) { androidEditText()->setMinEms(minems); }

	//! Makes the TextView at most this many pixels tall.
	void setMaxHeight(int maxHeight) { androidEditText()->setMaxHeight(maxHeight); }

	//! Makes the TextView at least this many pixels tall.
	void setMinHeight(int minHeight) { androidEditText()->setMinHeight(minHeight); }

	//! Makes the TextView at most this many lines tall.
	void setMaxLines(int maxlines) { androidEditText()->setMaxLines(maxlines); }

	//! Makes the TextView at least this many lines tall.
	void setMinLines(int minlines) { androidEditText()->setMinLines(minlines); }

	//! Makes the TextView at most this many pixels wide
	void setMaxWidth(int maxpixels) { androidEditText()->setMaxWidth(maxpixels); }

	//! Makes the TextView at least this many pixels wide
	void setMinWidth(int minpixels) { androidEditText()->setMinWidth(minpixels); }

	//! Sets the padding.
	void setPadding(int left, int top, int right, int bottom) { androidEditText()->setPadding(left, top, right, bottom); }

	//! Sets flags on the Paint being used to display the text and reflows the text if they are different from the old flags.
	void setPaintFlags(int flags) { androidEditText()->setPaintFlags(flags); }

	//! Set the TextView so that when it takes focus, all the text is selected.
	void setSelectAllOnFocus(bool selectAllOnFocus) { androidEditText()->setSelectAllOnFocus(selectAllOnFocus); }

	//! If true, sets the properties of this field (number of lines, horizontally scrolling, transformation method) to be for a single-line input; if false, restores these to the default conditions.
	void setSingleLine(bool singleLine = true) { androidEditText()->setSingleLine(singleLine); }

	//! Sets the text color for all the states (normal, selected, focused) to be this color.
	void setTextColor(int color) { androidEditText()->setTextColor(color); }

	//! NB: to use QGlobalColor, do like this: setTextColor(QColor(Qt::red)).
	void setTextColor(const QColor & color) { androidEditText()->setTextColor(color); }

	//! Sets the extent by which text should be stretched horizontally.
	void setTextScaleX(float size) { androidEditText()->setTextScaleX(size); }

	//! Sets whether the content of this view is selectable by the user.
	void setTextIsSelectable(bool selectable) { androidEditText()->setTextIsSelectable(selectable); }

	//! Sets the horizontal alignment of the text and the vertical gravity that will be used when there is extra space in the TextView beyond what is required for the text itself.
	void setGravity(int gravity) { androidEditText()->setGravity(gravity); }

	// //! Makes the TextView exactly this many pixels tall.
	// void setHeight(int pixels) { androidEditText()->setHeight(pixels); }

	//! Sets the color used to display the selection highlight.
	void setHighlightColor(int color) { androidEditText()->setHighlightColor(color); }

	//! NB: to use QGlobalColor, do like this: setTextColor(QColor(Qt::red)).
	void setHighlightColor(const QColor & color) { androidEditText()->setHighlightColor(color); }

	//! Sets the text to be displayed when the text of the TextView is empty.
	void setHint(const QString & hint) { androidEditText()->setHint(hint); }

	//! Sets the color of the hint text for all the states (disabled, focussed, selected...) of this TextView.
	void setHintTextColor(int color) { androidEditText()->setHintTextColor(color); }

	//! NB: to use QGlobalColor, do like this: setTextColor(QColor(Qt::red)).
	void setHintTextColor(const QColor & color) { androidEditText()->setHintTextColor(color); }

	// //! Makes the TextView exactly this many pixels wide.
	// void setWidth(int pixels) { androidEditText()->setWidth(pixels); }

	//! Sets line spacing for this TextView.
	void setLineSpacing(float add, float mult) { androidEditText()->setLineSpacing(add, mult); }

	//! Makes the TextView exactly this many lines tall.
	void setLines(int lines) { androidEditText()->setLines(lines); }

	//! Sets whether the text should be allowed to be wider than the View is.
	void setHorizontallyScrolling(bool whether) { androidEditText()->setHorizontallyScrolling(whether); }

	//! Sets the properties of this field to transform input to ALL CAPS display.
	void setAllCaps(bool allCaps) { androidEditText()->setAllCaps(allCaps); }

	void selectAll() { androidEditText()->selectAll(); }

	void setSelection(int index) { androidEditText()->setSelection(index); }

	void setSelection(int start, int stop) { androidEditText()->setSelection(start, stop); }

	void setAllowFullscreenKeyboard(bool allow) { androidEditText()->setAllowFullscreenKeyboard(allow); }

	void setHorizontalScrollBarEnabled(bool horizontalScrollBarEnabled) { androidEditText()->setHorizontalScrollBarEnabled(horizontalScrollBarEnabled); }

	void setVerticalScrollBarEnabled (bool verticalScrollBarEnabled) { androidEditText()->setVerticalScrollBarEnabled(verticalScrollBarEnabled); }

	void setCursorColorToTextColor() { androidEditText()->setCursorColorToTextColor(); }

	void setMaxLength(int length) { androidEditText()->setMaxLength(length); }


	void setImeOptions(int and_mask, int or_mask) { androidEditText()->setImeOptions(and_mask, or_mask); }
	void setImeAction(int action) { androidEditText()->setImeAction(action); }
	void setImeActionUnspecified() { androidEditText()->setImeActionUnspecified(); }
	void setImeActionGo() { androidEditText()->setImeActionGo(); }
	void setImeActionSearch() { androidEditText()->setImeActionSearch(); }
	void setImeActionSend() { androidEditText()->setImeActionSend(); }
	void setImeActionNext() { androidEditText()->setImeActionNext(); }
	void setImeActionDone() { androidEditText()->setImeActionDone(); }
	void setImeActionPrevious() { androidEditText()->setImeActionPrevious(); }

	void setPasswordMode() { androidEditText()->setPasswordMode(); }
	void setPasswordModeWithDefaultTypeface(bool enable) { androidEditText()->setPasswordModeWithDefaultTypeface(enable); }

signals:
	void onTextChanged(QString text);

	//! Wraps QAndroidOffscreenEditText::onEnterOrPositiveAction().
	void returnPressed();

	void backPressed();

protected slots:
	virtual void etTextChanged(QString text, int start, int before, int count);
	virtual void etKeyBack(bool down);
	virtual void etEnter();
};

