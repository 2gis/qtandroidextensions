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

package ru.dublgis.offscreenview;

import java.lang.reflect.Field;
import android.app.Activity;
import android.content.Context;
import android.graphics.Rect;
import android.graphics.Typeface;
import android.os.Handler;
import android.text.Editable;
import android.text.InputType;
import android.text.Layout.Alignment;
import android.text.StaticLayout;
import android.text.TextWatcher;
import android.text.method.PasswordTransformationMethod;
import android.text.method.TransformationMethod;
import android.view.inputmethod.EditorInfo;
import android.view.ViewGroup;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.widget.EditText;
import android.widget.TextView;
import android.graphics.Canvas;
import android.graphics.PorterDuff;
import android.graphics.Color;
import android.text.TextUtils;
import android.text.InputFilter;
import android.text.Layout;
import android.content.ClipboardManager;
import android.content.ClipData;
import java.lang.CharSequence;
import android.text.Spanned;

import ru.dublgis.androidhelpers.Log;
import ru.dublgis.offscreenview.inputmask.InputMaskFormatter;


class OffscreenEditText extends OffscreenView
{
    protected String text_ = "";
    boolean single_line_ = false;
    boolean need_to_reflow_text_ = false, need_to_reflow_hint_ = false;
    int selection_top_ = 0, selection_bottom_ = 0;
    private Object variables_mutex_ = new Object();

    static final int
        SYSTEM_DRAW_NEVER = 0,
        SYSTEM_DRAW_ALWAYS = 1,
        SYSTEM_DRAW_HACKY = 2;
    private int system_draw_ = SYSTEM_DRAW_HACKY;

    class MyEditText extends EditText
    {
        private boolean vertically_scrolling_ = false;
        private int content_height_ = 0;
        private boolean allow_rich_text_ = true;
        private MyTextWatcher textWatcher_ = null;

        class MyTextWatcher implements TextWatcher
        {
            private InputMaskFormatter formatter_;
            private String formattedText_;
            private boolean hasAcceptableInput_ = true;

            public void setMask(final String mask)
            {
                if (mask == null || mask.isEmpty()) {
                    formatter_ = null;
                    formattedText_ = null;

                    setHasAcceptableInput(true);
                } else {
                    formatter_ = new InputMaskFormatter(mask);
                    formattedText_ = formatter_.format(getText().toString());

                    setText(formattedText_);
                    setSelection(formattedText_.length());
                    setHasAcceptableInput(formatter_.hasAcceptableInput());
                }
            }

            @Override
            public void beforeTextChanged(CharSequence s, int start, int count, int after)
            {}

            @Override
            public void onTextChanged(CharSequence s, int start, int before, int count)
            {
                try {
                    final String text = s.toString();

                    if (formatter_ != null && !text.equals(formattedText_)) {
                        formattedText_ = formatter_.format(text);

                        setText(formattedText_);
                        setSelection(formattedText_.length());
                        setHasAcceptableInput(formatter_.hasAcceptableInput());
                        return;
                    }

                    need_to_reflow_text_ = false;
                    need_to_reflow_hint_ = false;

                    synchronized(text_) {
                        text_ = text;
                    }
                    synchronized(nativePtrMutex()) {
                        nativeOnTextChanged(getNativePtr(), text, start, before, count);
                    }

                    updateContentHeight();
                } catch (final Throwable e) {
                    Log.e(TAG, "Exception in onTextChanged:", e);
                }
            }

            @Override
            public void afterTextChanged(Editable s)
            {}

            private void setHasAcceptableInput(boolean hasAcceptableInput)
            {
                try {
                    if (hasAcceptableInput_ != hasAcceptableInput) {
                        hasAcceptableInput_ = hasAcceptableInput;
                        synchronized(nativePtrMutex()) {
                            nativeOnHasAcceptableInputChanged(getNativePtr(), hasAcceptableInput_);
                        }
                    }
                } catch (Exception e) {
                    Log.e(TAG, "Exception in setHasAcceptableInput:", e);
                }
            }
        }

        public MyEditText(Context context)
        {
            super(context);
            try {
                Log.i(TAG, "MyEditText constructor");
                textWatcher_ = new MyTextWatcher();
                addTextChangedListener(textWatcher_);

                // Eliminate any system background by setting null background drawable.
                // Any borders should be drawn on Qt side, because we simply cannot deal with
                // all these custom vendors backrounds.
                if (getApiLevel() >= 16)
                {
                    Log.i(TAG, "MyEditText constructor: using API >= 16 method (setBackground)");
                    setBackground(null);
                }
                else
                {
                    Log.i(TAG, "MyEditText constructor: using API < 16 method (setBackgroundDrawable)");
                    setBackgroundDrawable(null);
                }
            } catch (final Throwable e) {
                Log.e(TAG, "Exception in MyEditText ctor:", e);
            }
        }

        @Override
        public boolean onTextContextMenuItem(int id)
        {
            try {
                if (id == android.R.id.paste && !allow_rich_text_)
                {
                    if (getApiLevel() >= 23) // Android 6.0
                    {
                        id = android.R.id.pasteAsPlainText;
                    }
                    else
                    {
                        onInterceptClipDataToPlainText();
                    }
                                onInterceptClipDataToPlainText();
                }
            } catch (final Throwable e) {
                Log.e(TAG, "Exception in onTextContextMenuItem:", e);
            }
            return super.onTextContextMenuItem(id);
        }

        private void onInterceptClipDataToPlainText()
        {
            try
            {
                ClipboardManager clipboard = (ClipboardManager) getContext()
                    .getSystemService(Context.CLIPBOARD_SERVICE);
                ClipData clip = clipboard.getPrimaryClip();
                if (clip != null)
                {
                    for (int i = 0; i < clip.getItemCount(); i++)
                    {
                        final CharSequence paste;
                        // Get an item as text and remove all spans by toString().
                        final CharSequence text = clip.getItemAt(i).coerceToText(getContext());
                        paste = (text instanceof Spanned) ? text.toString() : text;
                        if (paste != null)
                        {
                            ClipData clipData = ClipData.newPlainText("rebase_copy", paste);
                            clipboard.setPrimaryClip(clipData);
                        }
                    }
                }
            }
            catch (final Throwable e)
            {
                Log.e(TAG, "Exception in onInterceptClipDataToPlainText:", e);
            }
        }

        void setMask(final String mask)
        {
            textWatcher_.setMask(mask);
        }

        // Super evil workaround to set cursor color equal to text color.
        // This is equal to setting android:textCursorDrawable="@null".
        void setCursorColorToTextColor()
        {
            setCursorByDrawableId(0);
        }

        private void setCursorByDrawableId(int id)
        {
            try
            {
                // Access to non-SDK interfaces denied on Android >= 10
                if (getApiLevel() >= 29)
                {
                    setTextCursorDrawable(id);
                }
                else
                {
                    // https://github.com/android/platform_frameworks_base/blob/kitkat-release/core/java/android/widget/TextView.java#L562-564
                    Field f = TextView.class.getDeclaredField("mCursorDrawableRes");
                    f.setAccessible(true);
                    f.set((TextView)this, id);
                }
            }
            catch (final Throwable e)
            {
                Log.e(TAG, "Exception in setCursorByDrawableId:", e);
            }
        }

        void setCursorByDrawableName(String name)
        {
            try
            {
                Context context = getContext();
                int id = context.getResources().getIdentifier(name, "drawable", context.getPackageName());
                setCursorByDrawableId(id);
            }
            catch (final Throwable e)
            {
                Log.e(TAG, "Exception in setCursorByDrawableName:", e);
            }
        }


        @Override
        protected void onDraw(Canvas canvas)
        {
            try
            {
                if (system_draw_ != SYSTEM_DRAW_NEVER && isInAttachingMode())
                {
                    if (system_draw_ == SYSTEM_DRAW_HACKY)
                    {
                        // We should perform this draw for system text zoom but ignore it
                        // for any other purposes.
                        //
                        // OK (zoom, should be painted):
                        // onDraw: my size is 992x72, canvas: 195x122, hwa: false, mainlayout: 1200x1662
                        //
                        // BAD (screen paint, should be ignored):
                        // onDraw: my size is 992x72, canvas: 1200x1824, hwa: false, mainlayout: 1200x1774
                        MyEditText edittext = (MyEditText)getView();
                        ViewGroup mainlayout = getMainLayout();
                        boolean ignore_this_draw =
                            // If the canvas is wider and taller than our EditText then it's probably not
                            // a zoom window (which is usually small) so it should be ignored.
                            // This is the worst part of the workaround :(
                            (canvas.getWidth() >= edittext.getWidth() && canvas.getHeight() >= edittext.getHeight())
                            // If output canvas is wider than my outer layout it must be a full window
                            // paint. We should ignore it as the window painting is handled by Qt.
                            ||  (mainlayout == null || mainlayout.getWidth() <= canvas.getWidth());
                        /*Log.d(TAG, "onDraw: my size is " +
                            edittext.getWidth() + "x", edittext.getHeight() +
                            ", canvas: " + canvas.getWidth() + "x" + canvas.getHeight() +
                            ", hwa: " + canvas.isHardwareAccelerated() +
                            ", mainlayout: " + ((mainlayout==null)?"null": "" + mainlayout.getWidth() + "x" + mainlayout.getHeight()) +
                            ", ignoring: " + ignore_this_draw);*/
                        if (ignore_this_draw)
                        {
                            return;
                        }
                    }
                    super.onDraw(canvas);
                }
            }
            catch (final Throwable e)
            {
                Log.e(TAG, "Exception in onDraw:", e);
            }
        }

        public void onDrawPublic(Canvas canvas)
        {
            // A text view has transparent background by default, which is not what we expect.
            try {
                synchronized(view_variables_mutex_) {
                    canvas.drawColor(Color.argb(fill_a_, fill_r_, fill_g_, fill_b_), PorterDuff.Mode.SRC);
                }
                super.onDraw(canvas);
            } catch (final Throwable e) {
                Log.e(TAG, "Exception in onDrawPublic:", e);
            }
        }

        @Override
        public void invalidate(Rect dirty)
        {
            try {
                super.invalidate(dirty);
                invalidateOffscreenView();
            } catch (final Throwable e) {
                Log.e(TAG, "Exception in invalidate:", e);
            }
        }

        @Override
        public void invalidate(int l, int t, int r, int b)
        {
            try {
                super.invalidate(l, t, r, b);
                int my_r = getScrollX() + getWidth();
                int my_b = getScrollY() + getHeight();
                // Check that the invalidated rectangle actually visible
                if (l > my_r || t > my_b)
                {
                    return;
                }
                invalidateOffscreenView();
            } catch (final Throwable e) {
                Log.e(TAG, "Exception in invalidate:", e);
            }
        }

        @Override
        public void invalidate()
        {
            try {
                super.invalidate();
                invalidateOffscreenView();
            } catch (final Throwable e) {
                Log.e(TAG, "Exception in invalidate:", e);
            }
        }

        @Override
        public void requestLayout()
        {
            try {
                super.requestLayout();
                invalidateOffscreenView();
            } catch (final Throwable e) {
                Log.e(TAG, "Exception in requestLayout:", e);
            }
        }

        private int text_layout_width_ = 0;

        @Override
        protected void onLayout(boolean changed, int left, int top, int right, int bottom)
        {
            try {
                super.onLayout(changed, left, top, right, bottom);

                // Here's an evil workaround. TextView does not update text flow on relayout.
                // We should avoid the workaround triggering when text edit bar appears because
                // it causes input method to restart.
                int w = right - left;
                if (changed && w != text_layout_width_)
                {
                    text_layout_width_ = w;
                    reflowWorkaround();
                }

                updateSelectionInfo();
                updateContentHeight();
            } catch (final Throwable e) {
                Log.e(TAG, "Exception in onLayout:", e);
            }
        }

        private void updateSelectionInfo() {
            try {
                Layout layout = getLayout();
                if (layout == null) {
                    return;
                }

                int start = Math.max(getSelectionStart(), 0);
                int end = Math.max(getSelectionEnd(), 0);
                int top = layout.getLineTop(layout.getLineForOffset(start));
                int bottom = layout.getLineBottom(layout.getLineForOffset(end));
                if (selection_top_ == top && selection_bottom_ == bottom) {
                    return;
                }
                selection_top_ = top;
                selection_bottom_ = bottom;
                synchronized(nativePtrMutex()) {
                    nativeSetSelectionInfo(getNativePtr(), top, bottom);
                }
            } catch (final Throwable e) {
                Log.e(TAG, "Exception in updateSelectionInfo:", e);
            }
        }

        protected void reflowWorkaround()
        {
            try {
                if (!single_line_)
                {
                    need_to_reflow_text_ = true;
                }
                need_to_reflow_hint_ = true;

                // Can't reflow text or hint right now because it causes text selection
                // markers to become "invincible" if they are visible. So let's post
                // it for later.
                (new Handler()).post(new Runnable(){
                    @Override
                    public void run(){
                        // No need to reflow hint if it's not shown
                        try {
                            if (getText().length() > 0)
                            {
                                need_to_reflow_hint_ = false;
                            }

                            if (need_to_reflow_text_ || need_to_reflow_hint_)
                            {
                                // Text selection markers may obtain invicibility if we call setText()
                                // or setHint(), so let's hide selection markers to be sure they are not there.
                                int cursor_pos = getSelectionEnd();
                                if (need_to_reflow_text_)
                                {
                                    setText(getText());
                                    need_to_reflow_text_ = false;
                                }
                                if (need_to_reflow_hint_)
                                {
                                    setHint(getHint());
                                    need_to_reflow_hint_ = false;
                                }
                                setSelection(cursor_pos);
                            }
                        } catch (final Throwable e) {
                            Log.e(TAG, "Exception in reflowWorkaround.run:", e);
                        }
                    }
                });
            } catch (final Throwable e) {
                Log.e(TAG, "Exception in reflowWorkaround:", e);
            }
        }

        @Override
        public boolean onKeyDown(int keyCode, KeyEvent event)
        {
            synchronized(nativePtrMutex()) {
                if (!nativeOnKey(getNativePtr(), true, keyCode))
                {
                    return super.onKeyDown(keyCode, event);
                }
                else
                {
                    return true;
                }
            }
        }

        @Override
        public boolean onKeyUp(int keyCode, KeyEvent event)
        {
            synchronized(nativePtrMutex()) {
                if (!nativeOnKey(getNativePtr(), false, keyCode))
                {
                    return super.onKeyUp(keyCode, event);
                }
                else
                {
                    return true;
                }
            }
        }

        @Override
        public void onEditorAction(int actionCode)
        {
            try {
                synchronized(nativePtrMutex()) {
                    nativeOnEditorAction(getNativePtr(), actionCode);
                }
            } catch (final Throwable e) {
                Log.e(TAG, "Exception in onEditorAction:", e);
            }
        }

        @Override
        public boolean onTouchEvent(MotionEvent event)
        {
            try {
                if (isOffscreenTouch())
                {
                    return super.onTouchEvent(event);
                }
            } catch (final Throwable e) {
                Log.e(TAG, "onTouchEvent exception: ", e);
            }
            return false;
        }

        @Override
        public void onSelectionChanged(int selStart, int selEnd)
        {
            try {
                super.onSelectionChanged(selStart, selEnd);
                synchronized(nativePtrMutex()) {
                    nativeTextSelectionChanged(getNativePtr(), selStart, selEnd);
                }
            } catch (final Throwable e) {
                Log.e(TAG, "Exception in onSelectionChanged:", e);
            }
        }

        @Override
        public void scrollTo(int x, int y) {
            try {
                super.scrollTo(x, vertically_scrolling_ ? y : 0);
            } catch (final Throwable e) {
                Log.e(TAG, "Exception in scrollTo:", e);
            }
        }

        public void setVerticallyScrolling(final boolean whether)
        {
            vertically_scrolling_ = whether;
        }

        private int getMinimalTextHeight()
        {
            try {
                String text = getText().toString();
                if (text.isEmpty()) {
                    text = "W";
                }

                int textHeight = (new StaticLayout(
                    text,
                    getPaint(),
                    getWidth() - getTotalPaddingRight() - getTotalPaddingLeft(),
                    Alignment.ALIGN_NORMAL,
                    1.0f,
                    0.0f,
                    true)).getHeight();

                if (getHint() == null || getHint().toString().isEmpty()) {
                    return textHeight;
                }

                int hintHeight = (new StaticLayout(
                    getHint(),
                    getPaint(),
                    getWidth() - getTotalPaddingRight() - getTotalPaddingLeft(),
                    Alignment.ALIGN_NORMAL,
                    1.0f,
                    0.0f,
                    true)).getHeight();

                return textHeight > hintHeight ? textHeight : hintHeight;

            } catch (final Throwable e) {
                Log.e(TAG, "getMinimalTextHeight exception:", e);
                return 0;
            }
        }

        private void updateContentHeight()
        {
            try {
                int contentHeight;
                if (single_line_)
                {
                    contentHeight = getHeight();
                }
                else
                {
                    int widthMeasureSpec = MeasureSpec.makeMeasureSpec(getWidth(), MeasureSpec.EXACTLY);
                    int heightMeasureSpec = MeasureSpec.makeMeasureSpec(getMinimalTextHeight(), MeasureSpec.UNSPECIFIED);
                    measure(widthMeasureSpec, heightMeasureSpec);
                    contentHeight =  getMeasuredHeight();
                }
                if (content_height_ != contentHeight)
                {
                    content_height_ = contentHeight;
                    runViewAction(new Runnable() {
                        @Override
                        public void run() {
                            try {
                                synchronized(nativePtrMutex()) {
                                    nativeOnContentHeightChanged(getNativePtr(), content_height_);
                                }
                            } catch (final Throwable e) {
                                Log.e(TAG, "Exception in updateContentHeight.run:", e);
                            }
                        }
                    });
                }
            } catch (final Throwable e) {
                Log.e(TAG, "updateContentHeight exception:", e);
            }
        }

        public void setRichTextMode(final boolean enabled)
        {
            allow_rich_text_ = enabled;
        }
    }

    OffscreenEditText()
    {
        // Log.i(TAG, "OffscreenEditText constructor");
    }

    @Override
    public void doCreateView()
    {
        try {
            setView(new MyEditText(getActivity()));
        } catch (final Throwable e) {
            Log.e(TAG, "Exception in doCreateView:", e);
        }
    }

    @Override
    public void callViewPaintMethod(Canvas canvas)
    {
        try {
            ((MyEditText)getView()).onDrawPublic(canvas);
        } catch (final Throwable e) {
            Log.e(TAG, "Exception in callViewPaintMethod:", e);
        }
    }




    public native void nativeOnTextChanged(long nativePtr, String s, int start, int before, int count);
    public native boolean nativeOnKey(long nativePtr, boolean down, int keyCode);
    public native void nativeOnEditorAction(long nativePtr, int action);
    public native void nativeSetSelectionInfo(long nativePtr, int top, int bottom);
    public native void nativeTextSelectionChanged(long nativePtr, int start, int stop);
    public native void nativeOnContentHeightChanged(long nativePtr, int height);
    public native void nativeOnHasAcceptableInputChanged(long nativePtr, boolean hasAcceptableInput);





    public void setText(final String text)
    {
        synchronized(text_)
        {
            text_ = text;
        }
        runViewAction(new Runnable(){
            @Override
            public void run(){
                // May crash input method manager in rare situations
                try
                {
                    ((MyEditText)getView()).setText(text);
                    if (text.isEmpty())
                    {
                        invalidateOffscreenView(true);
                    }
                    need_to_reflow_text_ = false;
                }
                catch (final Throwable e)
                {
                    Log.e(TAG, "Exception in setText:", e);
                }
            }
        });
    }

    String getText()
    {
        synchronized(text_)
        {
            return text_;
        }
    }

    void setTextSize(final float size, final int unit)
    {
        runViewAction(new Runnable(){
            @Override
            public void run(){
                //Note that the param order is different in C++ and View
                try {
                    ((MyEditText)getView()).setTextSize(unit, size);
                } catch (final Throwable e) {
                    Log.e(TAG, "Exception in setTextSize.run:", e);
                }
            }
        });
    }

    void setTypeface(final String name, final int style)
    {
        runViewAction(new Runnable(){
            @Override
            public void run(){
                try
                {
                    ((MyEditText)getView()).setTypeface(Typeface.create((name.length() > 0)? name: null, style));
                }
                catch (final Throwable e)
                {
                    Log.e(TAG, "Failed to create Typeface with name " + name + ":", e);
                }
            }
        });
    }

    void setTypefaceFromFile(final String filename, final int style)
    {
        runViewAction(new Runnable(){
            @Override
            public void run(){
                try
                {
                    Typeface face = Typeface.createFromFile(filename);
                    ((MyEditText)getView()).setTypeface((style==0)? face: Typeface.create(face, style));
                }
                catch (final Throwable e)
                {
                    Log.e(TAG, "Failed to create Typeface from file:", e);
                }
            }
        });
    }

    void setTypefaceFromAsset(final String filename, final int style)
    {
        runViewAction(new Runnable(){
            @Override
            public void run(){
                Activity a = getActivity();
                if (a != null)
                {
                    try
                    {
                        Typeface face = Typeface.createFromAsset(a.getAssets(), filename);
                        ((MyEditText)getView()).setTypeface((style==0)? face: Typeface.create(face, style));
                    }
                    catch (final Throwable e)
                    {
                        Log.e(TAG, "Failed to create Typeface from asset:", e);
                    }
                }
            }
        });
    }

    void setCursorVisible(final boolean visible)
    {
        runViewAction(new Runnable(){
            @Override
            public void run(){
                try {
                    ((MyEditText)getView()).setCursorVisible(visible);
                } catch (final Throwable e) {
                    Log.e(TAG, "Exception in setCursorVisible.run:", e);
                }
            }
        });
    }

    void setAutofillType(int type)
    {
        // Android 8+
        if (getApiLevel() >= 26)
        {
            runViewAction(new Runnable(){
                @Override
                public void run(){
                    try {
                        ((MyEditText)getView()).setImportantForAutofill(type);
                    } catch (final Throwable e) {
                        Log.e(TAG, "Exception in setAutofillType.run:", e);
                    }
                }
            });
        }
    }

    void setHintType(String type)
    {
        // Android 8+
        if (getApiLevel() >= 26)
        {
            runViewAction(new Runnable(){
                @Override
                public void run(){
                    try {
                        ((MyEditText)getView()).setAutofillHints(type);
                    } catch (final Throwable e) {
                        Log.e(TAG, "Exception in setHintType.run:", e);
                    }
                }
            });
        }
    }

    void setInputType(final int type)
    {
        runViewAction(new Runnable(){
            @Override
            public void run(){
                try {
                    ((MyEditText)getView()).setInputType(type);
                } catch (final Throwable e) {
                    Log.e(TAG, "Exception in setInputType.run:", e);
                }
            }
        });
    }

    void setInputType(final int type_and, final int type_or)
    {
        runViewAction(new Runnable(){
            @Override
            public void run(){
                try {
                    MyEditText edittext = (MyEditText)getView();
                    int type = edittext.getInputType();
                    edittext.setInputType((type & type_and) | type_or);
                } catch (final Throwable e) {
                    Log.e(TAG, "Exception in setInputType.run:", e);
                }
            }
        });
    }

    void setMarqueeRepeatLimit(final int marqueeLimit)
    {
        runViewAction(new Runnable(){
            @Override
            public void run(){
                try {
                    ((MyEditText)getView()).setMarqueeRepeatLimit(marqueeLimit);
                } catch (final Throwable e) {
                    Log.e(TAG, "Exception in setMarqueeRepeatLimit.run:", e);
                }
            }
        });
    }

    void setMaxEms(final int maxems)
    {
        runViewAction(new Runnable(){
            @Override
            public void run(){
                try {
                    ((MyEditText)getView()).setMaxEms(maxems);
                } catch (final Throwable e) {
                    Log.e(TAG, "Exception in setMaxEms.run:", e);
                }
            }
        });
    }

    void setMinEms(final int minems)
    {
        runViewAction(new Runnable(){
            @Override
            public void run(){
                try {
                    ((MyEditText)getView()).setMinEms(minems);
                } catch (final Throwable e) {
                    Log.e(TAG, "Exception in setMinEms.run:", e);
                }
            }
        });
    }

    void setMaxHeight(final int maxHeight)
    {
        runViewAction(new Runnable(){
            @Override
            public void run(){
                try {
                    ((MyEditText)getView()).setMaxHeight(maxHeight);
                } catch (final Throwable e) {
                    Log.e(TAG, "Exception in setMaxHeight.run:", e);
                }
            }
        });
    }

   void setMinHeight(final int minHeight)
    {
        runViewAction(new Runnable(){
            @Override
            public void run(){
                try {
                    ((MyEditText)getView()).setMinHeight(minHeight);
                } catch (final Throwable e) {
                    Log.e(TAG, "Exception in setMinHeight.run:", e);
                }
            }
        });
    }

    void setMaxLines(final int maxlines)
    {
        runViewAction(new Runnable(){
            @Override
            public void run(){
                try {
                    single_line_ = (maxlines == 1);
                    ((MyEditText)getView()).setMaxLines(maxlines);
                } catch (final Throwable e) {
                    Log.e(TAG, "Exception in setMaxLines.run:", e);
                }
            }
        });
    }

    void setMinLines(final int minlines)
    {
        runViewAction(new Runnable(){
            @Override
            public void run(){
                try {
                    ((MyEditText)getView()).setMinLines(minlines);
                } catch (final Throwable e) {
                    Log.e(TAG, "Exception in setMinLines.run:", e);
                }
            }
        });
    }

    void setMaxWidth(final int maxpixels)
    {
        runViewAction(new Runnable(){
            @Override
            public void run(){
                try {
                    ((MyEditText)getView()).setMaxWidth(maxpixels);
                } catch (final Throwable e) {
                    Log.e(TAG, "Exception in setMaxWidth.run:", e);
                }
            }
        });
    }

    void setMinWidth(final int minpixels)
    {
        runViewAction(new Runnable(){
            @Override
            public void run(){
                try {
                    ((MyEditText)getView()).setMinWidth(minpixels);
                } catch (final Throwable e) {
                    Log.e(TAG, "Exception in setMinWidth.run:", e);
                }
            }
        });
    }

    void setPadding(final int left, final int top, final int right, final int bottom)
    {
        runViewAction(new Runnable(){
            @Override
            public void run(){
                try {
                    ((MyEditText)getView()).setPadding(left, top, right, bottom);
                } catch (final Throwable e) {
                    Log.e(TAG, "Exception in setPadding.run:", e);
                }
            }
        });
    }

    void setIncludeFontPadding(final boolean enabled)
    {
        runViewAction(new Runnable(){
            @Override
            public void run(){
                try {
                    ((MyEditText)getView()).setIncludeFontPadding(enabled);
                } catch (final Throwable e) {
                    Log.e(TAG, "Exception in setIncludeFontPadding.run:", e);
                }
            }
        });
    }

    void setPaintFlags(final int flags)
    {
        runViewAction(new Runnable(){
            @Override
            public void run(){
                try {
                    ((MyEditText)getView()).setPaintFlags(flags);
                } catch (final Throwable e) {
                    Log.e(TAG, "Exception in setMinHeight.run:", e);
                }
            }
        });
    }

   void setSelectAllOnFocus(final boolean selectAllOnFocus)
    {
        runViewAction(new Runnable(){
            @Override
            public void run(){
                try {
                    ((MyEditText)getView()).setSelectAllOnFocus(selectAllOnFocus);
                } catch (final Throwable e) {
                    Log.e(TAG, "Exception in setSelectAllOnFocus.run:", e);
                }
            }
        });
    }

    void setSingleLine(final boolean singleLine)
    {
        runViewAction(new Runnable(){
            @Override
            public void run(){
                try {
                    single_line_ = singleLine;
                    ((MyEditText)getView()).setSingleLine(singleLine);
                    ((MyEditText)getView()).reflowWorkaround();
                } catch (final Throwable e) {
                    Log.e(TAG, "Exception in setSingleLine.run:", e);
                }
            }
        });
    }

    void setTextColor(final int color)
    {
        runViewAction(new Runnable(){
            @Override
            public void run(){
                try {
                    ((MyEditText)getView()).setTextColor(color);
                    invalidateOffscreenView();
                } catch (Exception e) {
                    Log.e(TAG, "Exception in setTextColor.run:", e);
                }
            }
        });
    }

    void setTextScaleX(final float size)
    {
        runViewAction(new Runnable(){
            @Override
            public void run(){
                try {
                    ((MyEditText)getView()).setTextScaleX(size);
                } catch (Exception e) {
                    Log.e(TAG, "Exception in setTextScaleX.run:", e);
                }
            }
        });
    }

    void setTextIsSelectable(final boolean selectable)
    {
        runViewAction(new Runnable(){
            @Override
            public void run(){
                try {
                    ((MyEditText)getView()).setTextIsSelectable(selectable);
                } catch (Exception e) {
                    Log.e(TAG, "Exception in setSingleLine.run:", e);
                }
            }
        });
    }

    void setGravity(final int gravity)
    {
        runViewAction(new Runnable(){
            @Override
            public void run(){
                try {
                    ((MyEditText)getView()).setGravity(gravity);
                } catch (Exception e) {
                    Log.e(TAG, "Exception in setGravity.run:", e);
                }
            }
        });
    }

    void setHeight(final int pixels)
    {
        runViewAction(new Runnable(){
            @Override
            public void run(){
                try {
                    ((MyEditText)getView()).setHeight(pixels);
                } catch (Exception e) {
                    Log.e(TAG, "Exception in setHeight.run:", e);
                }
            }
        });
    }

    void setHighlightColor(final int color)
    {
        runViewAction(new Runnable(){
            @Override
            public void run(){
                try {
                    ((MyEditText)getView()).setHighlightColor(color);
                } catch (Exception e) {
                    Log.e(TAG, "Exception in setHighlightColor.run:", e);
                }
            }
        });
    }

    void setHint(final String hint)
    {
        runViewAction(new Runnable(){
            @Override
            public void run(){
                try {
                    ((MyEditText)getView()).setHint(hint);
                    ((MyEditText)getView()).reflowWorkaround();

                    // This should not be done here:
                    //     need_to_reflow_hint_ = false;
                    // (Occasionally causes hint to be unreflown during startup.)
                } catch (Exception e) {
                    Log.e(TAG, "Exception in setHint.run:", e);
                }
            }
        });
    }

    void setHintTextColor(final int color)
    {
        runViewAction(new Runnable(){
            @Override
            public void run(){
                try {
                    ((MyEditText)getView()).setHintTextColor(color);
                } catch (Exception e) {
                    Log.e(TAG, "Exception in setHintTextColor.run:", e);
                }
            }
        });
    }

    void setWidth(final int pixels)
    {
        runViewAction(new Runnable(){
            @Override
            public void run(){
                try {
                    ((MyEditText)getView()).setWidth(pixels);
                } catch (Exception e) {
                    Log.e(TAG, "Exception in setWidth.run:", e);
                }
            }
        });
    }

    void setLineSpacing(final float add, final float mult)
    {
        runViewAction(new Runnable(){
            @Override
            public void run(){
                try {
                    ((MyEditText)getView()).setLineSpacing(add, mult);
                } catch (Exception e) {
                    Log.e(TAG, "Exception in setLineSpacing.run:", e);
                }
            }
        });
    }

    void setLines(final int lines)
    {
        runViewAction(new Runnable(){
            @Override
            public void run(){
                try {
                    ((MyEditText)getView()).setLines(lines);
                } catch (Exception e) {
                    Log.e(TAG, "Exception in setLines.run:", e);
                }
            }
        });
    }

    void setHorizontallyScrolling(final boolean whether)
    {
        runViewAction(new Runnable(){
            @Override
            public void run(){
                try {
                    ((MyEditText)getView()).setHorizontallyScrolling(whether);
                } catch (Exception e) {
                    Log.e(TAG, "Exception in setHorizontallyScrolling.run:", e);
                }
            }
        });
    }

    void setVerticallyScrolling(final boolean whether)
    {
        runViewAction(new Runnable(){
            @Override
            public void run(){
                try {
                    ((MyEditText)getView()).setVerticallyScrolling(whether);
                } catch (Exception e) {
                    Log.e(TAG, "Exception in setVerticallyScrolling.run:", e);
                }
            }
        });
    }

    void setAllCaps(final boolean allCaps)
    {
        runViewAction(new Runnable(){
            @Override
            public void run(){
                try {
                    ((MyEditText)getView()).setAllCaps(allCaps);
                } catch (Exception e) {
                    Log.e(TAG, "Exception in setAllCaps.run:", e);
                }
            }
        });
    }

    void selectAll()
    {
        runViewAction(new Runnable(){
            @Override
            public void run(){
                try {
                    ((MyEditText)getView()).selectAll();
                } catch (Exception e) {
                    Log.e(TAG, "Exception in selectAll.run:", e);
                }
            }
        });
    }

    void setSelection(final int index)
    {
        runViewAction(new Runnable(){
            @Override
            public void run(){
                try {
                    ((MyEditText)getView()).setSelection(index);
                } catch (Exception e) {
                    Log.e(TAG, "Exception in setSelection.run:", e);
                }
            }
        });
    }

    void setSelection(final int start, final int stop)
    {
        runViewAction(new Runnable() {
            @Override
            public void run() {
                try {
                    ((MyEditText)getView()).setSelection(start, stop);
                } catch (final Throwable e) {
                    Log.e(TAG, "Failed to set bounds to (" + start + ", " + stop + "):", e);
                }
            }
        });
    }

    void setVerticalScrollBarEnabled(final boolean verticalScrollBarEnabled)
    {
        runViewAction(new Runnable(){
            @Override
            public void run(){
                try {
                    ((MyEditText)getView()).setVerticalScrollBarEnabled(verticalScrollBarEnabled);
                } catch (Exception e) {
                    Log.e(TAG, "Exception in setVerticalScrollBarEnabled.run:", e);
                }
            }
        });
    }

    void setHorizontalScrollBarEnabled(final boolean horizontalScrollBarEnabled)
    {
        runViewAction(new Runnable(){
            @Override
            public void run(){
                try {
                    ((MyEditText)getView()).setHorizontalScrollBarEnabled(horizontalScrollBarEnabled);
                } catch (Exception e) {
                    Log.e(TAG, "Exception in setHorizontalScrollBarEnabled.run:", e);
                }
            }
        });
    }

    void setAllowFullscreenKeyboard(final boolean allow)
    {
        runViewAction(new Runnable(){
            @Override
            public void run(){
                try {
                    MyEditText met = (MyEditText)getView();
                    int ops = met.getImeOptions();
                    int flag = (getApiLevel() >= 11)?
                        EditorInfo.IME_FLAG_NO_FULLSCREEN | EditorInfo.IME_FLAG_NO_EXTRACT_UI
                        : EditorInfo.IME_FLAG_NO_EXTRACT_UI;
                    if (allow)
                    {
                        ops &= ~flag;
                    }
                    else
                    {
                        ops |= flag;
                    }
                    met.setImeOptions(ops);
                } catch (Exception e) {
                    Log.e(TAG, "Exception in setAllowFullscreenKeyboard.run:", e);
                }
            }
        });
    }

    void setMaxLength(final int length)
    {
        runViewAction(new Runnable(){
            @Override
            public void run(){
                try {
                    if (length > 0)
                    {
                        ((MyEditText)getView()).setFilters(new InputFilter[]{new InputFilter.LengthFilter(length)});
                    }
                    else
                    {
                        ((MyEditText)getView()).setFilters(new InputFilter[]{});
                    }
                } catch (Exception e) {
                    Log.e(TAG, "Exception in setMaxLength.run:", e);
                }
            }
        });
    }

    void setPasswordMode(final boolean enable)
    {
        runViewAction(new Runnable(){
            @Override
            public void run(){
                try {
                    final MyEditText myedittext = (MyEditText)getView();
                    final int selStart = myedittext.getSelectionStart();
                    final int selEnd = myedittext.getSelectionEnd();
                    final TransformationMethod methodInstance = enable
                        ? PasswordTransformationMethod.getInstance() 
                        : null;
                        
                    myedittext.setTransformationMethod(methodInstance);
                    myedittext.setSelection(selStart, selEnd);
                } catch (Exception e) {
                    Log.e(TAG, "Exception in setPasswordMode.run:", e);
                }
            }
        });
    }

    void setRichTextMode(final boolean enabled)
    {
        runViewAction(new Runnable(){
            @Override
            public void run(){
                try {
                    final MyEditText myedittext = (MyEditText)getView();
                    myedittext.setRichTextMode(enabled);
                } catch (Exception e) {
                    Log.e(TAG, "Exception in setRichTextMode.run:", e);
                }
            }
        });
    }


    private volatile TextWatcher password_text_watcher_ = null;
    private volatile int input_type_before_password_with_custom_typeface_ = 0;

    protected void setPasswordModeWithCustomTypeface(final boolean enable, final Typeface typeface)
    {
        runViewAction(new Runnable() {
            @Override
            public void run() {
                try {
                    final MyEditText myedittext = (MyEditText)getView();
                    // Disabling the mode
                    if (!enable) {
                        if (password_text_watcher_ != null) {
                            myedittext.removeTextChangedListener(password_text_watcher_);
                            password_text_watcher_ = null;
                            myedittext.setInputType(input_type_before_password_with_custom_typeface_);
                        } else {
                            Log.e(TAG, "Attempting to clear password mode with default typeface when it was not set!");
                        }
                    // Enabling the mode
                    } else {
                        if (password_text_watcher_ == null) {
                            // Need to set only transformation method after component creation, not input type
                            // because input type will change placeholder typeface to monospace, and we have
                            // no way to change it back to default typeface before we start typing text
                            // If we set TYPE_TEXT_VARIATION_PASSWORD right after component created it will change
                            // placeholder text typeface to monospace immediately, and we can't call setTypeface
                            // it will just have no effect, because typeface is changing to monospace not immediately:
                            // http://stackoverflow.com/questions/24117178/android-typeface-is-changed-when-i-apply-password-type-on-edittext
                            password_text_watcher_ = new TextWatcher()
                                {
                                    @Override
                                    public void beforeTextChanged(CharSequence s, int start, int count, int after) {
                                        if ((myedittext.getInputType() & InputType.TYPE_TEXT_VARIATION_PASSWORD) == 0) {
                                            input_type_before_password_with_custom_typeface_ = myedittext.getInputType();
                                            myedittext.setInputType(InputType.TYPE_CLASS_TEXT | InputType.TYPE_TEXT_VARIATION_PASSWORD);
                                        }
                                        myedittext.setTypeface(typeface);
                                    }

                                    @Override
                                    public void onTextChanged(CharSequence s, int start, int before, int count) {
                                        myedittext.setTypeface(typeface);
                                    }

                                    @Override
                                    public void afterTextChanged(Editable s) {
                                        myedittext.setTypeface(typeface);
                                    }
                                };
                            myedittext.setTransformationMethod(PasswordTransformationMethod.getInstance());
                            myedittext.addTextChangedListener(password_text_watcher_);
                        } else {
                            Log.e(TAG, "Attempting to set password mode with default typeface twice!");
                        }
                    } // Enabling mode
                } catch (Exception e) {
                    Log.e(TAG, "Exception in setPasswordModeWithCustomTypeface.run:", e);
                }
            } // run()
        });
    }

    void setPasswordModeWithDefaultTypeface(final boolean enable)
    {
        setPasswordModeWithCustomTypeface(enable, Typeface.DEFAULT);
    }

    void setImeOptions(final int and_mask, final int or_mask)
    {
        runViewAction(new Runnable(){
            @Override
            public void run(){
                try {
                    MyEditText et = (MyEditText)getView();
                    et.setImeOptions((et.getImeOptions() & and_mask) | or_mask);
                } catch (Exception e) {
                    Log.e(TAG, "Exception in setImeOptions.run:", e);
                }
            }
        });
    }

    void setEllipsize(final int ellipsis)
    {
        final TextUtils.TruncateAt where = TextUtils.TruncateAt.values()[ellipsis];

        runViewAction(new Runnable(){
            @Override
            public void run(){
                try {
                    ((MyEditText)getView()).setEllipsize(where);
                } catch (Exception e) {
                    Log.e(TAG, "Exception in setEllipsize.run:", e);
                }
            }
        });
    }

    void setCursorColorToTextColor()
    {
        runViewAction(new Runnable(){
            @Override
            public void run(){
                try {
                    ((MyEditText)getView()).setCursorColorToTextColor();
                } catch (Exception e) {
                    Log.e(TAG, "Exception in setCursorColorToTextColor.run:", e);
                }
            }
        });
    }

    void setCursorByDrawableName(String name)
    {
        runViewAction(new Runnable(){
            @Override
            public void run(){
                try {
                    ((MyEditText)getView()).setCursorByDrawableName(name);
                } catch (Exception e) {
                    Log.e(TAG, "Exception in setCursorByDrawableName.run:", e);
                }
            }
        });
    }

    int getSystemDrawMode()
    {
        return system_draw_;
    }

    void setSystemDrawMode(int mode)
    {
        system_draw_ = mode;
    }

    void setInputMask(String mask) 
    {
        runViewAction(new Runnable() {
            @Override
            public void run() {
                try 
                {
                    final MyEditText editText = ((MyEditText) getView());
                    editText.setMask(mask);
                } 
                catch (final Throwable e) 
                {
                    Log.w(TAG, "setInputMask: '" + mask + "' exeption:", e);
                }
            }
        });
    }
}
