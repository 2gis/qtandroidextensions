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
import android.net.http.SslError;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.SystemClock;
import android.text.method.MetaKeyKeyListener;
import android.text.InputType;
import android.util.Log;
import android.util.DisplayMetrics;
import android.view.ContextMenu;
import android.view.ContextMenu.ContextMenuInfo;
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
    MyEditText edittext_ = null;

    class MyEditText extends EditText
    {
        boolean invalidated_ = true;

        public MyEditText(Context context)
        {
            super(context);
            Log.i(TAG, "MyEditText constructor");
            // Fill in default properties
            setText("Hello EditText");
        }

        @Override
        protected void onDraw(Canvas canvas)
        {
            // Don't draw when called from layout
        }

        public void onDrawPublic(Canvas canvas)
        {
            // Log.i(TAG, "MyEditText.onDrawPublic "+getWidth()+"x"+getHeight());
            // A text view has transparent background by default, which is not what we expect.
            canvas.drawARGB (fill_a_, fill_r_, fill_g_, fill_b_);
            // Take View scroll into account. (It converts touch coordinates by itself,
            // but it doesn't draw scrolled).
            canvas.translate(-getScrollX(), -getScrollY());
            super.onDraw(canvas);
        }

        public void invalidateTexture()
        {
            invalidated_ = true;
            // A little dance with a tambourine to filter out subsequent invalidations happened before a single paint
            new Handler().post(new Runnable(){
                @Override
                public void run()
                {
                    // Log.i(TAG, "invalidateTexture: processing with invalidated_="+invalidated_);
                    if (invalidated_)
                    {
                        invalidated_ = false;
                        doDrawViewOnTexture();
                    }
                }
            });
        }

        @Override
        public void invalidate(Rect dirty)
        {
            Log.i(TAG, "MyEditText.invalidate(Rect dirty)");
            super.invalidate(dirty);
            invalidateTexture();
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
            invalidateTexture();
        }

        @Override
        public void invalidate()
        {
            Log.i(TAG, "MyEditText.invalidate()");
            super.invalidate();
            invalidateTexture();
        }

        @Override
        public void requestLayout()
        {
            doInvalidateOffscreenView();
            super.requestLayout();
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
        final Activity context = getActivity();
        synchronized(view_existence_mutex_)
        {
            edittext_ = new MyEditText(context);
        }
    }

    @Override
    public View getView()
    {
        synchronized(view_existence_mutex_)
        {
            return edittext_;
        }
    }

    @Override
    public void callViewPaintMethod(Canvas canvas)
    {
        if (edittext_ != null)
        {
            edittext_.onDrawPublic(canvas);
        }
    }

    @Override
    public void doInvalidateOffscreenView()
    {
        if (edittext_ != null)
        {
            edittext_.invalidateTexture();
        }
    }

/*Event processing onKeyDown(int, KeyEvent) Called when a new hardware key event occurs.
onKeyUp(int, KeyEvent) Called when a hardware key up event occurs.
onTrackballEvent(MotionEvent) Called when a trackball motion event occurs.
onTouchEvent(MotionEvent) Called when a touch screen motion event occurs. */

    public native void nativeUpdate(long nativeptr);
    public native Activity nativeGetActivity();
    public native void nativeViewCreated(long nativeptr);

    @Override
    public void doNativeUpdate()
    {
        nativeUpdate(getNativePtr());
    }

    @Override
    public Activity getActivity()
    {
        Activity a = nativeGetActivity();
        if (a == null)
        {
            Log.w(TAG, "getActivity: NULL ACTIVITY");
        }
        return a;
    }

    @Override
    public void doNativeViewCreated()
    {
        nativeViewCreated(getNativePtr());
    }

}

