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
import  android.graphics.SurfaceTexture;
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
import android.view.Surface;
import android.view.Window;
import android.view.WindowManager;
import android.view.WindowManager.LayoutParams;
import android.view.inputmethod.InputMethodManager;
import android.webkit.WebView;
import android.webkit.WebViewClient;
import android.widget.LinearLayout;
import android.graphics.Canvas;

//! \todo Not compatible with official Qt5, used in getContextStatic()
import org.qt.core.QtApplicationBase;

abstract class OffscreenView
{
    public static final String TAG = "Grym/OffscreenView";
    protected OffscreenRenderingSurface rendering_surface_ = null;

    private String default_object_name_ = "UnnamedView";
    private int default_texture_id_ = 0;
    private int default_texture_width_ = 512;
    private int default_texture_height_ = 512;
    private long default_native_ptr_ = 0;
    public void SetObjectName(String name) { default_object_name_ = name; }
    public void SetTexture(int tex) { default_texture_id_ = tex; }
    public void SetInitialWidth(int w) { default_texture_width_ = w; }
    public void SetInitialHeight(int h) { default_texture_height_ = h; }
    public void SetNativePtr(long ptr) { default_native_ptr_ = ptr; }

    void initialize()
    {
        final Activity context = getContextStatic();
        Log.i(TAG, "OffscreenView.intialize(name=\""+default_object_name_+"\", texture="+default_texture_id_+")");
        context.runOnUiThread(new Runnable()
        {
            @Override
            public void run()
            {
                rendering_surface_ = new OffscreenRenderingSurface(default_native_ptr_, default_object_name_, getView()
                    , default_texture_id_, default_texture_width_, default_texture_height_);

                View view = getView();
                if (view != null)
                {
                    doResizeOffscreenView(default_texture_width_, default_texture_height_);
                }

                // TODO !!! Walkaround !!! Something disables automatic orientation changes on some devices (with Lite plug-in),
                // have to figure out why. [VNA-23]
                Activity context = getContextStatic();
                context.setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_UNSPECIFIED);
            }
        });
    }

    static protected Activity getContextStatic()
    {
        //! \todo Use a way compatible with Qt 5
        return QtApplicationBase.getActivityStatic();
    }

    abstract public View getView();
    abstract public void callViewPaintMethod(Canvas canvas);
    abstract public void doInvalidateOffscreenView();
    abstract public void doResizeOffscreenView(final int width, final int height);
    abstract public void doNativeUpdate(long nativeptr);

    //! Schedule painting of the view (will be done in Android UI thread).
    protected void drawViewOnTexture()
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

    //! Performs actual painting of the view. Should be called in Android UI thread.
    protected void doDrawViewOnTexture()
    {
        synchronized(this)
        {
            if (rendering_surface_ == null)
            {
                Log.i(TAG, "doDrawViewOnTexture: helper is not initialized yet.");
                return;
            }
            if (rendering_surface_.getNativePtr() == 0)
            {
                Log.i(TAG, "doDrawViewOnTexture: zero native ptr, will not draw!");
                return;
            }
            try
            {
                // TODO: disable time measurement
                long t = System.nanoTime();
                Canvas canvas = rendering_surface_.lockCanvas();
                if (canvas == null)
                {
                    Log.e(TAG, "doDrawViewOnTexture: failed to lock canvas!");
                }
                else
                {
                    try
                    {
                        View v = getView();
                        if (v != null)
                        {
                            Log.i(TAG, "doDrawViewOnTexture texture="+rendering_surface_.getTexture()+", helper's texSize="+
                                rendering_surface_.getTextureWidth()+"x"+rendering_surface_.getTextureHeight()+
                                ", view size:"+v.getWidth()+"x"+v.getHeight());
                            callViewPaintMethod(canvas);
                        }
                        else
                        {
                            //! \todo Add ability to set fill color
                            Log.i(TAG, "doDrawViewOnTexture: View is not available yet, filling with white color....");
                            canvas.drawARGB(255, 255, 255, 255);
                        }
                    }
                    catch(Exception e)
                    {
                        Log.e(TAG, "doDrawViewOnTexture painting failed!", e);
                    }

                    rendering_surface_.unlockCanvas(canvas);

                    t = System.nanoTime() - t;

                    // Tell C++ part that we have a new image
                    doNativeUpdate(rendering_surface_.getNativePtr());

                    Log.i(TAG, "doDrawViewOnTexture: success, t="+t/1000000.0+"ms");
                }
            }
            catch(Exception e)
            {
                Log.e(TAG, "doDrawViewOnTexture exception:", e);
            }
        }
    }

    //! Called from C++ to get current texture.
    public boolean updateTexture()
    {
        synchronized(this)
        {
            if (rendering_surface_ == null)
            {
                return false;
            }
            // long t = System.nanoTime();
            boolean result = rendering_surface_.updateTexture();
            // Log.i(TAG, "updateTexture: "+t/1000000.0+"ms");
            return result;
        }
    }

    //! Called from C++ to notify us that the associated C++ object is being destroyed.
    public void cppDestroyed()
    {
        synchronized(this)
        {
            Log.i(TAG, "cppDestroyed");
            if (rendering_surface_ == null)
            {
                return;
            }
            rendering_surface_.cppDestroyed();
        }
    }

    //! Called from C++ to get texture coordinate transformation matrix (filled in updateTexture()).
    public float getTextureTransformMatrix(int index)
    {
        synchronized(this)
        {
            if (rendering_surface_ == null)
            {
                return 0;
            }
            return rendering_surface_.getTextureTransformMatrix(index);
        }
    }

    // TODO: refactor downt
    private long downt = 0;
    public void ProcessMouseEvent(final int action, final int x, final int y)
    {
        if (rendering_surface_.getNativePtr() == 0)
        {
            Log.i(TAG, "ProcessMouseEvent: zero native ptr, ignoring.");
            return;
        }

        final View view = getView();
        if (view != null)
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
                        view.onTouchEvent(event);
                        // TODO: If the view has only been scrolled, it won't call invalidate(). So we just force it to repaint for now.
                        // We should keep a larger piece of rendered HTML in the texture and only scroll the texture if possible.
                        doDrawViewOnTexture();
                    }
                });
            }
        }
    }

    // Called from C++ to force the view
    public void invalidateOffscreenView()
    {
        Log.i(TAG, "invalidateOffscreenView");
        if (getView() != null)
        {
            final Activity context = getContextStatic();
            context.runOnUiThread(new Runnable()
            {
                @Override
                public void run()
                {
                    doInvalidateOffscreenView();
                }
            });
        }
    }

    //! Called from C++ to change size of the view.
    public void resizeOffscreenView(final int w, final int h)
    {
        synchronized(this)
        {
            Log.i(TAG, "resizeOffscreenView "+w+"x"+h);
            if (rendering_surface_ == null)
            {
                return;
            }
            rendering_surface_.setNewSize(w, h);
            View view = getView();
            if (view != null)
            {
                final Activity context = getContextStatic();
                context.runOnUiThread(new Runnable()
                {
                    @Override
                    public void run()
                    {
                        doResizeOffscreenView(w, h);
                        doInvalidateOffscreenView();
                    }
                });
            }
        }
    }

    public boolean isViewCreated()
    {
        return getView() != null;
    }

    // C++ function called from Java to tell that the texture has new contents.
    // abstract public native void nativeUpdate(long nativeptr);

    /*!
     * This is a class which keeps the rendering infrastructure.
     */
    protected class OffscreenRenderingSurface
    {
        long native_ptr_ = 0;
        int gl_texture_id_ = 0;
        int texture_width_ = 512;
        int texture_height_ = 512;
        SurfaceTexture surface_texture_ = null;
        Surface surface_ = null;
        View view_;
        String object_name_ = null;
        boolean has_texture_ = false;

        public OffscreenRenderingSurface(final long nativeptr, final String objectname, final View view, final int gltextureid, final int texwidth, final int texheight)
        {
            Log.i(TAG, "OffscreenRenderingSurface(obj=\""+objectname+"\", texture="+gltextureid
                +", w="+texwidth+", h="+texheight+") tid="+Thread.currentThread().getId());

            native_ptr_ = nativeptr;
            view_ = view;
            gl_texture_id_ = gltextureid;
            texture_width_ = texwidth;
            texture_height_ = texheight;
            object_name_ = objectname;

            surface_texture_ = new SurfaceTexture(gltextureid);
            surface_ = new Surface(surface_texture_);

            surface_texture_.setDefaultBufferSize(texwidth, texheight); // API 15
        }

        final int getTexture()
        {
            return gl_texture_id_;
        }

        final int getTextureWidth()
        {
            return texture_width_;
        }

        final int getTextureHeight()
        {
            return texture_height_;
        }

        final long getNativePtr()
        {
            return native_ptr_;
        }

        protected Canvas lockCanvas()
        {
            try
            {
                return surface_.lockCanvas(new Rect(0, 0, texture_width_-1, texture_height_-1));
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
                has_texture_ = true;
            }
            catch(Exception e)
            {
                Log.e(TAG, "Failed to unlock canvas", e);
            }
        }

        private float [] mtx_ = {
           0, 0, 0, 0,
           0, 0, 0, 0,
           0, 0, 0, 0,
           0, 0, 0, 0
        };

        protected boolean updateTexture()
        {
            Log.i(TAG, "updateTexture tid="+Thread.currentThread().getId()+", tex="+gl_texture_id_);
            try
            {
                if (!has_texture_)
                {
                    return false;
                }
                surface_texture_.updateTexImage();
                surface_texture_.getTransformMatrix(mtx_);
                return true;
            }
            catch(Exception e)
            {
                Log.e(TAG, "Failed to update texture", e);
                return false;
            }
        }

        public void cppDestroyed()
        {
            native_ptr_ = 0;
        }

        final public float getTextureTransformMatrix(final int index)
        {
            return mtx_[index];
        }

        final public boolean hasTexture()
        {
            return has_texture_;
        }

        public void setNewSize(int w, int h)
        {
            texture_width_ = w;
            texture_height_ = h;
            surface_texture_.setDefaultBufferSize(w, h); // API 15
        }

    }

}
