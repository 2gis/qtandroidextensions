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

import java.lang.Boolean;
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

/*!
 * A base class for all Android off-screen views which implements any common interfaces and functionality.
 */
abstract class OffscreenView
{
    public static final String TAG = "Grym/OffscreenView";
    protected OffscreenRenderingSurface rendering_surface_ = null;
    private String object_name_ = "UnnamedView";
    private long native_ptr_ = 0;
    private int gl_texture_id_ = 0;
    private Boolean painting_now_ = new Boolean(false);

    private int initial_width_ = 512;
    private int initial_height_ = 512;

    public void SetObjectName(String name) { object_name_ = name; }
    public void SetTexture(int tex) { gl_texture_id_ = tex; }
    public void SetInitialWidth(int w) { initial_width_ = w; }
    public void SetInitialHeight(int h) { initial_height_ = h; }
    public void SetNativePtr(long ptr) { native_ptr_ = ptr; }

    /*!
     * Invokes View creation in Android UI thread.
     */
    void createView()
    {
        Log.i(TAG, "OffscreenView.createView(name=\""+object_name_+"\")");
        final Activity context = getActivity();
        context.runOnUiThread(new Runnable()
        {
            @Override
            public void run()
            {
                synchronized(this)
                {
                    doCreateView();
                }
            }
        });
    }

    /*!
     * Invokes object initialization based on values passed via SetObjectName(), SetTexture(),
     * SetInitialWidth(), SetInitialHeight(), SetNativePtr().
     */
    void initializeGL()
    {
        final Activity context = getActivity();
        Log.i(TAG, "OffscreenView.intializeGL(name=\""+object_name_+"\", texture="+gl_texture_id_+")");
        context.runOnUiThread(new Runnable()
        {
            @Override
            public void run()
            {
                synchronized(this)
                {
                    rendering_surface_ = new OffscreenGLTextureRenderingSurface();
                    doResizeOffscreenView(initial_width_, initial_height_);
                    // Make sure the view will be repainted on the rendering surface, even it did
                    // finish its updates before the surface is available and its size didn't change
                    // and/or not triggered update by the resize call.
                    invalidateOffscreenView();
                }
            }
        });
    }

    abstract public View getView();
    abstract public void callViewPaintMethod(Canvas canvas);
    abstract public void doInvalidateOffscreenView();
    abstract public void doResizeOffscreenView(final int width, final int height);
    abstract public void doNativeUpdate();
    abstract public void doCreateView();
    abstract public Activity getActivity();

    //! Schedule painting of the view (will be done in Android UI thread).
    protected void drawViewOnTexture()
    {
        Activity ctx = getActivity();
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
            if (getNativePtr() == 0)
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
                    synchronized(painting_now_)
                    {
                        painting_now_ = true;
                    }

                    try
                    {
                        View v = getView();
                        if (v != null)
                        {
                            // Log.i(TAG, "doDrawViewOnTexture view size:"+v.getWidth()+"x"+v.getHeight());
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

                    synchronized(painting_now_)
                    {
                        painting_now_ = false;
                    }

                    t = System.nanoTime() - t;

                    // Tell C++ part that we have a new image
                    doNativeUpdate();

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
    public boolean updateTexture(boolean synced)
    {
        if (synced)
        {
            synchronized(this)
            {
                return updateTexture(false);
            }
        }
        if (rendering_surface_ == null)
        {
            return false;
        }
        synchronized(painting_now_)
        {
            if (painting_now_)
            {
                return false;
            }
        }
        return rendering_surface_.updateTexture();
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
        if (getNativePtr() == 0)
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
            Activity ctx = getActivity();
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
            final Activity context = getActivity();
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
                final Activity context = getActivity();
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

    final long getNativePtr()
    {
        return native_ptr_;
    }

    //! Called from C++ to notify us that the associated C++ object is being destroyed.
    public void cppDestroyed()
    {
        synchronized(this)
        {
            native_ptr_ = 0;
        }
    }

    // C++ function called from Java to tell that the texture has new contents.
    // abstract public native void nativeUpdate(long nativeptr);

    //! Imagine we'll have another type of rendering surface one day (e.g. Bitmap)
    protected interface OffscreenRenderingSurface
    {
        abstract Canvas lockCanvas();
        abstract void unlockCanvas(Canvas canvas);
        abstract boolean updateTexture();
        //! In non-GL mode, this should not be used and can return 0 for any index.
        abstract float getTextureTransformMatrix(final int index);
        abstract public boolean hasTexture();
        abstract public void setNewSize(int w, int h);
    }

    /*!
     * This is a class which keeps the rendering infrastructure.
     */
    protected class OffscreenGLTextureRenderingSurface implements OffscreenRenderingSurface
    {
        int texture_width_ = 512;
        int texture_height_ = 512;
        SurfaceTexture surface_texture_ = null;
        Surface surface_ = null;
        boolean has_texture_ = false;

        public OffscreenGLTextureRenderingSurface()
        {
            Log.i(TAG, "OffscreenGLTextureRenderingSurface(obj=\""+object_name_+"\", texture="+gl_texture_id_
                +", w="+initial_width_+", h="+initial_height_+") tid="+Thread.currentThread().getId());
            surface_texture_ = new SurfaceTexture(gl_texture_id_);
            surface_ = new Surface(surface_texture_);
            setNewSize(initial_width_, initial_height_);
        }

        @Override
        public Canvas lockCanvas()
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

        @Override
        public void unlockCanvas(Canvas canvas)
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

        // Texture coordinate transformation matrix. The transformation is typically simply a flip of Y axis.
        // And, of course, it should never change Z. So it could be a 2D matrix + shift vector as well.
        // However, Android returns a 4D uniform matrix so it could be passed to GL functions directly.
        // We optimize that on C++/JNI side by reading only relevant values.
        private float [] mtx_ = {
           0, 0, 0, 0,
           0, 0, 0, 0,
           0, 0, 0, 0,
           0, 0, 0, 0
        };

        @Override
        public boolean updateTexture()
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

        @Override
        final public float getTextureTransformMatrix(final int index)
        {
            return mtx_[index];
        }

        @Override
        final public boolean hasTexture()
        {
            return has_texture_;
        }

        @Override
        public void setNewSize(int w, int h)
        {
            texture_width_ = w;
            texture_height_ = h;
            surface_texture_.setDefaultBufferSize(w, h); // API 15
        }

    }

}
