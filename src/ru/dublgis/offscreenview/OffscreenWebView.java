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
import android.os.Looper;
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
import android.view.Window;
import android.view.WindowManager;
import android.view.WindowManager.LayoutParams;
import android.view.inputmethod.InputMethodManager;
import android.webkit.WebView;
import android.webkit.WebViewClient;
import android.widget.LinearLayout;
import android.graphics.Canvas;

import org.qt.core.QtApplicationBase;

import ru.dublgis.offscreenview.OffscreenViewHelper;

class OffscreenWebView implements OffscreenView
{
    public static final String TAG = "Grym/OffscreenView";
    OffscreenViewHelper helper_ = null;
    MyWebView webview_ = null;

    class MyWebView extends WebView
    {
        // TODO: pauseTimers/ resumeTimers ()
        // http://developer.android.com/reference/android/webkit/WebView.html
        //  public void setInitialScale (int scaleInPercent) (0 = default)

        int width_ = 100, height_ = 100;
        MyWebViewClient webviewclient_ = null;

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
                if (helper_ != null)
                {
                     nativeUpdate(helper_.getNativePtr());
                }
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

        public MyWebView(Context context, int width, int height)
        {
            super(context);
            Log.i(TAG, "MyWebView "+width+"x"+height);
            width_ = width;
            height_ = height;
            webviewclient_ = new MyWebViewClient();
            setWebViewClient(webviewclient_);
            /*onAttachedToWindow();
            onSizeChanged(width, height, 0, 0);
            onVisibilityChanged(this, View.VISIBLE);
            onLayout(true, 0, 0, width, height);*/
            setLeft(0);
            setRight(width-1);
            setTop(0);
            setBottom(height-1);
        }

        public void onDrawPublic(Canvas canvas)
        {

            Log.i(TAG, "MyWebView.onDrawPublic "+getWidth()+"x"+getHeight());
            canvas.drawARGB (255, 255, 255, 255);
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

        /*@Override
        protected void dispatchDraw(Canvas canvas)
        {
             Log.i(TAG, "MyWebView.dispatchDraw");
             super.dispatchDraw(canvas);
        }*/

        /*@Override
        protected void onDraw (Canvas canvas)
        {
            Log.i(TAG, "MyWebView.onDraw");
            if (helper_ != null)
            {
                 nativeUpdate(helper_.getNativePtr());
            }
        }*/

        // Old WebKit updating
        @Override
        public void invalidate(Rect dirty)
        {
            Log.i(TAG, "MyWebView.invalidate(Rect dirty)");
            if (helper_ != null)
            {
                 nativeUpdate(helper_.getNativePtr());
            }
            // super.invalidate(dirty);
        }

        // Old WebKit updating
        @Override
        public void invalidate(int l, int t, int r, int b)
        {
            Log.i(TAG, "MyWebView.invalidate(int l, int t, int r, int b)");
            if (helper_ != null)
            {
                 nativeUpdate(helper_.getNativePtr());
            }
            // super.invalidate(l, t, r, b);
        }

        // Old WebKit updating
        @Override
        public void invalidate()
        {
            Log.i(TAG, "MyWebView.invalidate()");
            if (helper_ != null)
            {
                 nativeUpdate(helper_.getNativePtr());
            }
            // super.invalidate();
        }

        /*@Override
        public ViewParent invalidateChildInParent(int[] location, Rect r)
        {
            Log.i(TAG, "MyWebView.invalidateChildInParent(int[] location, Rect r)");
            if (helper_ != null)
            {
                 nativeUpdate(helper_.getNativePtr());
            }
            return super.invalidateChildInParent(location, r);
        }*/

        // Chromium updating
        @Override
        public void requestLayout()
        {
            Log.i(TAG, "MyWebView.requestLayout()");
            if (helper_ != null)
            {
               final Activity context = getContextStatic();
               context.runOnUiThread(new Runnable()
               {
                   @Override
                   public void run()
                   {
                       onLayout(true, 0, 0, width_, height_);
                       nativeUpdate(helper_.getNativePtr());
                   }
               });
            }
            super.requestLayout();
        }

      /*  @Override
        public void invalidateDrawable(Drawable drawable)
        {
            Log.i(TAG, "MyWebView.invalidateDrawable()");
            if (helper_ != null)
            {
                 nativeUpdate(helper_.getNativePtr());
            }
        }

        @Override
        public void scheduleDrawable(Drawable who, Runnable what, long when)
        {
            Log.i(TAG, "MyWebView.scheduleDrawable()");
            if (helper_ != null)
            {
                 nativeUpdate(helper_.getNativePtr());
            }
        }
        
        @Override
        public void childDrawableStateChanged(View child)
        {
            Log.i(TAG, "MyWebView.childDrawableStateChanged()");
            if (helper_ != null)
            {
                 nativeUpdate(helper_.getNativePtr());
            }
            super.childDrawableStateChanged(child);
        }*/

        //  public boolean isDirty () 
    }

    OffscreenWebView(final String objectname, final long nativeptr, final int gltextureid, final int width, final int height)
    {
        super();
        final Activity context = getContextStatic();
        Log.i(TAG, "OffscreenWebView(name=\""+objectname+"\", texture="+gltextureid+")");
        context.runOnUiThread(new Runnable()
        {
            @Override
            public void run()
            {
                 webview_ = new MyWebView(context, width, height);
                 helper_ = new OffscreenViewHelper(nativeptr, objectname, webview_, gltextureid, width, height);
                 webview_.getSettings().setJavaScriptEnabled(true);
                 webview_.loadUrl("http://google.com/");
                 //webview_.loadData("<html><body style=\"background-color: green;\"><h1>Teach Me To Web</h1></body></html>", "text/html", null);
            }
        });
    }

    // http://developer.android.com/reference/android/view/View.html

    static private Activity getContextStatic()
    {
        return QtApplicationBase.getActivityStatic();
    }

    /*static Bitmap bitmap_ = null;
    private int DrawableResourceId(Context my_context, final String name)
    {
        return my_context.getResources().getIdentifier(name, "drawable", my_context.getPackageName());
    }
    private void CheckBitmap()
    {
        if (bitmap_ == null)
        {
            try
            {
                Context ctx = getContextStatic();
                int resource = DrawableResourceId(ctx, "kotik");
                BitmapFactory.Options ops = new BitmapFactory.Options();
                ops.inScaled = false;
                ops.inDensity = 0;
                ops.inTargetDensity = 0;
                ops.inScreenDensity = 0;
                bitmap_ = BitmapFactory.decodeResource(ctx.getResources(), resource, ops);
                bitmap_.setDensity(Bitmap.DENSITY_NONE);
            }
            catch(Exception e)
            {
                Log.e(TAG, "CheckBitmap error", e);
            }
        }
    }*/

    @Override
    public void drawViewOnTexture()
    {
        Activity ctx = getContextStatic();
        if (ctx != null)
        {
            ctx.runOnUiThread(new Runnable()
            {
                @Override
                public void run()
                {
                    doDrawViewOnTexture();
                }
            });
        }
    }

    private void doDrawViewOnTexture()
    {
        if (helper_ == null)
        {
            Log.i(TAG, "OffscreenWebView.drawViewOnTexture: helper is not initialized yet.");
            return;
        }
        Log.i(TAG, "OffscreenWebView.drawViewOnTexture tid="+Thread.currentThread().getId()+
            " texture="+helper_.getTexture()+", size="+helper_.getTextureWidth()+"x"+helper_.getTextureHeight());
        synchronized(helper_)
        {
            try
            {
                Canvas canvas = helper_.lockCanvas();
                if (canvas == null)
                {
                    Log.e(TAG, "drawViewOnTexture: failed to lock canvas!");
                }
                else
                {
                    if (webview_ != null)
                    {
                        Log.i(TAG, "Drawing WebView....");
                        webview_.onDrawPublic(canvas);
                    }
                    else
                    {
                        Log.i(TAG, "WebView is not available, filling with white color....");
                        canvas.drawARGB (255, 255, 255, 255);
                    }

                    helper_.unlockCanvas(canvas);
                    Log.i(TAG, "OffscreenWebView.drawViewOnTexture success.");
                }
            }
            catch(Exception e)
            {
                Log.e(TAG, "drawViewOnTexture failed", e);
            }
        }
    }

    @Override
    public void updateTexture()
    {
        synchronized(helper_)
        {
            helper_.updateTexture();
        }
    }

    @Override
    public float getTextureTransformMatrix(int index)
    {
        synchronized(helper_)
        {
            return helper_.getTextureTransformMatrix(index);
        }
    }

    // JNI
    public native void nativeUpdate(long nativeptr);

 // Repainting: invalidate().
 
 /*Event processing onKeyDown(int, KeyEvent) Called when a new hardware key event occurs.
onKeyUp(int, KeyEvent) Called when a hardware key up event occurs.
onTrackballEvent(MotionEvent) Called when a trackball motion event occurs.
onTouchEvent(MotionEvent) Called when a touch screen motion event occurs. */
}



