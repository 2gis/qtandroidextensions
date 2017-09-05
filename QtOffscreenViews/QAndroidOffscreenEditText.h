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
#include "QAndroidOffscreenView.h"

class QAndroidOffscreenEditText
	: public QAndroidOffscreenView
{
	Q_OBJECT
	Q_ENUMS(AndroidInputType)
public:
	QAndroidOffscreenEditText(const QString & object_name, const QSize & def_size, QObject * parent = 0);
	virtual ~QAndroidOffscreenEditText();

	/*!
	 * This function should be called from main() to make sure that it all will work from QML threads.
	 * For Qt4/Grym plugin this is not necessary if you use QAndroidOffscreenView functions from GUI thread.
	 */
	static void preloadJavaClasses();

	//
	// EditText functions
	//

	void setText(const QString & text);
	QString getText() const;

	// More constants: http://developer.android.com/reference/android/util/TypedValue.html
	static const int
		ANDROID_TYPEDVALUE_COMPLEX_UNIT_DIP	= 0x00000001,
		ANDROID_TYPEDVALUE_COMPLEX_UNIT_IN	= 0x00000004,
		ANDROID_TYPEDVALUE_COMPLEX_UNIT_MM	= 0x00000005,
		ANDROID_TYPEDVALUE_COMPLEX_UNIT_PT	= 0x00000003,
		ANDROID_TYPEDVALUE_COMPLEX_UNIT_PX	= 0x00000000,
		ANDROID_TYPEDVALUE_COMPLEX_UNIT_SP	= 0x00000002;

	//! Set the default text size to a given unit (ANDROID_TYPEDVALUE_...) and value.
	void setTextSize(float size, int unit = ANDROID_TYPEDVALUE_COMPLEX_UNIT_PX);

	// http://developer.android.com/reference/android/graphics/Typeface.html
	static const int
		ANDROID_TYPEFACE_BOLD			= 0x00000001,
		ANDROID_TYPEFACE_BOLD_ITALIC	= 0x00000003,
		ANDROID_TYPEFACE_ITALIC			= 0x00000002,
		ANDROID_TYPEFACE_NORMAL			= 0x00000000;

	/*! Sets the typeface and style (ANDROID_TYPEFACE_...) in which the text should be displayed.
		\param name can be "normal", "sans", "serif", "monospace". */
	void setTypeface(const QString & name, int style = ANDROID_TYPEFACE_NORMAL);

	//! Creates Typeface from file and sets it.
	void setTypefaceFromFile(const QString & filename, int style = ANDROID_TYPEFACE_NORMAL);

	//! Creates Typeface from asset file and sets it.
	void setTypefaceFromAsset(const QString & filename, int style = ANDROID_TYPEFACE_NORMAL);

	//! Set whether the cursor is visible.
	void setCursorVisible(bool visible);

	// http://developer.android.com/reference/android/text/InputType.html
	enum AndroidInputType
	{
		ANDROID_INPUTTYPE_TYPE_CLASS_DATETIME					= 0x00000004,
		ANDROID_INPUTTYPE_TYPE_CLASS_NUMBER						= 0x00000002,
		ANDROID_INPUTTYPE_TYPE_CLASS_PHONE						= 0x00000003,
		ANDROID_INPUTTYPE_TYPE_CLASS_TEXT						= 0x00000001,
		ANDROID_INPUTTYPE_TYPE_DATETIME_VARIATION_DATE			= 0x00000010,
		ANDROID_INPUTTYPE_TYPE_DATETIME_VARIATION_NORMAL		= 0x00000000,
		ANDROID_INPUTTYPE_TYPE_DATETIME_VARIATION_TIME			= 0x00000020,
		ANDROID_INPUTTYPE_TYPE_MASK_CLASS						= 0x0000000f,
		ANDROID_INPUTTYPE_TYPE_MASK_FLAGS						= 0x00fff000,
		ANDROID_INPUTTYPE_TYPE_MASK_VARIATION					= 0x00000ff0,
		ANDROID_INPUTTYPE_TYPE_NULL								= 0x00000000,
		ANDROID_INPUTTYPE_TYPE_NUMBER_FLAG_DECIMAL				= 0x00002000,
		ANDROID_INPUTTYPE_TYPE_NUMBER_FLAG_SIGNED				= 0x00001000,
		ANDROID_INPUTTYPE_TYPE_NUMBER_VARIATION_NORMAL			= 0x00000000,
		ANDROID_INPUTTYPE_TYPE_NUMBER_VARIATION_PASSWORD		= 0x00000010,
		ANDROID_INPUTTYPE_TYPE_TEXT_FLAG_AUTO_COMPLETE			= 0x00010000,
		ANDROID_INPUTTYPE_TYPE_TEXT_FLAG_AUTO_CORRECT			= 0x00008000,
		ANDROID_INPUTTYPE_TYPE_TEXT_FLAG_CAP_CHARACTERS			= 0x00001000,
		ANDROID_INPUTTYPE_TYPE_TEXT_FLAG_CAP_SENTENCES			= 0x00004000,
		ANDROID_INPUTTYPE_TYPE_TEXT_FLAG_CAP_WORDS				= 0x00002000,
		ANDROID_INPUTTYPE_TYPE_TEXT_FLAG_IME_MULTI_LINE			= 0x00040000,
		ANDROID_INPUTTYPE_TYPE_TEXT_FLAG_MULTI_LINE				= 0x00020000,
		ANDROID_INPUTTYPE_TYPE_TEXT_FLAG_NO_SUGGESTIONS			= 0x00080000,
		ANDROID_INPUTTYPE_TYPE_TEXT_VARIATION_EMAIL_ADDRESS		= 0x00000020,
		ANDROID_INPUTTYPE_TYPE_TEXT_VARIATION_EMAIL_SUBJECT		= 0x00000030,
		ANDROID_INPUTTYPE_TYPE_TEXT_VARIATION_FILTER			= 0x000000b0,
		ANDROID_INPUTTYPE_TYPE_TEXT_VARIATION_LONG_MESSAGE		= 0x00000050,
		ANDROID_INPUTTYPE_TYPE_TEXT_VARIATION_NORMAL			= 0x00000000,
		ANDROID_INPUTTYPE_TYPE_TEXT_VARIATION_PASSWORD			= 0x00000080,
		ANDROID_INPUTTYPE_TYPE_TEXT_VARIATION_PERSON_NAME		= 0x00000060,
		ANDROID_INPUTTYPE_TYPE_TEXT_VARIATION_PHONETIC			= 0x000000c0,
		ANDROID_INPUTTYPE_TYPE_TEXT_VARIATION_POSTAL_ADDRESS	= 0x00000070,
		ANDROID_INPUTTYPE_TYPE_TEXT_VARIATION_SHORT_MESSAGE		= 0x00000040,
		ANDROID_INPUTTYPE_TYPE_TEXT_VARIATION_URI				= 0x00000010,
		ANDROID_INPUTTYPE_TYPE_TEXT_VARIATION_VISIBLE_PASSWORD	= 0x00000090,
		ANDROID_INPUTTYPE_TYPE_TEXT_VARIATION_WEB_EDIT_TEXT		= 0x000000a0,
		ANDROID_INPUTTYPE_TYPE_TEXT_VARIATION_WEB_EMAIL_ADDRESS	= 0x000000d0,
		ANDROID_INPUTTYPE_TYPE_TEXT_VARIATION_WEB_PASSWORD		= 0x000000e0
	};

	//! Set the type of the content with a constant as defined for inputType (ANDROID_INPUTTYPE_...)
	void setInputType(int type);

	//! setInputType((getInputType() & type_and) | type_or)
	void setInputType(int type_and, int type_or);

	//! Sets how many times to repeat the marquee animation.
	void setMarqueeRepeatLimit(int marqueeLimit);

	//! Makes the TextView at most this many ems wide
	void setMaxEms(int maxems);

	//! Makes the TextView at least this many ems wide
	void setMinEms(int minems);

	//! Makes the TextView at most this many pixels tall.
	void setMaxHeight(int maxHeight);

	//! Makes the TextView at least this many pixels tall.
	void setMinHeight(int minHeight);

	//! Makes the TextView at most this many lines tall.
	void setMaxLines(int maxlines);

	//! Makes the TextView at least this many lines tall.
	void setMinLines(int minlines);

	//! Makes the TextView at most this many pixels wide
	void setMaxWidth(int maxpixels);

	//! Makes the TextView at least this many pixels wide
	void setMinWidth(int minpixels);

	//! Sets the padding.
	void setPadding(int left, int top, int right, int bottom);

	// http://developer.android.com/reference/android/graphics/Paint.html
	static const int
		ANDROID_PAINT_ANTI_ALIAS_FLAG			= 0x00000001,
		ANDROID_PAINT_DEV_KERN_TEXT_FLAG		= 0x00000100,
		ANDROID_PAINT_DITHER_FLAG			= 0x00000004,
		ANDROID_PAINT_EMBEDDED_BITMAP_TEXT_FLAG	= 0x00000400,
		ANDROID_PAINT_FAKE_BOLD_TEXT_FLAG		= 0x00000020,
		ANDROID_PAINT_FILTER_BITMAP_FLAG		= 0x00000002,
		ANDROID_PAINT_LINEAR_TEXT_FLAG		= 0x00000040,
		ANDROID_PAINT_STRIKE_THRU_TEXT_FLAG		= 0x00000010,
		ANDROID_PAINT_SUBPIXEL_TEXT_FLAG		= 0x00000080,
		ANDROID_PAINT_UNDERLINE_TEXT_FLAG		= 0x00000008;

	//! Sets flags on the Paint being used to display the text and reflows the text if they are different from the old flags.
	void setPaintFlags(int flags);

	//! Return the flags on the Paint being used to display the text.
	int getPaintFlags();

	//! Set the TextView so that when it takes focus, all the text is selected.
	void setSelectAllOnFocus(bool selectAllOnFocus);

	//! If true, sets the properties of this field (number of lines, horizontally scrolling, transformation method) to be for a single-line input; if false, restores these to the default conditions.
	void setSingleLine(bool singleLine = true);

	//! Sets the text color for all the states (normal, selected, focused) to be this color.
	void setTextColor(int color);

	//! NB: to use QGlobalColor, do like this: setTextColor(QColor(Qt::red)).
	void setTextColor(const QColor & color);

	//! Sets the extent by which text should be stretched horizontally.
	void setTextScaleX(float size);

	//! Sets whether the content of this view is selectable by the user.
	void setTextIsSelectable(bool selectable);

	// http://developer.android.com/reference/android/widget/TextView.html#attr_android:gravity
	static const int
		ANDROID_GRAVITY_TOP					= 0x00000030,
		ANDROID_GRAVITY_BOTTOM				= 0x00000050,
		ANDROID_GRAVITY_LEFT				= 0x00000003,
		ANDROID_GRAVITY_RIGHT				= 0x00000005,
		ANDROID_GRAVITY_CENTER				= 0x00000011,
		ANDROID_GRAVITY_CENTER_HORIZONTAL	= 0x00000001,
		ANDROID_GRAVITY_CENTER_VERTICAL		= 0x00000010,
		ANDROID_GRAVITY_FILL				= 0x00000077,
		ANDROID_GRAVITY_FILL_HORIZONTAL		= 0x00000007,
		ANDROID_GRAVITY_FILL_VERTICAL		= 0x00000070,
		ANDROID_GRAVITY_CLIP_HORIZONTAL		= 0x00000008,
		ANDROID_GRAVITY_CLIP_VERTICAL		= 0x00000080,
		ANDROID_GRAVITY_START				= 0x00800003,
		ANDROID_GRAVITY_END					= 0x00800005;

	// http://developer.android.com/reference/android/view/inputmethod/EditorInfo.html
	static const int
		ANDROID_IME_NULL						= 0x00000000,
		ANDROID_IME_ACTION_UNSPECIFIED			= 0x00000000,
		ANDROID_IME_ACTION_NONE					= 0x00000001,
		ANDROID_IME_ACTION_GO					= 0x00000002,
		ANDROID_IME_ACTION_SEARCH				= 0x00000003,
		ANDROID_IME_ACTION_SEND					= 0x00000004,
		ANDROID_IME_ACTION_NEXT					= 0x00000005,
		ANDROID_IME_ACTION_DONE					= 0x00000006,
		ANDROID_IME_ACTION_PREVIOUS 			= 0x00000007,
		ANDROID_IME_MASK_ACTION					= 0x000000ff,

		ANDROID_IME_FLAG_NO_FULLSCREEN			= 0x02000000,
		ANDROID_IME_FLAG_NAVIGATE_PREVIOUS		= 0x04000000,
		ANDROID_IME_FLAG_NAVIGATE_NEXT			= 0x08000000,
		ANDROID_IME_FLAG_NO_EXTRACT_UI			= 0x10000000,

		ANDROID_IME_FLAG_NO_ENTER_ACTION		= 0x40000000,
		ANDROID_IME_FLAG_NO_ACCESSORY_ACTION	= 0x20000000,
		ANDROID_IME_FLAG_FORCE_ASCII			= -2147483648; // 0x80000000

	//! Sets the horizontal alignment of the text and the vertical gravity that will be used when there is extra space in the TextView beyond what is required for the text itself.
	void setGravity(int gravity);

	//! Makes the TextView exactly this many pixels tall.
	void setHeight(int pixels);

	//! Sets the color used to display the selection highlight.
	void setHighlightColor(int color);

	//! NB: to use QGlobalColor, do like this: setTextColor(QColor(Qt::red)).
	void setHighlightColor(const QColor & color);

	//! Sets the text to be displayed when the text of the TextView is empty.
	void setHint(const QString & hint);

	//! Sets the color of the hint text for all the states (disabled, focussed, selected...) of this TextView.
	void setHintTextColor(int color);

	//! NB: to use QGlobalColor, do like this: setTextColor(QColor(Qt::red)).
	void setHintTextColor(const QColor & color);

	//! Makes the TextView exactly this many pixels wide.
	void setWidth(int pixels);

	//! Sets line spacing for this TextView.
	void setLineSpacing(float add, float mult);

	//! Makes the TextView exactly this many lines tall.
	void setLines(int lines);

	//! Sets whether the text should be allowed to be wider than the View is.
	void setHorizontallyScrolling(bool whether);

	//! Sets the properties of this field to transform input to ALL CAPS display.
	void setAllCaps(bool allCaps);

	//! Set password mode.
	void setPasswordMode();

	// Set/unset password mode with default typeface. This is not normally possible in Android.
	// A text change watcher is installed to workaround the problem. To remove the watcher
	// (e.g. to change the purpose of the EditText) please call to
	// setPasswordModeWithDefaultTypeface(false) first.
	void setPasswordModeWithDefaultTypeface(bool enable);

	// Change the editor type integer associated with the text view, which will be reported to an IME with imeOptions when it has focus.
	// options = (options & and) | or
	void setImeOptions(int and_mask, int or_mask);

	void setImeAction(int action) { setImeOptions(~ANDROID_IME_MASK_ACTION, action); }

	void setImeActionUnspecified() { setImeAction(ANDROID_IME_ACTION_UNSPECIFIED); }
	void setImeActionGo() { setImeAction(ANDROID_IME_ACTION_GO); }
	void setImeActionSearch() { setImeAction(ANDROID_IME_ACTION_SEARCH); }
	void setImeActionSend() { setImeAction(ANDROID_IME_ACTION_SEND); }
	void setImeActionNext() { setImeAction(ANDROID_IME_ACTION_NEXT); }
	void setImeActionDone() { setImeAction(ANDROID_IME_ACTION_DONE); }
	void setImeActionPrevious() { setImeAction(ANDROID_IME_ACTION_PREVIOUS); }

	// http://developer.android.com/reference/android/text/TextUtils.TruncateAt.html
	enum AndroidTruncateAt
	{
		ANDROID_TRUNCATE_AT_START	= 0,
		ANDROID_TRUNCATE_AT_MIDDLE	= 1,
		ANDROID_TRUNCATE_AT_END		= 2,
		ANDROID_TRUNCATE_AT_MARQUEE	= 3
	};

	// Causes words in the text that are longer than the view is wide to be ellipsized instead of broken in the middle.
	void setEllipsize(AndroidTruncateAt ellipsis);

	//
	// EditText methods
	//
	void selectAll();
	void setSelection(int index);
	void setSelection(int start, int stop);
	int getSelectionStart();
	int getSelectionEnd();
	void setHorizontalScrollBarEnabled(bool horizontalScrollBarEnabled);
	void setVerticalScrollBarEnabled (bool verticalScrollBarEnabled);

	//
	// Own helper methods
	//
	void setAllowFullscreenKeyboard(bool allow);
	void setCursorColorToTextColor();
	void setMaxLength(int length);

	//
	// Drawing mode hack
	//
	static const int
		SYSTEM_DRAW_NEVER		= 0,
		SYSTEM_DRAW_ALWAYS		= 1,
		SYSTEM_DRAW_HACKY		= 2;
	int getSystemDrawMode();
	void setSystemDrawMode(int mode);

signals:
	//! See Android: TextWatcher.onTextChanged().
	void onTextChanged(QString text, int start, int before, int count);

	//! Simple notification that the text has been changed.
	void onTextChanged();

	//! Emitted when KEYCODE_DPAD_CENTER or KEYCODE_ENTER has been released.
	void onEnter();

public:
	static const int
		ANDROID_EDITORINFO_IME_ACTION_DONE			= 0x00000006,
		ANDROID_EDITORINFO_IME_ACTION_GO			= 0x00000002,
		ANDROID_EDITORINFO_IME_ACTION_NEXT			= 0x00000005,
		ANDROID_EDITORINFO_IME_ACTION_NONE			= 0x00000001,
		ANDROID_EDITORINFO_IME_ACTION_PREVIOUS		= 0x00000007,
		ANDROID_EDITORINFO_IME_ACTION_SEARCH		= 0x00000003,
		ANDROID_EDITORINFO_IME_ACTION_SEND			= 0x00000004,
		ANDROID_EDITORINFO_IME_ACTION_UNSPECIFIED	= 0x00000000;

signals:
	/*!
	 * Emitted when Back key is pressed. Note: QAndroidOffscreenEditText suppresses
	 * the default processing of the Back key (which ends current activity).
	 * A wrapper can inject Qt::Key_Escape press into Qt queue or simply process
	 * the signal by itself.
	 */
	void onKeyBack(bool down);

	//! Use ANDROID_EDITORINFO_IME_... for actions.
	void onEditorAction(int action);

	//! Emitted when Enter pressed or editor action required.
	void onEnterOrPositiveAction();



/*	void onKeyMultiple(int keyCode, int repeatCount, KeyEvent event)	//	Default implementation of KeyEvent.Callback.onKeyMultiple(): always returns false (doesn't handle the event).
	void onKeyPreIme(int keyCode, KeyEvent event)	//	Handle a key event before it is processed by any input method associated with the view hierarchy.
	//	Called on the focused view when a key shortcut event is not handled.
	void onKeyShortcut(int keyCode, KeyEvent event); */


	//
	// Functions to implement:
	//

	//	void 	addTextChangedListener(TextWatcher watcher)
	//	Adds a TextWatcher to the list of those whose methods are called whenever this TextView's text changes.
		//	abstract void 	afterTextChanged(Editable s)
		//	This method is called to notify you that, somewhere within s, the text has been changed.
		//	abstract void 	beforeTextChanged(CharSequence s, int start, int count, int after)
		//	This method is called to notify you that, within s, the count characters beginning at start are about to be replaced by new text with length after.
		//	abstract void 	onTextChanged(CharSequence s, int start, int before, int count)

	//	final void 	append(CharSequence text) // Convenience method: Append the specified text to the TextView's display buffer, upgrading it to BufferType.EDITABLE if it was not already editable.
	//	void 	append(CharSequence text, int start, int end) // Convenience method: Append the specified text slice to the TextView's display buffer, upgrading it to BufferType.EDITABLE if it was not already editable.

	//  void beginBatchEdit()
	//	boolean 	bringPointIntoView(int offset)	// Move the point, specified by the offset, into the view if it is needed.
	//	void 	cancelLongPress() //	Cancels a pending long press.
	//	void 	clearComposingText() //	Use BaseInputConnection.removeComposingSpans() to remove any IME composing state from this text view.
	//	void 	computeScroll() //	Called by a parent to request that a child update its values for mScrollX and mScrollY if necessary.
	//	void 	debug(int depth) //	Prints information about this view in the log output, with the tag VIEW_LOG_TAG.
	//	boolean 	didTouchFocusSelect() //	Returns true, only while processing a touch gesture, if the initial touch down event caused focus to move to the text view and as a result its selection changed.
	//	void 	endBatchEdit()
	//	boolean 	extractText(ExtractedTextRequest request, ExtractedText outText)
	//	If this TextView contains editable content, extract a portion of it based on the information in request in to outText.
	//	void 	findViewsWithText(ArrayList<View> outViews, CharSequence searched, int flags)
	//	Finds the Views that contain given text.
	//	final int 	getAutoLinkMask()
	//	Gets the autolink mask of the text.
	//	int 	getBaseline() // Return the offset of the widget's text baseline from the widget's top boundary.
	//	int 	getCompoundDrawablePadding()	//	Returns the padding between the compound drawables and the text.
	//	final int 	getCurrentHintTextColor()	//	Return the current color selected to paint the hint text.
	//	final int 	getCurrentTextColor()	//	Return the current color selected for normal text.
	//	ActionMode.Callback 	getCustomSelectionActionModeCallback()	//	Retrieves the value set in setCustomSelectionActionModeCallback(ActionMode.Callback).
	//	Editable 	getEditableText()	//	Return the text the TextView is displaying as an Editable object.
	//	TextUtils.TruncateAt 	getEllipsize()	//	Returns where, if anywhere, words that are longer than the view is wide should be ellipsized.
	//	CharSequence 	getError()	//	Returns the error message that was set to be displayed with setError(CharSequence), or null if no error was set or if it the error was cleared by the widget after user input.
	//	int 	getExtendedPaddingBottom()	//	Returns the extended bottom padding of the view, including both the bottom Drawable if any and any extra space to keep more than maxLines of text from showing.
	//	int 	getExtendedPaddingTop()	//	Returns the extended top padding of the view, including both the top Drawable if any and any extra space to keep more than maxLines of text from showing.
	//	InputFilter[] 	getFilters()	//	Returns the current list of input filters.
	//	void 	getFocusedRect(Rect r)	//	When a view has focus and the user navigates away from it, the next view is searched for starting from the rectangle filled in by this method.
	//	boolean 	getFreezesText()	//	Return whether this text view is including its entire text contents in frozen icicles.
	//	int 	getGravity()	//	Returns the horizontal and vertical alignment of this TextView.
	//	int 	getHighlightColor()	//	CharSequence
	//	getHint()	//	Returns the hint that is displayed when the text of the TextView is empty.
	//	final ColorStateList 	getHintTextColors()
	//	int 	getImeActionId()	//	Get the IME action ID previous set with setImeActionLabel(CharSequence, int).
	//	CharSequence 	getImeActionLabel()	//	Get the IME action label previous set with setImeActionLabel(CharSequence, int).
	//	int 	getImeOptions()	//	Get the type of the IME editor.
	//	boolean 	getIncludeFontPadding()	//	Gets whether the TextView includes extra top and bottom padding to make room for accents that go above the normal ascent and descent.
	//	Bundle 	getInputExtras(boolean create)	//	Retrieve the input extras currently associated with the text view, which can be viewed as well as modified.
	//	int 	getInputType()	//	Get the type of the editable content.
	//	final KeyListener 	getKeyListener()
	//	final Layout 	getLayout()
	//	int 	getLineBounds(int line, Rect bounds)	//	Return the baseline for the specified line (0...getLineCount() - 1) If bounds is not null, return the top, left, right, bottom extents of the specified line in it.
	//	int 	getLineCount()	//	Return the number of lines of text, or 0 if the internal Layout has not been built.
	//	int 	getLineHeight()
	//	float 	getLineSpacingExtra()	//	Gets the line spacing extra space
	//	float 	getLineSpacingMultiplier()
	//	Gets the line spacing multiplier
	//	final ColorStateList 	getLinkTextColors()
	//	final boolean 	getLinksClickable()	//	Returns whether the movement method will automatically be set to LinkMovementMethod if setAutoLinkMask(int) has been set to nonzero and links are detected in setText(char[], int, int).
	//	int 	getMarqueeRepeatLimit()	//	Gets the number of times the marquee animation is repeated.
	//	int 	getMaxEms()
	//	int 	getMaxHeight()
	//	int 	getMaxLines()
	//	int 	getMaxWidth()
	//	int 	getMinEms()
	//	int 	getMinHeight()
	//	int 	getMinLines()
	//	int 	getMinWidth()
	//	final MovementMethod 	getMovementMethod()
	//	int 	getOffsetForPosition(float x, float y)	//	Get the character offset closest to the specified absolute position.
	//	TextPaint 	getPaint()
	//	String 	getPrivateImeOptions()	//	Get the private type of the content.
	//	int 	getShadowColor()
	//	float 	getShadowDx()
	//	float 	getShadowDy()
	//	float 	getShadowRadius()	//	Gets the radius of the shadow layer.
	//	static int 	getTextColor(Context context, TypedArray attrs, int def)	//	Returns the default color from the TextView_textColor attribute from the AttributeSet, if set, or the default color from the TextAppearance_textColor from the TextView_textAppearance attribute, if TextView_textColor was not set directly.
	//	final ColorStateList 	getTextColors()	//	Gets the text colors for the different states (normal, selected, focused) of the TextView.
	//	static ColorStateList 	getTextColors(Context context, TypedArray attrs)	//	Returns the TextView_textColor attribute from the TypedArray, if set, or the TextAppearance_textColor from the TextView_textAppearance attribute, if TextView_textColor was not set directly.
	//	Locale 	getTextLocale()	//	Get the default Locale of the text in this TextView.
	//	float 	getTextScaleX()
	//	float 	getTextSize()
	//	int 	getTotalPaddingBottom()	//	Returns the total bottom padding of the view, including the bottom Drawable if any, the extra space to keep more than maxLines from showing, and the vertical offset for gravity, if any.
	//	int 	getTotalPaddingEnd()	//	Returns the total end padding of the view, including the end Drawable if any.
	//	int 	getTotalPaddingLeft()	//	Returns the total left padding of the view, including the left Drawable if any.
	//	int 	getTotalPaddingRight()	//	Returns the total right padding of the view, including the right Drawable if any.
	//	int 	getTotalPaddingStart()	//	Returns the total start padding of the view, including the start Drawable if any.
	//	int 	getTotalPaddingTop()	//	Returns the total top padding of the view, including the top Drawable if any, the extra space to keep more than maxLines from showing, and the vertical offset for gravity, if any.
	//	final TransformationMethod 	getTransformationMethod()
	//	Typeface 	getTypeface()
	//	URLSpan[] 	getUrls()	//	Returns the list of URLSpans attached to the text (by Linkify or otherwise) if any.
	//	boolean 	hasOverlappingRendering()	//	Returns whether this View has content which overlaps.
	//	boolean 	hasSelection()	//	Return true iff there is a selection inside this text view.
	//	void 	invalidateDrawable(Drawable drawable)	//	Invalidates the specified Drawable.
	//	boolean 	isCursorVisible()
	//	boolean 	isInputMethodTarget()	//	Returns whether this text view is a current input method target.
	//	boolean 	isSuggestionsEnabled()	//	Return whether or not suggestions are enabled on this TextView.
	//	boolean 	isTextSelectable()	//	Returns the state of the textIsSelectable flag (See setTextIsSelectable()).
	//	void 	jumpDrawablesToCurrentState()	//	Call Drawable.jumpToCurrentState() on all Drawable objects associated with this view.
	//	int 	length()	//	Returns the length, in characters, of the text managed by this TextView
	//	boolean 	moveCursorToVisibleOffset()	//	Move the cursor, if needed, so that it is at an offset that is visible to the user.
	//	void 	onBeginBatchEdit()	//	Called by the framework in response to a request to begin a batch of edit operations through a call to link beginBatchEdit().
	//	boolean 	onCheckIsTextEditor()	//	Check whether the called view is a text editor, in which case it would make sense to automatically display a soft input window for it.
	//	void 	onCommitCompletion(CompletionInfo text)	//	Called by the framework in response to a text completion from the current input method, provided by it calling InputConnection.commitCompletion().
	//	void 	onCommitCorrection(CorrectionInfo info)	//	Called by the framework in response to a text auto-correction (such as fixing a typo using a a dictionnary) from the current input method, provided by it calling commitCorrection(CorrectionInfo) InputConnection.commitCorrection()}.
	//	InputConnection 	onCreateInputConnection(EditorInfo outAttrs)	//	Create a new InputConnection for an InputMethod to interact with the view.
	//	boolean 	onDragEvent(DragEvent event)	//	Handles drag events sent by the system following a call to startDrag().
	//	void 	onEditorAction(int actionCode)	//	Called when an attached input method calls InputConnection.performEditorAction() for this text view.
	//	void 	onEndBatchEdit()	//	Called by the framework in response to a request to end a batch of edit operations through a call to link endBatchEdit().
	//	void 	onFinishTemporaryDetach()	//	Called after onStartTemporaryDetach() when the container is done changing the view.
	//	boolean 	onGenericMotionEvent(MotionEvent event)	//	Implement this method to handle generic motion events.
	//	void 	onInitializeAccessibilityEvent(AccessibilityEvent event)	//	Initializes an AccessibilityEvent with information about this View which is the event source.
	//	void 	onInitializeAccessibilityNodeInfo(AccessibilityNodeInfo info)	//	Initializes an AccessibilityNodeInfo with information about this view.
	//	void 	onPopulateAccessibilityEvent(AccessibilityEvent event)	//	Called from dispatchPopulateAccessibilityEvent(AccessibilityEvent) giving a chance to this View to populate the accessibility event with its text content.
	//	boolean 	onPreDraw()	//	Callback method to be invoked when the view tree is about to be drawn.
	//	boolean 	onPrivateIMECommand(String action, Bundle data)	//	Called by the framework in response to a private command from the current method, provided by it calling InputConnection.performPrivateCommand().
	//	void 	onRestoreInstanceState(Parcelable state)	//	Hook allowing a view to re-apply a representation of its internal state that had previously been generated by onSaveInstanceState().
	//	void 	onRtlPropertiesChanged(int layoutDirection)	//	Called when any RTL property (layout direction or text direction or text alignment) has been changed.
	//	Parcelable 	onSaveInstanceState()	//	Hook allowing a view to generate a representation of its internal state that can later be used to create a new instance with that same state.
	//	void 	onScreenStateChanged(int screenState)	//	This method is called whenever the state of the screen this view is attached to changes.
	//	void 	onStartTemporaryDetach()	//	This is called when a container is going to temporarily detach a child, with ViewGroup.detachViewFromParent.
	//	boolean 	onTextContextMenuItem(int id)	//	Called when a context menu option for the text view is selected.
	//	boolean 	onTouchEvent(MotionEvent event)	//	Implement this method to handle touch screen motion events.
	//	boolean 	onTrackballEvent(MotionEvent event)	//	Implement this method to handle trackball motion events.
	//	void 	onWindowFocusChanged(boolean hasWindowFocus)	//	Called when the window containing this view gains or loses focus.
	//	boolean 	performAccessibilityAction(int action, Bundle arguments)	//	Performs the specified accessibility action on the view.
	//	boolean 	performLongClick()	//	Call this view's OnLongClickListener, if it is defined.
	//	void 	removeTextChangedListener(TextWatcher watcher)	//	Removes the specified TextWatcher from the list of those whose methods are called whenever this TextView's text changes.
	//	void 	sendAccessibilityEvent(int eventType)	//	Sends an accessibility event of the given type.
	//	final void 	setAutoLinkMask(int mask)	//	Sets the autolink mask of the text.
	//	void 	setCustomSelectionActionModeCallback(ActionMode.Callback actionModeCallback)	//	If provided, this ActionMode.Callback will be used to create the ActionMode when text selection is initiated in this View.
	//	final void 	setEditableFactory(Editable.Factory factory)	//	Sets the Factory used to create new Editables.
	//	void 	setEms(int ems)	//	Makes the TextView exactly this many ems wide
	//	void 	setEnabled(boolean enabled)	//	Set the enabled state of this view.
	//	void 	setError(CharSequence error)	//	Sets the right-hand compound drawable of the TextView to the "error" icon and sets an error message that will be displayed in a popup when the TextView has focus.
	//	void 	setError(CharSequence error, Drawable icon)	//	Sets the right-hand compound drawable of the TextView to the specified icon and sets an error message that will be displayed in a popup when the TextView has focus.
	//	void 	setExtractedText(ExtractedText text)	//	Apply to this text view the given extracted text, as previously returned by extractText(ExtractedTextRequest, ExtractedText).
	//	void 	setFilters(InputFilter[] filters)	//	Sets the list of input filters that will be used if the buffer is Editable.
	//	void 	setFreezesText(boolean freezesText)	//	Control whether this text view saves its entire text contents when freezing to an icicle, in addition to dynamic state such as cursor position.
	//	void 	setImeActionLabel(CharSequence label, int actionId)	//	Change the custom IME action associated with the text view, which will be reported to an IME with actionLabel and actionId when it has focus.
	//	void 	setIncludeFontPadding(boolean includepad)	//	Set whether the TextView includes extra top and bottom padding to make room for accents that go above the normal ascent and descent.
	//	void 	setInputExtras(int xmlResId)	//	Set the extra input data of the text, which is the TextBoxAttribute.extras Bundle that will be filled in when creating an input connection.
	//	void 	setKeyListener(KeyListener input)	//	Sets the key listener to be used with this TextView.
	//	final void 	setLinkTextColor(ColorStateList colors)	//	Sets the color of links in the text.
	//	final void 	setLinkTextColor(int color)	//	Sets the color of links in the text.
	//	final void 	setLinksClickable(boolean whether)	//	Sets whether the movement method will automatically be set to LinkMovementMethod if setAutoLinkMask(int) has been set to nonzero and links are detected in setText(char[], int, int).
	//	final void 	setMovementMethod(MovementMethod movement)	//	Sets the movement method (arrow key handler) to be used for this TextView.
	//	void 	setOnEditorActionListener(TextView.OnEditorActionListener l)	//	Set a special listener to be called when an action is performed on the text view.
	//	void 	setPrivateImeOptions(String type)	//	Set the private content type of the text, which is the EditorInfo.privateImeOptions field that will be filled in when creating an input connection.
	//	void 	setRawInputType(int type)	//	Directly change the content type integer of the text view, without modifying any other state.
	//	void 	setScroller(Scroller s)
	//	void 	setSelected(boolean selected)	//	Changes the selection state of this view.
	//	void 	setShadowLayer(float radius, float dx, float dy, int color)	//	Gives the text a shadow of the specified radius and color, the specified distance from its normal position.
	//	final void 	setSpannableFactory(Spannable.Factory factory)	//	Sets the Factory used to create new Spannables.
	//	void 	setText(CharSequence text, TextView.BufferType type)	//	Sets the text that this TextView is to display (see setText(CharSequence)) and also sets whether it is stored in a styleable/spannable buffer and whether it is editable.
	//	void 	setTextAppearance(Context context, int resid)	//	Sets the text color, size, style, hint color, and highlight color from the specified TextAppearance resource.
	//	final void 	setTextKeepState(CharSequence text)	//	Like setText(CharSequence), except that the cursor position (if any) is retained in the new text.
	//	final void 	setTextKeepState(CharSequence text, TextView.BufferType type)	//	Like setText(CharSequence, android.widget.TextView.BufferType), except that the cursor position (if any) is retained in the new text.
	//	final void 	setTransformationMethod(TransformationMethod method)	//	Sets the transformation that is applied to the text that this TextView is displaying.
	// void setTextLocale(Locale locale)	//	Set the default Locale of the text in this TextView to the given value.

protected:
	virtual void javaOnTextChanged(const QString & str, int start, int before, int count);
	virtual bool javaOnKey(bool down, int androidKey);
	virtual void javaOnEditorAction(int action);

private:
	friend void JNICALL Java_AndroidOffscreenEditText_nativeOnTextChanged(JNIEnv * env, jobject jo, jlong param, jstring str, jint start, jint before, jint count);
	friend jboolean JNICALL Java_AndroidOffscreenEditText_nativeOnKey(JNIEnv * env, jobject jo, jlong param, jboolean down, jint keycode);
	friend void JNICALL Java_AndroidOffscreenEditText_nativeOnEditorAction(JNIEnv *, jobject, jlong param, jint action);

private:
	int paint_flags_;
};

