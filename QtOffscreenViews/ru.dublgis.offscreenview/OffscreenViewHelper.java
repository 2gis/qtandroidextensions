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
    public static final String TAG = "Grym/OffscreenView";

    long native_ptr_ = 0;
    int gl_texture_id_ = 0;
    int texture_width_ = 512;
    int texture_height_ = 512;
    SurfaceTexture surface_texture_ = null;
    Surface surface_ = null;
    View view_;
    String object_name_ = null;
    boolean has_texture_ = false;

    public OffscreenViewHelper(final long nativeptr, final String objectname, final View view, final int gltextureid, final int texwidth, final int texheight)
    {
        Log.i(TAG, "OffscreenViewHelper(obj=\""+objectname+"\", texture="+gltextureid
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

/*
    The steps to render your view to OpenGL:
    Initialize an OpenGL texture
    Within an OpenGL context construct a SurfaceTexture with the texture id. Use SurfaceTexture.setDefaultBufferSize(int width, int height) to make sure you have enough space on the texture for the view to render.
    Create a Surface constructed with the above SurfaceTexture.
    Within the View’s onDraw, use the Canvas returned by Surface.lockCanvas to do the view drawing. 
    You can obviously do this with any View, and not just WebView. Plus Canvas has a whole bunch of drawing methods, allowing you to do funky, funky things.
*/
}


