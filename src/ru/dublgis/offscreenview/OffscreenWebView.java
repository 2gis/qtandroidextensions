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
import android.graphics.Paint;
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
import android.view.KeyEvent;
import android.view.KeyCharacterMap;
import android.view.Menu;
import android.view.MenuItem;
import android.view.Window;
import android.view.WindowManager;
import android.view.WindowManager.LayoutParams;
import android.view.inputmethod.InputMethodManager;
import android.webkit.WebView;
import android.graphics.Canvas;

import org.qt.core.QtApplicationBase;

import ru.dublgis.offscreenview.OffscreenViewHelper;

class OffscreenWebView extends WebView implements OffscreenView
{
    public static final String TAG = "Grym/OffscreenView";
    OffscreenViewHelper helper_;

    OffscreenWebView(final String objectname, int gltextureid, int width, int height)
    {
        super(getContextStatic());
        Log.i(TAG, "OffscreenWebView(name=\""+objectname+"\", texture="+gltextureid+")");
        helper_ = new OffscreenViewHelper(objectname, (View)this, gltextureid, width, height);

// !!!! WORKAROUND FOR CHROMIUM !!!!
//setLayerType(View.LAYER_TYPE_SOFTWARE, null);

onAttachedToWindow();
onSizeChanged(width, height, 0, 0);
onVisibilityChanged(this, View.VISIBLE);
onLayout(true, 0, 0, width, height);

        // getSettings().setJavaScriptEnabled(true);
        // webview.loadUrl("http://slashdot.org/");
        // OR, you can also load from an HTML string:
        // String summary = "<html><body>You scored <b>192</b> points.</body></html>";
        // webview.loadData(summary, "text/html", null);
        loadData("<html><body style=\"background-color: orange;\"><h1>Teach Me To Web</h1></body></html>", "text/html", null);
    }

    @Override
    protected void onDraw(Canvas canvas)
    {
        Log.i(TAG, "OffscreenWebView.onDraw tid="+Thread.currentThread().getId());
        drawViewOnTexture();
    }

    // http://developer.android.com/reference/android/view/View.html

    static private Context getContextStatic()
    {
        return QtApplicationBase.getActivityStatic();
    }

    static Bitmap bitmap_ = null;
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
    }


    @Override
    public void drawViewOnTexture()
    {
        Log.i(TAG, "OffscreenWebView.drawViewOnTexture tid="+Thread.currentThread().getId()+
            " texture="+helper_.getTexture()+", size="+helper_.getTextureWidth()+"x"+helper_.getTextureHeight());
        try
        {
            Canvas canvas = helper_.lockCanvas();
            if (canvas == null)
            {
                Log.e(TAG, "drawViewOnTexture: failed to lock canvas!");
            }
            else
            {
//                super.onDraw(canvas);

canvas.drawARGB (255, 255, 100, 0);
Paint paint = new Paint();
paint.setColor(Color.BLACK);
// paint.setAntiAlias(true);
canvas.drawLine(0, 0, 400, 400, paint);

CheckBitmap();
if (bitmap_ != null){
Log.i(TAG, ">>>>>>>>>>>>>>>>> PAINTING <<<<<<<<<<<<<<<<<<<<");
canvas.drawBitmap(bitmap_, 0, 0, paint);
}else Log.i(TAG, "!!!!!!!!!!!!!!!! NO BITMAP !!!!!!!!!!!!!!!!!!!");


                helper_.unlockCanvas(canvas);
                Log.i(TAG, "OffscreenWebView.drawViewOnTexture success.");
            }
        }
        catch(Exception e)
        {
            Log.e(TAG, "drawViewOnTexture failed", e);
        }
    }

    @Override
    public void updateTexture()
    {
        helper_.updateTexture();
    }

    @Override
    public float getTextureTransformMatrix(int index)
    {
        return helper_.getTextureTransformMatrix(index);
    }

// protected void onSizeChanged (int w, int h, int oldw, int oldh) 

 // Repainting: invalidate().
 
 /*Event processing onKeyDown(int, KeyEvent) Called when a new hardware key event occurs.
onKeyUp(int, KeyEvent) Called when a hardware key up event occurs.
onTrackballEvent(MotionEvent) Called when a trackball motion event occurs.
onTouchEvent(MotionEvent) Called when a touch screen motion event occurs. */
}



