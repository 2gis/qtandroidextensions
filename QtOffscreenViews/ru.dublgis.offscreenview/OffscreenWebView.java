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
import android.webkit.WebResourceResponse;
import android.webkit.HttpAuthHandler;
import android.webkit.SslErrorHandler;
import android.widget.LinearLayout;
import android.graphics.Canvas;

class OffscreenWebView extends OffscreenView
{
    MyWebView webview_ = null;
    boolean is_visible_ = true;

    class MyWebView extends WebView
    {
        // TODO: pauseTimers/ resumeTimers ()
        // http://developer.android.com/reference/android/webkit/WebView.html
        //  public void setInitialScale (int scaleInPercent) (0 = default)

        int width_ = 0, height_ = 0;
        boolean invalidated_ = true;

        private class MyWebViewClient extends WebViewClient
        {
            @Override
            public void doUpdateVisitedHistory(WebView view, String url, boolean isReload) { OffscreenWebView.this.doUpdateVisitedHistory(getNativePtr(), url, isReload); }
            @Override
            public void onFormResubmission(WebView view, Message dontResend, Message resend) { OffscreenWebView.this.onFormResubmission(getNativePtr(), dontResend, resend); }
            @Override
            public void onLoadResource(WebView view, String url) { OffscreenWebView.this.onLoadResource(getNativePtr(), url); }
            @Override
            public void onPageFinished(WebView view, String url) { OffscreenWebView.this.onPageFinished(getNativePtr(), url); }
            @Override
            public void onPageStarted(WebView view, String url, Bitmap favicon) { OffscreenWebView.this.onPageStarted(getNativePtr(), url, favicon); }
            @Override
            public void onReceivedError(WebView view, int errorCode, String description, String failingUrl) { OffscreenWebView.this.onReceivedError(getNativePtr(), errorCode, description, failingUrl); }
            @Override
            public void onReceivedHttpAuthRequest(WebView view, HttpAuthHandler handler, String host, String realm) { OffscreenWebView.this.onReceivedHttpAuthRequest(getNativePtr(), handler, host, realm); }
            @Override
            public void onReceivedLoginRequest(WebView view, String realm, String account, String args) { OffscreenWebView.this.onReceivedLoginRequest(getNativePtr(), realm, account, args); }
            @Override
            public void onReceivedSslError(WebView view, SslErrorHandler handler, SslError error) { OffscreenWebView.this.onReceivedSslError(getNativePtr(), handler, error); }
            @Override
            public void onScaleChanged(WebView view, float oldScale, float newScale) { OffscreenWebView.this.onScaleChanged(getNativePtr(), oldScale, newScale); }
            @Override
            public void onTooManyRedirects(WebView view, Message cancelMsg, Message continueMsg) { OffscreenWebView.this.onTooManyRedirects(getNativePtr(), cancelMsg, continueMsg); }
            @Override
            public void onUnhandledKeyEvent(WebView view, KeyEvent event) { OffscreenWebView.this.onUnhandledKeyEvent(getNativePtr(), event); }
            @Override
            public WebResourceResponse shouldInterceptRequest(WebView view, String url) { return OffscreenWebView.this.shouldInterceptRequest(getNativePtr(), url); }
            @Override
            public boolean shouldOverrideKeyEvent(WebView view, KeyEvent event) { return OffscreenWebView.this.shouldOverrideKeyEvent(getNativePtr(), event); }
            @Override
            public boolean shouldOverrideUrlLoading(WebView view, String url) { return OffscreenWebView.this.shouldOverrideUrlLoading(getNativePtr(), url); }
        }

        public MyWebView(Context context)
        {
            super(context);
            Log.i(TAG, "MyWebView constructor");
            setWebViewClient(new MyWebViewClient());

            // Fill in default properties
            setHorizontalScrollBarEnabled(false);
            setVerticalScrollBarEnabled(true);
            resizeOffscreenView(256, 256);
            getSettings().setJavaScriptEnabled(true);

            // No positive effect found
            // setPersistentDrawingCache(PERSISTENT_ALL_CACHES);
            // setAlwaysDrawnWithCacheEnabled(true);
        }

        public void setOffscreenViewVisible(boolean visible)
        {
            Log.i(TAG, "MyWebView.setOffscreenViewVisible "+visible);
            if (visible)
            {
                setVisibility(View.VISIBLE);
                resumeTimers();
            }
            else
            {
                setVisibility(View.INVISIBLE);
                pauseTimers();
            }
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
            // Log.i(TAG, "MyWebView.invalidate(int l, int t, int r, int b) "+l+", "+t+", "+r+", "+b+
            //    "; width="+width_+", height="+height_+"; scrollX="+getScrollX()+", scrollY="+getScrollY());
            super.invalidate(l, t, r, b);
            int my_r = getScrollX() + width_;
            int my_b = getScrollY() + height_;
            // Check that the invalidated rectangle actually visible
            if (l > my_r || t > my_b)
            {
                // Log.i(TAG, "MyWebView.invalidate: ignoring invisible rectangle");
                return;
            }
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
               runOnUiThread(new Runnable() {
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
        webview_.setOffscreenViewVisible(is_visible_);
    }

    @Override
    public View getView()
    {
        return webview_;
    }

    @Override
    public void callViewPaintMethod(Canvas canvas)
    {
        if (webview_ != null)
        {
            webview_.onDrawPublic(canvas);
        }
    }

    @Override
    public void doInvalidateOffscreenView()
    {
        if (webview_ != null)
        {
            webview_.invalidateTexture();
        }
    }

    @Override
    public void doResizeOffscreenView(final int width, final int height)
    {
        if (webview_ != null)
        {
            webview_.resizeOffscreenView(width, height);
        }
    }

    @Override
    public boolean isVisible()
    {
        return is_visible_;
    }

    @Override
    public void setVisible(final boolean visible)
    {
        if (visible != is_visible_)
        {
            is_visible_ = visible;
            if (webview_ != null)
            {
                runOnUiThread(new Runnable(){
                    @Override
                    public void run()
                    {
                        webview_.setOffscreenViewVisible(visible);
                        webview_.invalidateTexture();
                    }
                });
            }
        }
    }






    // From C++
    public void loadUrl(final String url)
    {
        Log.i(TAG, "loadUrl: scheduling");
        if (webview_ != null)
        {
            runOnUiThread(new Runnable() {
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
    /*!
     * \param additionalHttpHeaders contains additional HTTP headers in this format:
     * header1\n
     * value\1n
     * header2\n
     * value2\n
     * ...
     */
    public void loadUrl(final String url, final String additionalHttpHeaders)
    {
        Log.i(TAG, "loadUrl: scheduling");
        if (webview_ != null)
        {
            runOnUiThread(new Runnable() {
                @Override
                public void run()
                {
                    Log.i(TAG, "loadUrl: doing it");
                    Map<String, String> ma = new HashMap<String, String>();
                    String[] parts = additionalHttpHeaders.split("\n");
                    for (int i = 0, h = 0, v = 1; i < parts.length / 2; i++, h += 2, v += 2)
                    {
                        ma.put(parts[h], parts[v]);
                    }
                    webview_.loadUrl(url, ma);
                }
            });
        }
        else
        {
            Log.e(TAG, "loadUrl failed because context or view is not available!");
        }
    }


    // From C++
    public void loadData(final String text, final String mime, final String encoding)
    {
        Log.i(TAG, "loadData: scheduling");
        if (webview_ != null)
        {
            runOnUiThread(new Runnable(){
                @Override
                public void run()
                {
                    Log.i(TAG, "loadData: doing it");
                    webview_.loadData(text, mime, encoding);
                }
            });
        }
        else
        {
            Log.e(TAG, "loadData failed because context or view is not available!");
        }
    }

    // From C++
    public void loadDataWithBaseURL(final String baseUrl, final String data, final String mimeType, final String encoding, final String historyUrl)
    {
        Log.i(TAG, "loadDataWithBaseURL: scheduling");
        if (webview_ != null)
        {
            runOnUiThread(new Runnable() {
                @Override
                public void run()
                {
                    Log.i(TAG, "loadDataWithBaseURL: doing it");
                    webview_.loadDataWithBaseURL(baseUrl, data, mimeType, encoding, historyUrl);
                }
            });
        }
        else
        {
            Log.e(TAG, "loadDataWithBaseURL failed because context or view is not available!");
        }
    }

    public boolean requestContentHeight()
    {
        if (webview_ != null)
        {
            runOnUiThread(new Runnable() {
                @Override
                public void run()
                {
                    onContentHeightReceived(getNativePtr(), webview_.getContentHeight());
                }
            });
            return true;
        }
        return false;
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
        Activity a = nativeGetActivity();
        if (a == null)
        {
            Log.w(TAG, "getActivity: NULL ACTIVITY");
        }
        return a;
    }

    // WebViewClient
    public native void doUpdateVisitedHistory(long nativeptr, String url, boolean isReload);
    public native void onFormResubmission(long nativeptr, Message dontResend, Message resend);
    public native void onLoadResource(long nativeptr, String url);
    public native void onPageFinished(long nativeptr, String url);
    public native void onPageStarted(long nativeptr, String url, Bitmap favicon);
    public native void onReceivedError(long nativeptr, int errorCode, String description, String failingUrl);
    public native void onReceivedHttpAuthRequest(long nativeptr, HttpAuthHandler handler, String host, String realm);
    public native void onReceivedLoginRequest(long nativeptr, String realm, String account, String args);
    public native void onReceivedSslError(long nativeptr, SslErrorHandler handler, SslError error);
    public native void onScaleChanged(long nativeptr, float oldScale, float newScale);
    public native void onTooManyRedirects(long nativeptr, Message cancelMsg, Message continueMsg);
    public native void onUnhandledKeyEvent(long nativeptr, KeyEvent event);
    public native WebResourceResponse shouldInterceptRequest(long nativeptr, String url);
    public native boolean shouldOverrideKeyEvent(long nativeptr, KeyEvent event);
    public native boolean shouldOverrideUrlLoading(long nativeptr, String url);

    // Own callbacks
    public native void onContentHeightReceived(long nativeptr, int height);

}

