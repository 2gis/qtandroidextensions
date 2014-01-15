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
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Looper;
import android.os.SystemClock;
import android.text.method.MetaKeyKeyListener;
import android.text.InputType;
import android.util.Log;
import android.util.DisplayMetrics;
import android.view.ContextMenu;
import android.view.ContextMenu.ContextMenuInfo;
import android.view.View;
import android.view.ViewParent;
import android.view.KeyEvent;
import android.view.KeyCharacterMap;
import android.view.Menu;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.Window;
import android.view.WindowManager;
import android.view.WindowManager.LayoutParams;
import android.view.inputmethod.InputMethodManager;
import android.webkit.WebView;
import android.webkit.WebViewClient;
import android.widget.LinearLayout;
import android.graphics.Canvas;
import org.qt.core.QtApplicationBase;

class OffscreenWebView extends OffscreenView
{
    MyWebView webview_ = null;

    class MyWebView extends WebView
    {
        // TODO: pauseTimers/ resumeTimers ()
        // http://developer.android.com/reference/android/webkit/WebView.html
        //  public void setInitialScale (int scaleInPercent) (0 = default)

        int width_ = 0, height_ = 0;
        MyWebViewClient webviewclient_ = null;
        boolean invalidated_ = true;

        private class MyWebViewClient extends WebViewClient
        {
            @Override
            public boolean shouldOverrideUrlLoading(WebView view, String url)
            {
                // This should always be done for Chrome to avoid opening links in external browser.
                view.loadUrl(url);
                return true;
            }

            @Override
            public void onPageFinished(WebView view, String url)
            {
                Log.i(TAG, "MyWebView.MyWebViewClient.onPageFinished");
                nativeUpdate(getNativePtr());
                super.onPageFinished(view, url);
            }

            @Override
            public void onLoadResource(WebView view, String url)
            {
                /*(if (url.equals("http://redirectexample.com"))
                {
                ...
                }
                else
                {
                    super.onLoadResource(view, url);
                }*/
                super.onLoadResource(view, url);
            }
        }

        public MyWebView(Context context)
        {
            super(context);
            Log.i(TAG, "MyWebView constructor");
            webviewclient_ = new MyWebViewClient();
            setWebViewClient(webviewclient_);

            // Fill in default properties
            setHorizontalScrollBarEnabled(false);
            setVerticalScrollBarEnabled(true);
            resizeOffscreenView(256, 256);
            getSettings().setJavaScriptEnabled(true);
        }

        // NB: caller should call the invalidation
        public void resizeOffscreenView(int width, int height)
        {
            onSizeChanged(width, height, width_, height_);
            width_ = width;
            height_ = height;
            setLeft(0);
            setRight(width-1);
            setTop(0);
            setBottom(height-1);
        }

        public void onDrawPublic(Canvas canvas)
        {

            Log.i(TAG, "MyWebView.onDrawPublic "+getWidth()+"x"+getHeight());
            canvas.drawARGB (255, 255, 255, 255);

            // Take View scroll into account. (It converts touch coordinates by itself,
            // but it doesn't draw scrolled).
            canvas.translate(-getScrollX(), -getScrollY());

            super.onDraw(canvas);

            /*
            Paint paint = new Paint();
            paint.setColor(Color.BLUE);
            canvas.drawLine(0, 0, getWidth()-1, getHeight()-1, paint);
            canvas.drawLine(getWidth()-1, 0, 0, getHeight()-1, paint);
            canvas.drawLine(0, 0, getWidth()-1, 0, paint);
            canvas.drawLine(0, getHeight()-1, getWidth()-1, getHeight()-1, paint);
            canvas.drawLine(0, 0, 0, getHeight()-1, paint);
            canvas.drawLine(getWidth()-1, 0, getWidth()-1, getHeight()-1, paint);
            // CheckBitmap(); if (bitmap_ != null){ canvas.drawBitmap(bitmap_, 200, 2000, paint); }
            */
        }

        public void invalidateTexture()
        {
            invalidated_ = true;
            // A little dance with a tambourine to filter out subsequent invalidations happened before a single paint
            new Handler().post(new Runnable(){
                @Override
                public void run()
                {
                    Log.i(TAG, "invalidateTexture: processing with invalidated_="+invalidated_);
                    if (invalidated_)
                    {
                        invalidated_ = false;
                        doDrawViewOnTexture();
                    }
                }
            });
        }

        // Old WebKit updating
        @Override
        public void invalidate(Rect dirty)
        {
            Log.i(TAG, "MyWebView.invalidate(Rect dirty)");
            super.invalidate(dirty);
            invalidateTexture();
        }

        // Old WebKit updating
        @Override
        public void invalidate(int l, int t, int r, int b)
        {
            Log.i(TAG, "MyWebView.invalidate(int l, int t, int r, int b) "+l+", "+t+", "+r+", "+b);
            super.invalidate(l, t, r, b);
            invalidateTexture();
        }

        // Old WebKit updating
        @Override
        public void invalidate()
        {
            Log.i(TAG, "MyWebView.invalidate()");
            super.invalidate();
            invalidateTexture();
        }

        /*// ????
        @Override
        public ViewParent invalidateChildInParent(int[] location, Rect r)
        {
            Log.i(TAG, "MyWebView.invalidateChildInParent(int[] location, Rect r)");
            invalidateTexture();
            return super.invalidateChildInParent(location, r);
        }*/

        // Chromium updating
        @Override
        public void requestLayout()
        {
            Log.i(TAG, "MyWebView.requestLayout()");
            if (rendering_surface_ != null)
            {
               final Activity context = getActivity();
               context.runOnUiThread(new Runnable()
               {
                   @Override
                   public void run()
                   {
                       onLayout(true, 0, 0, width_, height_);
                       invalidateTexture();
                   }
               });
            }
            super.requestLayout();
        }

        /*
        // ????
        @Override
        public void invalidateDrawable(Drawable drawable)
        {
            Log.i(TAG, "MyWebView.invalidateDrawable()");
            invalidateTexture();
        }

        // ????
        @Override
        public void scheduleDrawable(Drawable who, Runnable what, long when)
        {
            Log.i(TAG, "MyWebView.scheduleDrawable()");
            invalidateTexture();
        }

        // ????
        @Override
        public void childDrawableStateChanged(View child)
        {
            Log.i(TAG, "MyWebView.childDrawableStateChanged()");
            invalidateTexture();
            super.childDrawableStateChanged(child);
        }
        */

        //  public boolean isDirty () 
    }

    OffscreenWebView()
    {
        // Log.i(TAG, "OffscreenWebView constructor");
    }

    @Override
    public void doCreateView()
    {
        final Activity context = getActivity();
        webview_ = new MyWebView(context);
    }

    @Override
    public View getView()
    {
        return webview_;
    }

    public void callViewPaintMethod(Canvas canvas)
    {
        if (webview_ != null)
        {
            webview_.onDrawPublic(canvas);
        }
    }

    public void doInvalidateOffscreenView()
    {
        if (webview_ != null)
        {
            webview_.invalidateTexture();
        }
    }

    public void doResizeOffscreenView(final int width, final int height)
    {
        if (webview_ != null)
        {
            webview_.resizeOffscreenView(width, height);
        }
    }


    // From C++
    public void loadUrl(final String url)
    {
        Log.i(TAG, "loadUrl: scheduling");
        Activity ctx = getActivity();
        if (ctx != null && webview_ != null)
        {
            ctx.runOnUiThread(new Runnable()
            {
                @Override
                public void run()
                {
                    Log.i(TAG, "loadUrl: doing it");
                    webview_.loadUrl(url);
                }
            });
        }
        else
        {
            Log.e(TAG, "loadUrl failed because context or view is not available!");
        }
    }

    // From C++
    public void loadData(final String text, final String mime)
    {
        Log.i(TAG, "loadData: scheduling");
        Activity ctx = getActivity();
        if (ctx != null && webview_ != null)
        {
            ctx.runOnUiThread(new Runnable()
            {
                @Override
                public void run()
                {
                    Log.i(TAG, "loadData: doing it");
                    webview_.loadData(text, mime, null);
                }
            });
        }
        else
        {
            Log.e(TAG, "loadData failed because context or view is not available!");
        }
    }


/*Event processing onKeyDown(int, KeyEvent) Called when a new hardware key event occurs.
onKeyUp(int, KeyEvent) Called when a hardware key up event occurs.
onTrackballEvent(MotionEvent) Called when a trackball motion event occurs.
onTouchEvent(MotionEvent) Called when a touch screen motion event occurs. */

    private native void nativeUpdate(long nativeptr);
    private native Activity nativeGetActivity();

    @Override
    public void doNativeUpdate()
    {
        nativeUpdate(getNativePtr());
    }

    @Override
    public Activity getActivity()
    {
        return nativeGetActivity();
    }
}

