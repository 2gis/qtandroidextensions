package ru.dublgis.offscreenview;

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
    public static final String TAG = "OffscreenView";
    OffscreenViewHelper helper_;

    OffscreenWebView(final String objectname, int gltextureid, int width, int height)
    {
        super(getContextStatic());
        Log.i(TAG, "OffscreenWebView(name=\""+objectname+"\", texture="+gltextureid+")");
        helper_ = new OffscreenViewHelper(objectname, (View)this, gltextureid, width, height);

onAttachedToWindow();
onSizeChanged(width, height, 0, 0);
onVisibilityChanged(this, View.VISIBLE);
onLayout(true, 0, 0, width, height);

        // getSettings().setJavaScriptEnabled(true);
        // webview.loadUrl("http://slashdot.org/");
        // OR, you can also load from an HTML string:
        // String summary = "<html><body>You scored <b>192</b> points.</body></html>";
        // webview.loadData(summary, "text/html", null);
        loadData("<html><body><h1>Teach Me To Web</h1></body></html>", "text/html", null);
    }

    @Override
    protected void onDraw(Canvas canvas)
    {
        Log.i(TAG, "OffscreenWebView.onDraw");
        updateGlTexture();
    }

    // http://developer.android.com/reference/android/view/View.html

    static private Context getContextStatic()
    {
        return QtApplicationBase.getActivityStatic();
    }

    @Override
    public void updateGlTexture()
    {
        Log.i(TAG, "OffscreenWebView.updateGlTexture");
        try
        {
            Canvas canvas = helper_.lockCanvas();
            if (canvas == null)
            {
                Log.e(TAG, "updateGlTexture: failed to lock canvas!");
            }
            else
            {
                super.onDraw(canvas);
                helper_.unlockCanvas(canvas);
            }
        }
        catch(Exception e)
        {
            Log.e(TAG, "updateGlTexture failed", e);
        }
    }

// protected void onSizeChanged (int w, int h, int oldw, int oldh) 

 // Repainting: invalidate().
 
 /*Event processing onKeyDown(int, KeyEvent) Called when a new hardware key event occurs.
onKeyUp(int, KeyEvent) Called when a hardware key up event occurs.
onTrackballEvent(MotionEvent) Called when a trackball motion event occurs.
onTouchEvent(MotionEvent) Called when a touch screen motion event occurs. */
}



