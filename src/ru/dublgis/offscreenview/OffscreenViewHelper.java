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
import android.graphics.Canvas;
import android.graphics.Rect;
import android.graphics.SurfaceTexture;
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
import android.view.Surface;
import android.view.Window;
import android.view.WindowManager;
import android.view.WindowManager.LayoutParams;
import android.view.inputmethod.InputMethodManager;

class OffscreenViewHelper
{
    public static final String TAG = "OffscreenView";

    int gl_texture_id_ = 0;
    int width_ = 64;
    int height_ = 64;
    SurfaceTexture surface_texture_ = null;
    Surface surface_ = null;
    View view_;
    String object_name_ = null;

    public OffscreenViewHelper(String objectname, View view, int gltextureid, int width, int height)
    {
        Log.i(TAG, "OffscreenViewHelper(obj=\""+objectname+"\", texture="+gltextureid+", w="+width+", h="+height);

        view_ = view;
        gl_texture_id_ = gltextureid;
        width_ = width;
        height_ = height;
        object_name_ = objectname;

        surface_texture_ = new SurfaceTexture(gltextureid);
        surface_ = new Surface(surface_texture_);

        surface_texture_.setDefaultBufferSize(width, height); // API 15
    }

    protected Canvas lockCanvas()
    {
        try
        {
            return surface_.lockCanvas(new Rect(0, 0, width_, height_));
        }
        catch(Exception e)
        {
            Log.e(TAG, "Failed to lock canvas", e);
            return null;
        }
    }

    protected void unlockCanvas(Canvas canvas)
    {
        try
        {
            if (canvas != null)
            {
                surface_.unlockCanvasAndPost(canvas);
            }
        }
        catch(Exception e)
        {
            Log.e(TAG, "Failed to unlock canvas", e);
        }
    }


/*
    The steps to render your view to OpenGL:
    Initialize an OpenGL texture
    Within an OpenGL context construct a SurfaceTexture with the texture id. Use SurfaceTexture.setDefaultBufferSize(int width, int height) to make sure you have enough space on the texture for the view to render.
    Create a Surface constructed with the above SurfaceTexture.
    Within the View’s onDraw, use the Canvas returned by Surface.lockCanvas to do the view drawing. 
    You can obviously do this with any View, and not just WebView. Plus Canvas has a whole bunch of drawing methods, allowing you to do funky, funky things.
*/
}


