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

import java.lang.Thread;
import java.util.Set;
import java.util.List;
import java.util.LinkedList;
import java.util.Map;
import java.util.HashMap;
import java.util.Arrays;
import java.util.TreeSet;
import java.util.Locale;
import java.util.List;
import java.util.LinkedList;
import java.io.File;
import android.app.Activity;
import android.app.ActivityManager;
import android.app.AlertDialog;
import android.content.res.Configuration;
import android.content.Context;
import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.content.pm.ConfigurationInfo;
import android.content.DialogInterface;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Color;
import android.graphics.drawable.Drawable;
import android.graphics.Paint;
import android.graphics.Rect;
import android.graphics.Typeface;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.SystemClock;
import android.text.Editable;
import android.text.method.MetaKeyKeyListener;
import android.text.InputType;
import android.text.TextWatcher;
import android.util.Log;
import android.util.DisplayMetrics;
import android.view.ContextMenu;
import android.view.ContextMenu.ContextMenuInfo;
import android.view.inputmethod.EditorInfo;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewParent;
import android.view.KeyEvent;
import android.view.KeyCharacterMap;
import android.view.Menu;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.Window;
import android.view.WindowManager;
import android.view.inputmethod.InputMethodManager;
import android.widget.EditText;
import android.graphics.Canvas;

class OffscreenEditText extends OffscreenView
{
    protected String text_ = "";
    boolean single_line_ = false;

    class MyEditText extends EditText
    {

        class MyTextWatcher implements TextWatcher
        {
            @Override
            public void afterTextChanged(Editable s)
            {}

            @Override
            public void beforeTextChanged(CharSequence s, int start, int count, int after)
            {}

            @Override
            public void onTextChanged(CharSequence s, int start, int before, int count)
            {
                String str = s.toString();
                synchronized(text_)
                {
                    text_ = str;
                }
                nativeOnTextChanged(getNativePtr(), str, start, before, count);
            }
        }

        public MyEditText(Context context)
        {
            super(context);
            Log.i(TAG, "MyEditText constructor");
            addTextChangedListener(new MyTextWatcher());

            // Eliminate any system background by setting null background drawable.
            // Any borders should be drawn on Qt side, because we simply cannot deal with
            // all these custom vendors backrounds.
            if (getApiLevel() >= 16)
            {
                setBackground(null);
            }
            else
            {
                setBackgroundDrawable(null);
            }
        }

        @Override
        protected void onDraw(Canvas canvas)
        {
            // Don't draw when called from layout
        }

        public void onDrawPublic(Canvas canvas)
        {
            // A text view has transparent background by default, which is not what we expect.
            canvas.drawARGB (fill_a_, fill_r_, fill_g_, fill_b_);
            super.onDraw(canvas);
        }

        @Override
        public void invalidate(Rect dirty)
        {
            super.invalidate(dirty);
            invalidateOffscreenView();
        }

        @Override
        public void invalidate(int l, int t, int r, int b)
        {
            // Log.i(TAG, "MyEditText.invalidate(int l, int t, int r, int b) "+l+", "+t+", "+r+", "+b+
            //    "; width="+width_+", height="+height_+"; scrollX="+getScrollX()+", scrollY="+getScrollY());
            super.invalidate(l, t, r, b);
            int my_r = getScrollX() + getWidth();
            int my_b = getScrollY() + getHeight();
            // Check that the invalidated rectangle actually visible
            if (l > my_r || t > my_b)
            {
                // Log.i(TAG, "MyEditText.invalidate: ignoring invisible rectangle");
                return;
            }
            invalidateOffscreenView();
        }

        @Override
        public void invalidate()
        {
            super.invalidate();
            invalidateOffscreenView();
        }

        @Override
        public void requestLayout()
        {
            super.requestLayout();
            invalidateOffscreenView();
        }

        private int text_layout_width_ = 0;

        @Override
        protected void onLayout(boolean changed, int left, int top, int right, int bottom)
        {
            super.onLayout(changed, left, top, right, bottom);
            if (!single_line_)
            {
                // Here's an evil workaround. TextView does not update text flow on relayout.
                // We should avoid the workaround triggering when text edit bar appears because
                // it causes input method to restart.
                int w = right - left;
                if (changed && w != text_layout_width_)
                {
                    setText(getText());
                }
                text_layout_width_ = w;
            }
        }

        @Override
        public boolean onKeyDown(int keyCode, KeyEvent event)
        {
            if (!nativeOnKey(getNativePtr(), true, keyCode))
            {
                return super.onKeyDown(keyCode, event);
            }
            else
            {
                return true;
            }
        }

        @Override
        public boolean onKeyUp(int keyCode, KeyEvent event)
        {
            if (!nativeOnKey(getNativePtr(), false, keyCode))
            {
                return super.onKeyUp(keyCode, event);
            }
            else
            {
                return true;
            }
        }

        @Override
        public boolean onTouchEvent(MotionEvent event)
        {
            if (isOffscreenTouch())
            {
                return super.onTouchEvent(event);
            }
            return false;
        }
    }

    OffscreenEditText()
    {
        // Log.i(TAG, "OffscreenEditText constructor");
    }

    @Override
    public void doCreateView()
    {
        setView(new MyEditText(getActivity()));
    }

    @Override
    public void callViewPaintMethod(Canvas canvas)
    {
        ((MyEditText)getView()).onDrawPublic(canvas);
    }




    public native void nativeOnTextChanged(long nativePtr, String s, int start, int before, int count);
    public native boolean nativeOnKey(long nativePtr, boolean down, int keyCode);





    public void setText(final String text)
    {
        runViewAction(new Runnable(){
            @Override
            public void run(){
                ((MyEditText)getView()).setText(text);
                synchronized(text_)
                {
                    text_ = text;
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
                ((MyEditText)getView()).setTextSize(unit, size);
            }
        });
    }

    void setTypeface(final String name, final int style)
    {
        runViewAction(new Runnable(){
            @Override
            public void run(){
                ((MyEditText)getView()).setTypeface(Typeface.create((name.length() > 0)? name: null, style));
            }
        });
    }

    void setTypefaceFromFile(final String filename, final int style)
    {
        runViewAction(new Runnable(){
            @Override
            public void run(){
                Typeface face = Typeface.createFromFile(filename);
                ((MyEditText)getView()).setTypeface((style==0)? face: Typeface.create(face, style));
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
                    Typeface face = Typeface.createFromAsset(a.getAssets(), filename);
                    ((MyEditText)getView()).setTypeface((style==0)? face: Typeface.create(face, style));
                }
            }
        });
    }

    void setCursorVisible(final boolean visible)
    {
        runViewAction(new Runnable(){
            @Override
            public void run(){
                ((MyEditText)getView()).setCursorVisible(visible);
            }
        });
    }

    void setInputType(final int type)
    {
        runViewAction(new Runnable(){
            @Override
            public void run(){
                ((MyEditText)getView()).setInputType(type);
            }
        });
    }

    void setMarqueeRepeatLimit(final int marqueeLimit)
    {
        runViewAction(new Runnable(){
            @Override
            public void run(){
                ((MyEditText)getView()).setMarqueeRepeatLimit(marqueeLimit);
            }
        });
    }

    void setMaxEms(final int maxems)
    {
        runViewAction(new Runnable(){
            @Override
            public void run(){
                ((MyEditText)getView()).setMaxEms(maxems);
            }
        });
    }

    void setMinEms(final int minems)
    {
        runViewAction(new Runnable(){
            @Override
            public void run(){
                ((MyEditText)getView()).setMinEms(minems);
            }
        });
    }

    void setMaxHeight(final int maxHeight)
    {
        runViewAction(new Runnable(){
            @Override
            public void run(){
                ((MyEditText)getView()).setMaxHeight(maxHeight);
            }
        });
    }

   void setMinHeight(final int minHeight)
    {
        runViewAction(new Runnable(){
            @Override
            public void run(){
                ((MyEditText)getView()).setMinHeight(minHeight);
            }
        });
    }

    void setMaxLines(final int maxlines)
    {
        runViewAction(new Runnable(){
            @Override
            public void run(){
                single_line_ = (maxlines == 1);
                ((MyEditText)getView()).setMaxLines(maxlines);
            }
        });
    }

    void setMinLines(final int minlines)
    {
        runViewAction(new Runnable(){
            @Override
            public void run(){
                ((MyEditText)getView()).setMinLines(minlines);
            }
        });
    }

    void setMaxWidth(final int maxpixels)
    {
        runViewAction(new Runnable(){
            @Override
            public void run(){
                ((MyEditText)getView()).setMaxWidth(maxpixels);
            }
        });
    }

    void setMinWidth(final int minpixels)
    {
        runViewAction(new Runnable(){
            @Override
            public void run(){
                ((MyEditText)getView()).setMinWidth(minpixels);
            }
        });
    }

    void setPadding(final int left, final int top, final int right, final int bottom)
    {
        runViewAction(new Runnable(){
            @Override
            public void run(){
                ((MyEditText)getView()).setPadding(left, top, right, bottom);
            }
        });
    }

    void setPaintFlags(final int flags)
    {
        runViewAction(new Runnable(){
            @Override
            public void run(){
                ((MyEditText)getView()).setPaintFlags(flags);
            }
        });
    }

   void setSelectAllOnFocus(final boolean selectAllOnFocus)
    {
        runViewAction(new Runnable(){
            @Override
            public void run(){
                ((MyEditText)getView()).setSelectAllOnFocus(selectAllOnFocus);
            }
        });
    }

    void setSingleLine(final boolean singleLine)
    {
        runViewAction(new Runnable(){
            @Override
            public void run(){
                single_line_ = singleLine;
                ((MyEditText)getView()).setSingleLine(singleLine);
            }
        });
    }

    void setTextColor(final int color)
    {
        runViewAction(new Runnable(){
            @Override
            public void run(){
                ((MyEditText)getView()).setTextColor(color);
            }
        });
    }

    void setTextScaleX(final float size)
    {
        runViewAction(new Runnable(){
            @Override
            public void run(){
                ((MyEditText)getView()).setTextScaleX(size);
            }
        });
    }

    void setTextIsSelectable(final boolean selectable)
    {
        runViewAction(new Runnable(){
            @Override
            public void run(){
                ((MyEditText)getView()).setTextIsSelectable(selectable);
            }
        });
    }

    void setGravity(final int gravity)
    {
        runViewAction(new Runnable(){
            @Override
            public void run(){
                ((MyEditText)getView()).setGravity(gravity);
            }
        });
    }

    void setHeight(final int pixels)
    {
        runViewAction(new Runnable(){
            @Override
            public void run(){
                ((MyEditText)getView()).setHeight(pixels);
            }
        });
    }

    void setHighlightColor(final int color)
    {
        runViewAction(new Runnable(){
            @Override
            public void run(){
                ((MyEditText)getView()).setHighlightColor(color);
            }
        });
    }

    void setHint(final String hint)
    {
        runViewAction(new Runnable(){
            @Override
            public void run(){
                ((MyEditText)getView()).setHint(hint);
            }
        });
    }

    void setHintTextColor(final int color)
    {
        runViewAction(new Runnable(){
            @Override
            public void run(){
                ((MyEditText)getView()).setHintTextColor(color);
            }
        });
    }

    void setWidth(final int pixels)
    {
        runViewAction(new Runnable(){
            @Override
            public void run(){
                ((MyEditText)getView()).setWidth(pixels);
            }
        });
    }

    void setLineSpacing(final float add, final float mult)
    {
        runViewAction(new Runnable(){
            @Override
            public void run(){
                ((MyEditText)getView()).setLineSpacing(add, mult);
            }
        });
    }

    void setLines(final int lines)
    {
        runViewAction(new Runnable(){
            @Override
            public void run(){
                ((MyEditText)getView()).setLines(lines);
            }
        });
    }

    void setHorizontallyScrolling(final boolean whether)
    {
        runViewAction(new Runnable(){
            @Override
            public void run(){
                ((MyEditText)getView()).setHorizontallyScrolling(whether);
            }
        });
    }

    void setAllCaps(final boolean allCaps)
    {
        runViewAction(new Runnable(){
            @Override
            public void run(){
                ((MyEditText)getView()).setAllCaps(allCaps);
            }
        });
    }

    void selectAll()
    {
        runViewAction(new Runnable(){
            @Override
            public void run(){
                ((MyEditText)getView()).selectAll();
            }
        });
    }

    void setSelection(final int index)
    {
        runViewAction(new Runnable(){
            @Override
            public void run(){
                ((MyEditText)getView()).setSelection(index);
            }
        });
    }

    void setSelection(final int start, final int stop)
    {
        runViewAction(new Runnable(){
            @Override
            public void run(){
                ((MyEditText)getView()).setSelection(start, stop);
            }
        });
    }

    void setAllowFullscreenKeyboard(final boolean allow)
    {
        runViewAction(new Runnable(){
            @Override
            public void run(){
                // Log.i(TAG, "setAllowFullscreenKeyboard "+allow);
                MyEditText met = (MyEditText)getView();
                int ops = met.getImeOptions();
                if (allow)
                {
                    ops &= ~EditorInfo.IME_FLAG_NO_EXTRACT_UI;
                }
                else
                {
                    ops |= EditorInfo.IME_FLAG_NO_EXTRACT_UI;
                }
                met.setImeOptions(ops);
            }
        });
    }
}

