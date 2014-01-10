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

import ru.dublgis.offscreenview.OffscreenViewHelper;

class OffscreenWebView implements OffscreenView
{
    public static final String TAG = "Grym/OffscreenView";
    OffscreenViewHelper helper_ = null;
    MyWebView webview_ = null;

    // WebView wants everything to happen in UI thread only, so it's better to
    // make it a separate class inside of OffscreenWebView and call any of its functions
    // via runOnUiThread().
    class MyWebView extends WebView
    {
        // TODO: pauseTimers/ resumeTimers ()
        // http://developer.android.com/reference/android/webkit/WebView.html
        //  public void setInitialScale (int scaleInPercent) (0 = default)

        int width_ = 100, height_ = 100;
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
            webviewclient_ = new MyWebViewClient();
            setWebViewClient(webviewclient_);
            //setVerticalScrollBarEnabled(true);
            /*onAttachedToWindow();
            onSizeChanged(width, height, 0, 0);
            onVisibilityChanged(this, View.VISIBLE);
            onLayout(true, 0, 0, width, height);*/
            resizeOffscreenView(width, height);
        }

        // NB: caller should call the invalidation
        public void resizeOffscreenView(int width, int height)
        {
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
            }        }*/

        // Old WebKit updating
        @Override
        public void invalidate(Rect dirty)
        {
            Log.i(TAG, "MyWebView.invalidate(Rect dirty)");
            invalidateTexture();
            // super.invalidate(dirty);
        }

        // Old WebKit updating
        @Override
        public void invalidate(int l, int t, int r, int b)
        {
            Log.i(TAG, "MyWebView.invalidate(int l, int t, int r, int b) "+l+", "+t+", "+r+", "+b);
            invalidateTexture();
            // super.invalidate(l, t, r, b);
        }

        // Old WebKit updating
        @Override
        public void invalidate()
        {
            Log.i(TAG, "MyWebView.invalidate()");
            invalidateTexture();
            // super.invalidate();
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
            if (helper_ != null)
            {
               final Activity context = getContextStatic();
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
                 Log.i(TAG, "OffscreenWebView(name=\""+objectname+"\", texture="+gltextureid+") RUNNABLE");
                 // int save_req_orientation = context.getRequestedOrientation();
                 // Log.i(TAG, "SGEXP orientation was: "+save_req_orientation);
                 webview_ = new MyWebView(context, width, height);
                 helper_ = new OffscreenViewHelper(nativeptr, objectname, webview_, gltextureid, width, height);
                 webview_.getSettings().setJavaScriptEnabled(true);
                 webview_.loadUrl("http://www.android.com/intl/en/about/");
                 //webview_.loadUrl("http://www.youtube.com/watch?v=D36JUfE1oYk");
                 //webview_.loadUrl("http://beta.2gis.ru/");
                 // webview_.loadUrl("http://google.com/");
                 //webview_.loadUrl("http://beta.2gis.ru/novosibirsk/booklet/7?utm_source=products&utm_medium=mobile");
                 // TODO !!! Walkaround !!! Adding WebView disables automatic orientation changes on some devices (with Lite plug-in),
                 // have to figure out why.
                 context.setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_UNSPECIFIED);
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
            Log.i(TAG, "OffscreenWebView.doDrawViewOnTexture: helper is not initialized yet.");
            return;
        }
        Log.i(TAG, "OffscreenWebView.doDrawViewOnTexture tid="+Thread.currentThread().getId()+
            " texture="+helper_.getTexture()+", size="+helper_.getTextureWidth()+"x"+helper_.getTextureHeight());
        synchronized(helper_)
        {
            if (helper_.getNativePtr() == 0)
            {
                Log.i(TAG, "doDrawViewOnTexture: zero native ptr, will not draw!");
                return;
            }
            try
            {
                // TODO: disable time measurement
                long t = System.nanoTime();
                Canvas canvas = helper_.lockCanvas();
                if (canvas == null)
                {
                    Log.e(TAG, "doDrawViewOnTexture: failed to lock canvas!");
                }
                else
                {
                    try
                    {
                        if (webview_ != null)
                        {
                            // Log.i(TAG, "doDrawViewOnTexture: drawing WebView....");
                            webview_.onDrawPublic(canvas);
                        }
                        else
                        {
                            Log.i(TAG, "doDrawViewOnTexture: WebView is not available yet, filling with white color....");
                            canvas.drawARGB(255, 255, 255, 255);
                        }
                    }
                    catch(Exception e)
                    {
                        Log.e(TAG, "doDrawViewOnTexture painting failed!", e);
                    }

                    helper_.unlockCanvas(canvas);

                    t = System.nanoTime() - t;

                    // Tell C++ part that we have a new image
                    nativeUpdate(helper_.getNativePtr());

                    Log.i(TAG, "doDrawViewOnTexture: OffscreenWebView.doDrawViewOnTexture success, t="+t/1000000.0+"ms");
                }
            }
            catch(Exception e)
            {
                Log.e(TAG, "doDrawViewOnTexture failed!", e);
            }
        }
    }

    // TODO: add non-blocking version of updateTexture()?
    @Override
    public boolean updateTexture()
    {
        if (helper_ == null)
        {
            return false;
        }
        synchronized(helper_)
        {
            // long t = System.nanoTime();
            boolean result = helper_.updateTexture();
            // Log.i(TAG, "updateTexture: "+t/1000000.0+"ms");
            return result;
        }
    }

    @Override
    public void cppDestroyed()
    {
        if (helper_ == null)
        {
            return;
        }
        synchronized(helper_)
        {
            Log.i(TAG, "cppDestroyed");
            helper_.cppDestroyed();
        }
    }

    @Override
    public float getTextureTransformMatrix(int index)
    {
        if (helper_ == null)
        {
            return 0;
        }
        synchronized(helper_)
        {
            return helper_.getTextureTransformMatrix(index);
        }
    }

    // TODO: refactor downt
    long downt = 0;
    @Override
    public void ProcessMouseEvent(final int action, final int x, final int y)
    {
        if (helper_.getNativePtr() == 0)
        {
            Log.i(TAG, "ProcessMouseEvent: zero native ptr, ignoring.");
            return;
        }

        if (webview_ != null)
        {
            final long t = SystemClock.uptimeMillis();
            if (action == MotionEvent.ACTION_DOWN || downt == 0)
            {

                downt = t;
            }
            Log.i(TAG, "ProcessMouseEvent("+action+", "+x+", "+y+") downt="+downt+", t="+t);
            final MotionEvent event = MotionEvent.obtain(downt /* downTime*/, t /* eventTime */, action, x, y, 0 /*metaState*/);
            Activity ctx = getContextStatic();
            if (ctx != null)
            {
                ctx.runOnUiThread(new Runnable()
                {
                    @Override
                    public void run()
                    {
                        webview_.onTouchEvent(event);
                        // TODO: If the view has only been scrolled, it won't call invalidate(). So we just force it to repaint for now.
                        // We should keep a larger piece of rendered HTML in the texture and only scroll the texture if possible.
                        doDrawViewOnTexture();
                    }
                });
            }
        }
    }

    @Override
    public void invalidateOffscreenView()
    {
        if (webview_ != null)
        {
            Log.i(TAG, "invalidateOffscreenView");
            final Activity context = getContextStatic();
            context.runOnUiThread(new Runnable()
            {
                @Override
                public void run()
                {
                    webview_.invalidateTexture();
                }
            });
        }
    }

    @Override
    public void resizeOffscreenView(int w, int h)
    {
        if (helper_ == null)
        {
            return;
        }
        synchronized(helper_)
        {
            Log.i(TAG, "resizeOffscreenView "+w+"x"+h);
            if (helper_.getTextureWidth() != w || helper_.getTextureHeight() != h)
            {
                helper_.setNewSize(w, h);
                if (webview_ != null)
                {
                    webview_.resizeOffscreenView(w, h);
                    webview_.invalidateTexture();
                }
            }
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



