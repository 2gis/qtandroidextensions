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
import java.util.ArrayList;
import java.util.Iterator;
import java.util.TreeSet;
import java.util.Locale;
import java.util.List;
import java.util.LinkedList;
import java.util.Timer;
import java.util.TimerTask;
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
import android.graphics.SurfaceTexture;
import android.os.Bundle;
import android.os.Build;
import android.os.Environment;
import android.os.Handler;
import android.os.IBinder;
import android.os.Looper;
import android.os.SystemClock;
import android.text.method.MetaKeyKeyListener;
import android.text.InputType;
import android.util.Log;
import android.util.DisplayMetrics;
import android.view.ContextMenu;
import android.view.ContextMenu.ContextMenuInfo;
import android.view.View;
import android.view.ViewGroup;
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
//import android.widget.FrameLayout;
import android.graphics.Canvas;

/*!
 * A base class for all Android off-screen views which implements any common interfaces and functionality.
 */
abstract class OffscreenView
{
    public static final String TAG = "Grym/OffscreenView";
    private View view_ = null;
    protected OffscreenRenderingSurface rendering_surface_ = null;
    private String object_name_ = "UnnamedView";
    private long native_ptr_ = 0;
    private int gl_texture_id_ = 0;
    private ArrayList<Runnable> precreation_actions_ = new ArrayList<Runnable>();

    private Object native_ptr_mutex_ = new Object();
    private Object view_existence_mutex_ = new Object();
    private Object view_variables_mutex_ = new Object();
    private Object texture_mutex_ = new Object();
    private Object texture_transform_mutex_ = new Object();

    private int view_width_ = 512;
    private int view_height_ = 512;
    private int view_left_ = 0;
    private int view_top_ = 0;
    protected int fill_a_ = 255, fill_r_ = 255, fill_g_ = 255, fill_b_ = 255;
    private MyLayout layout_ = null;
    private boolean last_visibility_ = false;
    private boolean last_enabled_ = true;
    private boolean offscreen_touch_ = false;
    private boolean is_attached_ = false;
    private boolean attaching_mode_ = true;
    private boolean hide_keyboard_on_focus_loss_ = true;
    private boolean show_keyboard_on_focus_in_ = false;
    private int scroll_x_ = 0;
    private int scroll_y_ = 0;
    private int measured_width_ = -1;
    private int measured_height_ = -1;

    // Variables to inform C++ about last painted texture / control size
    private int last_painted_width_ = 0;
    private int last_painted_height_ = 0;
    private int last_texture_width_ = 0;
    private int last_texture_height_ = 0;

    //! Simple one-element absolute layout.
    private class MyLayout extends ViewGroup
    {
        public MyLayout(Activity a)
        {
            super(a);
        }

        @Override
        protected void onDraw(Canvas canvas)
        {
            // We can actually do totally nothing.
        }

        @Override
        protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec)
        {
            // Log.i(TAG, "onMeasure ws="+widthMeasureSpec+", hs="+heightMeasureSpec);
            if (getChildCount() != 1)
            {
                throw new IllegalStateException("OffscreenView layout should have 1 child!");
            }
            super.onMeasure(widthMeasureSpec, heightMeasureSpec);

            View child = getChildAt(0);
            measureChild(child, widthMeasureSpec, heightMeasureSpec);
            synchronized(view_variables_mutex_)
            {
                measured_width_ = child.getMeasuredWidth();
                measured_height_ = child.getMeasuredHeight();
            }
        }

        @Override
        protected void onLayout(boolean changed, int left, int top, int right, int bottom)
        {
            // Log.i(TAG, "onLayout changed="+changed+", l="+left+", t="+top+", r="+right+", b="+bottom+
            //     ", view l="+view_left_+", t="+view_top_+", sz="+view_width_+"x"+view_height_);
            if (getChildCount() != 1)
            {
                throw new IllegalStateException("OffscreenView layout should have 1 child!");
            }
            View child = getChildAt(0);
            // This absolutely must be done, because this is what updates layout of overlays
            // on some Android versions.
            child.layout(view_left_, view_top_, view_left_+view_width_, view_top_+view_height_);
            if (getApiLevel() >= 11)
            {
                // Child view should not overlap with the keyboard
                child.layout(0, 0, view_width_, view_height_);
                // Translate to real position
                child.setX(view_left_);
                child.setY(view_top_);
            }
        }
    }

    OffscreenView()
    {
        Log.i(TAG, "OffscreenView constructor");
    }

    public void SetObjectName(String name)
    {
        synchronized(view_variables_mutex_)
        {
            object_name_ = name;
        }
    }

    public void SetTexture(int tex)
    {
        synchronized(texture_mutex_)
        {
            if (gl_texture_id_ != 0 && gl_texture_id_ != tex)
            {
                throw new IllegalStateException("OpenGL texture can be assigned only once!");
            }
            gl_texture_id_ = tex;
        }
    }

    public void SetInitialWidth(int w)
    {
        synchronized(view_variables_mutex_)
        {
            view_width_ = w;
        }
    }

    public void SetInitialHeight(int h)
    {
        synchronized(view_variables_mutex_)
        {
            view_height_ = h;
        }
    }

    public void SetNativePtr(long ptr)
    {
        synchronized(native_ptr_mutex_)
        {
            native_ptr_ = ptr;
        }
    }

    public boolean runOnUiThread(final Runnable runnable)
    {
        try
        {
            if (runnable == null)
            {
                Log.e(TAG, "OffscreenView.runOnUiThread: null runnable!");
                return false;
            }
            final Activity context = getActivity();
            if (context == null)
            {
                Log.e(TAG, "OffscreenView.runOnUiThread: cannot schedule task because of the null context!");
                return false;
            }
            // Log.i(TAG, "OffscreenView.runOnUiThread: scheduling runnable...");
            context.runOnUiThread(runnable);
            return true;
        }
        catch (Exception e)
        {
            Log.e(TAG, "Exception when posting a runnable:", e);
            return false;
        }
    }

    public boolean runViewAction(final Runnable runnable)
    {
        synchronized(view_existence_mutex_)
        {
            if (getView() == null)
            {
                Log.i(TAG, "runViewAction: scheduling action "+(precreation_actions_.size()+1)+" for future execution...");
                precreation_actions_.add(runnable);
                return false;
            }
            else
            {
                return runOnUiThread(runnable);
            }
        }
    }

    final public View getView()
    {
        synchronized(view_existence_mutex_)
        {
            return view_;
        }
    }

    final protected void setView(View v)
    {
        synchronized(view_existence_mutex_)
        {
            if (view_ != null)
            {
                throw new IllegalStateException("OffscreenView View can be set only once!");
            }
            view_ = v;
        }
    }

    /*!
     * Invokes View creation in Android UI thread.
     */
    public boolean createView()
    {
        Log.i(TAG, "OffscreenView.createView(name=\""+object_name_+"\") called");
        boolean result = runOnUiThread(new Runnable() {
            @Override
            public void run()
            {
                Log.i(TAG, "OffscreenView.createView: run/syncing...");
                synchronized(view_variables_mutex_) // Using these variables
                {
                    Log.i(TAG, "OffscreenView.createView: creating the view!");
                    final Activity activity = getActivity();

                    // Call final widget implementation function to handle actual
                    // construction of the view.
                    synchronized(view_existence_mutex_)
                    {
                        doCreateView();
                    }

                    final View view = getView();

                    // Set initial view properties
                    // View becomes focusable only when Qt requests that. It should not
                    // be focused from Android side.
                    view.setFocusable(false);
                    view.setFocusableInTouchMode(false);
                    view.setVisibility(last_visibility_? View.VISIBLE: View.INVISIBLE);
                    if (getApiLevel() >= 11)
                    {
                        view.setLeft(0);
                        view.setTop(0);
                        view.setRight(view_width_);
                        view.setBottom(view_height_);
                    }

                    // Insert the View into layout.
                    // Note: functions of many views will crash if they are not inserted into layout.
                    layout_ = new MyLayout(activity);
                    if (getApiLevel() >= 11)
                    {
                        layout_.setRight(view_width_);
                        layout_.setBottom(view_height_);
                    }
                    layout_.addView(view);
                    uiAttachViewToQtScreen();

                    // Process command queue
                    synchronized(view_existence_mutex_)
                    {
                        Log.i(TAG, "createView: processing "+(precreation_actions_.size()+1)+" actions...");
                        Iterator<Runnable> it = precreation_actions_.iterator();
                        int i = 0;
                        while(it.hasNext())
                        {
                            Log.i(TAG, "createView: processing action #"+i);
                            //runOnUiThread(it.next());
                            it.next().run();
                            i++;
                        }
                        precreation_actions_.clear();
                    }

                    // Notify C++ part that the view construction has been completed.
                    nativeViewCreated(getNativePtr());
                }
            }
        });
        Log.i(TAG, "createView result="+result);
        return result;
    }

    private ViewGroup getMainLayout()
    {
        final Activity activity = getActivity();
        if (activity == null)
        {
            Log.e(TAG, "Failed find main layout because the activity is null!");
            return null;
        }
        ViewGroup vg = (ViewGroup)activity.findViewById(android.R.id.content);
        if (vg == null)
        {
            Log.e(TAG, "findViewById failed to find content!");
        }
        return vg;
    }

    private boolean uiAttachViewToQtScreen()
    {
        try
        {
            if (!attaching_mode_ || is_attached_)
            {
                return false;
            }
            Log.i(TAG, "uiAttachViewToQtScreen "+object_name_);
            if (layout_ == null)
            {
                Log.e(TAG, "Failed to insert "+object_name_+" into the ViewGroup because View is null!");
                return false;
            }
            ViewGroup vg = getMainLayout();
            if (vg != null)
            {
                Log.i(TAG, "Inserting "+object_name_+" (layout_ id="+layout_.getId()+") into the ViewGroup...");
                vg.addView(layout_);
                is_attached_ = true;
                return true;
            }
            else
            {
                Log.w(TAG, "Failed to insert "+object_name_+" into the ViewGroup because it was not found!");
                return false;
            }
        }
        catch(Exception e)
        {
            Log.e(TAG, "Exception in uiAttachViewToQtScreen:", e);
            return false;
        }
    }

    /*!
     * \note This function doesn't check attaching_mode_/is_attached_.
     */
    private boolean uiDetachViewFromQtScreen()
    {
        Log.i(TAG, "uiDetachViewFromQtScreen "+object_name_);
        uiHideKeyboardFromView();
        try
        {
            Activity activity = getActivity();
            View view = getView();
            if (activity == null || layout_ == null || view == null)
            {
                Log.w(TAG, "Could not remove "+object_name_+" from the ViewGroup because Activity, layout or view is null.");
                return false;
            }
            // Remove layout_ from its previous known parent.
            ViewGroup parent = (ViewGroup)layout_.getParent();
            if (parent != null)
            {
                Log.i(TAG, "Removing "+object_name_+" (layout_ id="+layout_.getId()+") from its parent...");
                parent.removeView(layout_);
                is_attached_ = false;
            }
            // This step is probably not necessary, but it's not harmful either.
            ViewGroup vg = (ViewGroup)activity.findViewById(android.R.id.content);
            if (vg != null)
            {
                Log.i(TAG, "Removing "+object_name_+" (layout_ id="+layout_.getId()+") from the ViewGroup...");
                if (view.isFocused())
                {
                    int count = vg.getChildCount();
                    for (int i = 0; i < count; i++) {
                       final View child = vg.getChildAt(i);
                       if (child != (View)layout_ && child.getVisibility() == View.VISIBLE && child.isEnabled() && child.isFocusable())
                       {
                           // Avoid trying to pass focus to another OffscreenView
                           try {
                               if ((MyLayout)child != null)
                               {
                                   continue;
                               }
                           } catch(Exception e) {} // ClassCastException
                           child.requestFocus();
                           Log.i(TAG, "Successfully passed focus from "+object_name_);
                           break;
                       }
                    }
                }
                vg.removeView(layout_);
                is_attached_ = false;
                return true;
            }
            else
            {
                Log.w(TAG, "Failed to remove "+object_name_+" from the ViewGroup because it was not found.");
                return false;
            }
        }
        catch(Exception e)
        {
            Log.e(TAG, "Exception in uiDetachViewFromQtScreen:", e);
            return false;
        }
    }

    final public boolean isInAttachingMode()
    {
        return attaching_mode_;
    }

    //! Called from C++ to control attaching mode.
    public void setAttachingMode(final boolean attaching)
    {
        if (attaching_mode_ != attaching)
        {
            attaching_mode_ = attaching;
            runOnUiThread(new Runnable(){
                @Override
                public void run()
                {
                    if (getView() != null)
                    {
                        if (attaching && !is_attached_)
                        {
                            uiAttachViewToQtScreen();
                        }
                        else if (!attaching && is_attached_)
                        {
                            uiDetachViewFromQtScreen();
                        }
                    }
                }
            });
        }
    }

/*    //! This function detaches view and attaches it back. Can be used for certain workarounds.
    public void reattachView()
    {
        runOnUiThread(new Runnable(){
            @Override
            public void run()
            {
                View v = getView();
                if (v != null && attaching_mode_)
                {
                    Log.i(TAG, "reattachView "+object_name_);
                    uiDetachViewFromQtScreen();
                    uiAttachViewToQtScreen();
                }
            }
        });
    } */

    /*!
     * Invokes object initialization based on values passed via SetObjectName(), SetTexture(),
     * SetInitialWidth(), SetInitialHeight(), SetNativePtr().
     */
    void initializeGL()
    {
        Log.i(TAG, "OffscreenView.intializeGL(name=\""+object_name_+"\", texture="+gl_texture_id_+")");
        runViewAction(new Runnable() {
            @Override
            public void run()
            {
                synchronized(texture_mutex_)
                {
                    Log.i(TAG, "OffscreenView.intializeGL(name=\""+object_name_+"\", texture="+gl_texture_id_+") RUN");
                    rendering_surface_ = new OffscreenGLTextureRenderingSurface();
                    if (layout_ != null)
                    {
                        runViewAction(new Runnable(){
                                @Override
                                public void run()
                                {
                                    layout_.requestLayout();
                                }
                        });
                    }
                    // Make sure the view will be repainted on the rendering surface, even it did
                    // finish its updates before the surface is available and its size didn't change
                    // and/or not triggered update by the resize call.
                    invalidateOffscreenView();
                }
            }
        });
    }

    void initializeBitmap(final Bitmap bitmap_a, final Bitmap bitmap_b)
    {
        Log.i(TAG, "OffscreenView.intializeBitmap(name=\""+object_name_+"\"");
        synchronized(texture_mutex_)
        {
            rendering_surface_ = new OffscreenBitmapRenderingSurface();
            rendering_surface_.setBitmaps(bitmap_a, bitmap_b);
        }
        runViewAction(new Runnable() {
            @Override
            public void run()
            {
                synchronized(texture_mutex_)
                {
                    Log.i(TAG, "OffscreenView.intializeBitmap(name=\""+object_name_+"\") RUN");
                    if (layout_ != null)
                    {
                        runViewAction(new Runnable(){
                                @Override
                                public void run()
                                {
                                    layout_.requestLayout();
                                }
                        });
                    }
                    invalidateOffscreenView();
                }
            }
        });
    }

    abstract public void callViewPaintMethod(Canvas canvas);
    abstract public void doCreateView();

    //! Note: all views are hidden by default (after creation).
    public boolean isVisible()
    {
        return last_visibility_;
    }

    //! Note: all views are hidden by default.
    public void setVisible(final boolean visible)
    {
        last_visibility_ = visible;
        runViewAction(new Runnable(){
            @Override
            public void run()
            {
                View v = getView();
                if (v != null)
                {
                    int vis = last_visibility_? View.VISIBLE: View.INVISIBLE;
                    if (vis != v.getVisibility())
                    {
                        if (!visible)
                        {
                            uiDetachViewFromQtScreen();
                        }
                        v.setVisibility(vis);
                        if (visible)
                        {
                            if (attaching_mode_)
                            {
                                uiAttachViewToQtScreen();
                            }
                            invalidateOffscreenView();
                        }
                    }
                    else
                    {
                        Log.i(TAG, "setVisible: skipping double setting to "+visible+" for "+object_name_);
                    }
                }
            }
        });
    }

    public boolean isEnabled()
    {
        return last_enabled_;
    }

    public void setEnabled(final boolean enabled)
    {
        last_enabled_ = enabled;
        runViewAction(new Runnable(){
            @Override
            public void run()
            {
                View v = getView();
                if (v != null)
                {
                    if (!enabled)
                    {
                        uiHideKeyboardFromView();
                    }
                    v.setEnabled(enabled);
                }
            }
        });
    }

    private int last_texture_invalidation_ = 0;
    private boolean invalidated_ = true;

    //! Schedules doDrawViewOnTexture() with filtering out extra calls.
    protected void invalidateOffscreenView()
    {
        // Log.i(TAG, "invalidateOffscreenView "+object_name_+", last := "+last_texture_invalidation_);
        runOnUiThread(new Runnable(){
            @Override
            public void run()
            {
                last_texture_invalidation_ = (last_texture_invalidation_ >= 2000000000)? 0: last_texture_invalidation_ + 1;
                invalidated_ = true;
                new Handler().post(new Runnable(){
                    private final int invalidation_ = last_texture_invalidation_;
                    @Override
                    public void run()
                    {
                        // Log.i(TAG, "invalidateOffscreenView "+object_name_+" RUNNABLE");

                        // 1. We always have to draw last queued paint event to have the View painted to its latest state
                        //    (which may be changed by some events scheduled between paints).
                        // 2. We have to draw first event in the queue because otherwise fast input event generation
                        //    may defer any painting for too long. For example, when user quickly types on soft keyboard
                        //    the editor will not be repainted at all until all scheduled key presses are processed,
                        //    which may easily take up a second.
                        // Unfortunately, this approach sometimes causes unnecessary double paints, but that seems to be
                        // a lesser evil available.
                        if (invalidation_ >= last_texture_invalidation_ || invalidated_)
                        {
                            //Log.i(TAG, "invalidateOffscreenView "+object_name_+" RUNNABLE: inval="+invalidation_+", last="+last_texture_invalidation_+
                             //  ", invalidated="+invalidated_);
                            invalidated_ = false;
                            boolean drawn = doDrawViewOnTexture();
                            if (!drawn)
                            {
                                // Log.i(TAG, "invalidateOffscreenView: "+object_name_+" Failed to draw the View.");
                                invalidated_ = true;
                            }
                        }
                        else
                        {
                            //Log.i(TAG, "invalidateOffscreenView "+object_name_+" RUNNABLE SKIPPED: inval="+invalidation_+", last="+last_texture_invalidation_+
                            //   ", invalidated="+invalidated_);
                        }
                    }
                });
            }
        });
    }

    //! Performs actual painting of the view. Should be called in Android UI thread.
    protected boolean doDrawViewOnTexture()
    {
        boolean result = false;
        synchronized(texture_mutex_)
        {
            // Skip painting in certain conditions.
            // Note: with a null View we will continue, but will only fill
            // background with the fill color.

            // Normal situation (rendering surface is not created yet, this is OK as we do all asynchronously).
            if (rendering_surface_ == null)
            {
                // Log.i(TAG, "doDrawViewOnTexture: surface is null: "+object_name_);
                return false;
            }

            // C++ part is not set or already lost (may happen during init/deinit).
            if (getNativePtr() == 0)
            {
                Log.i(TAG, "doDrawViewOnTexture: native ptr is null: "+object_name_);
                return false;
            }

            // Thou shalt not paint while app is miminized, or GL may freak out and the app will hang
            // with black screen.
            if (!last_visibility_)
            {
                Log.i(TAG, "doDrawViewOnTexture: skipping paint for invisible view: "+object_name_);
                return false;
            }

            View v = getView();
            if (v != null && v.getVisibility() != View.VISIBLE)
            {
                // Note: setVisible()'s lambda will schedule one more paint after the view will become visible.
                Log.i(TAG, "doDrawViewOnTexture: skipping paint because view visibility did not apply yet: "+object_name_);
                return false;
            }

            try
            {
                // long t = System.nanoTime();
                Canvas canvas = rendering_surface_.lockCanvas();
                if (canvas == null)
                {
                    Log.e(TAG, "doDrawViewOnTexture: failed to lock canvas for: "+object_name_);
                }
                else
                {
                    try
                    {
                        if (v != null)
                        {
                            // Log.i(TAG, "doDrawViewOnTexture view size:"+v.getWidth()+"x"+v.getHeight());

                            // Prepare canvas.
                            // Take View scroll into account.
                            synchronized(view_variables_mutex_)
                            {
                                scroll_x_ = v.getScrollX();
                                scroll_y_ = v.getScrollY();
                            }
                            canvas.translate(-scroll_x_, -scroll_y_);

                            callViewPaintMethod(canvas);

                            synchronized(texture_transform_mutex_)
                            {
                                last_painted_width_ = v.getWidth();
                                last_painted_height_ = v.getHeight();
                            }

                            result = true;
                        }
                        else
                        {
                            synchronized(view_variables_mutex_)
                            {
                                canvas.drawARGB(fill_a_, fill_r_, fill_g_, fill_b_);
                            }
                        }
                    }
                    catch(Exception e)
                    {
                        Log.e(TAG, "doDrawViewOnTexture painting failed!", e);
                    }

                    rendering_surface_.unlockCanvas(canvas);
                    // t = System.nanoTime() - t;
                    // Tell C++ part that we have a new image
                    nativeUpdate(getNativePtr());

                    // Log.i(TAG, "doDrawViewOnTexture: success, t="+t/1000000.0+"ms");
                }
            }
            catch(Exception e)
            {
                Log.e(TAG, "doDrawViewOnTexture exception:", e);
            }
        }
        return result;
    }

    //! Called from C++ to get current texture.
    public boolean updateTexture()
    {
        // If Android UI thread is drawing the View now, the sync will pause execution it finishes.
        synchronized(texture_mutex_)
        {
            if (rendering_surface_ == null)
            {
                return false;
            }
            return rendering_surface_.updateTexture();
        }
    }


    /*! Called from C++ to get texture coordinate transformation matrix (filled in updateTexture()).
        This function should be called after updateTexture(). */
    public float getTextureTransformMatrix(int index)
    {
        if (rendering_surface_ == null)
        {
            return 0;
        }
        return rendering_surface_.getTextureTransformMatrix(index);
    }

    //! Called from C++.
    public int getLastTextureWidth()
    {
        synchronized(texture_transform_mutex_)
        {
            return last_texture_width_;
        }
    }

    //! Called from C++.
    public int getLastTextureHeight()
    {
        synchronized(texture_transform_mutex_)
        {
            return last_texture_height_;
        }
    }

    //! Called from C++
    public void setBitmaps(final Bitmap bitmap_a, final Bitmap bitmap_b)
    {
        if (rendering_surface_ != null)
        {
            rendering_surface_.setBitmaps(bitmap_a, bitmap_b);
        }
        System.gc();
    }

    //! Called from C++
    public int getQtPaintingTexture()
    {
        if (rendering_surface_ != null)
        {
            // Log.i(TAG, "getQtPaintingTexture "+object_name_+" getting texture...");
            return rendering_surface_.getQtPaintingTexture();
        }
        else
        {
            // Log.i(TAG, "getQtPaintingTexture "+object_name_+" no rendering surface, returning -1");
            return -1;
        }
    }

    /*!
     * This function should be called in view's onTouchEvent to check if the touch should be processed.
     */
    final public boolean isOffscreenTouch()
    {
        return offscreen_touch_;
    }

    private long mouse_time_of_press_ = 0;

    public void ProcessMouseEvent(final int action, final int x, final int y, final long timestamp)
    {
        if (getNativePtr() == 0)
        {
            Log.i(TAG, "ProcessMouseEvent: zero native ptr, ignoring.");
            return;
        }
        final View view = getView();
        if (view != null)
        {
            final long t = (timestamp == 0)? SystemClock.uptimeMillis(): timestamp;
            // Log.i(TAG, "MILLIS="+SystemClock.uptimeMillis()+" TS="+timestamp);
            if (action == MotionEvent.ACTION_DOWN || mouse_time_of_press_ == 0)
            {
                mouse_time_of_press_ = t;
            }
            // Log.i(TAG, "ProcessMouseEvent("+action+", "+x+", "+y+") time of press = "+mouse_time_of_press_+", t="+t);
            final MotionEvent event = MotionEvent.obtain(mouse_time_of_press_ /* downTime*/, t /* eventTime */, action, x, y, 0 /*metaState*/);
            runOnUiThread(new Runnable() {
                @Override
                public void run()
                {
                    offscreen_touch_ = true;
                    view.onTouchEvent(event);
                    offscreen_touch_ = false;
                    if (!attaching_mode_)
                    {
                        // If the view has only been scrolled, it won't call invalidate(). So we just force it to repaint for now.
                        doDrawViewOnTexture();
                    }
                }
            });
        }
    }

    /*//! Called from C++
    public boolean isFocused()
    {
        final View v = getView();
        if (v != null)
        {
            return v.isFocused();
        }
        return false;
    }*/

    //! Called from C++
    public void setFocused(final boolean focused)
    {
        runViewAction(new Runnable() {
            @Override
            public void run()
            {
                Log.i(TAG, "setFocused("+focused+") "+object_name_+": run");
                final View v = getView();
                if (v != null)
                {
                    if (focused)
                    {
                        v.setFocusable(true);
                        v.setFocusableInTouchMode(true);
                        v.requestFocus();
                        if (show_keyboard_on_focus_in_)
                        {
                            uiShowKeyboard();
                        }
                    }
                    else
                    {
                        boolean was_focused = v.isFocused();
                        v.setFocusable(false);
                        v.setFocusableInTouchMode(false);
                        v.clearFocus();
                        if (hide_keyboard_on_focus_loss_)
                        {
                            uiHideKeyboardFromView();
                        }
                    }
                    invalidateOffscreenView();
                }
            }
        });
    }

    /*!
     * We need to workaround a problem: when View is defocused it doesn't hide SIP (weird but no ideas how to fix it another way).
     */
    protected void uiHideKeyboardFromView()
    {
        try
        {
            final View v = getView();
            if (v == null)
            {
                Log.e(TAG, "uiHideKeyboardFromView: View is null");
                return;
            }
            InputMethodManager imm = (InputMethodManager)v.getContext().getSystemService(Context.INPUT_METHOD_SERVICE);
            if (imm == null)
            {
                Log.w(TAG, "uiHideKeyboardFromView: InputMethodManager is null");
                return;
            }
            IBinder token = v.getWindowToken();
            if( token != null )
            {
                imm.hideSoftInputFromWindow(token, 0);
            }
            else
            {
                Log.i(TAG, "uiHideKeyboardFromView: Window token is null");
            }
        }
        catch (Exception e)
        {
            Log.e(TAG, "uiHideKeyboardFromView: exception:", e);
        }
    }

    // Called from C++ to hide keyboard
    public void hideKeyboard()
    {
        runOnUiThread(new Runnable(){
            @Override
            public void run()
            {
                uiHideKeyboardFromView();
            }
        });
    }

    protected void uiShowKeyboard()
    {
        try
        {
            final View v = getView();
            if (v == null)
            {
                Log.e(TAG, " uiShowKeyboard: View is null");
                return;
            }
            InputMethodManager imm = (InputMethodManager)v.getContext().getSystemService(Context.INPUT_METHOD_SERVICE);
            if (imm == null)
            {
                Log.w(TAG, " uiShowKeyboard: InputMethodManager is null");
                return;
            }
            imm.showSoftInput(v, InputMethodManager.SHOW_IMPLICIT);
        }
        catch (Exception e)
        {
            Log.e(TAG, " uiShowKeyboard: exception:", e);
        }
    }

    // Called from C++ to show keyboard
    public void showKeyboard()
    {
        runOnUiThread(new Runnable(){
            @Override
            public void run()
            {
                uiShowKeyboard();
            }
        });
    }

    // Called from C++ to set hide_keyboard_on_focus_loss_ flag
    public void setHideKeyboardOnFocusLoss(boolean hide)
    {
        hide_keyboard_on_focus_loss_ = hide;
    }

    // Called from C++ to set hide_keyboard_on_focus_loss_ flag
    public void setShowKeyboardOnFocusIn(boolean show)
    {
        show_keyboard_on_focus_in_ = show;
    }

    //! Called from C++ to change size of the view.
    public void resizeOffscreenView(final int w, final int h)
    {
        synchronized(view_variables_mutex_)
        {
            Log.i(TAG, "resizeOffscreenView "+w+"x"+h);
            view_width_ = w;
            view_height_ = h;
            if (rendering_surface_ != null)
            {
                rendering_surface_.setNewSize(w, h);
            }
            runViewAction(new Runnable(){
                @Override
                public void run()
                {
                    final View v = getView();
                    if (v != null)
                    {
                        if (!attaching_mode_)
                        {
                            if (getApiLevel() >= 11)
                            {
                                v.setLeft(0);
                                v.setTop(0);
                                v.setRight(w);
                                v.setBottom(h);
                            }
                        }
                        else
                        {
                            v.forceLayout();
                            v.requestLayout();
                        }
                        invalidateOffscreenView();
                    }
                }
            });
        }
    }

    /*!
     * Called from C++ to inform Android about real position of the view.
     * This is necessary for text editors as it affects position of text selection markers.
     */
    public void setPosition(final int left, final int top)
    {
        synchronized(view_variables_mutex_)
        {
            if (view_left_ != left || view_top_ != top)
            {
                view_left_ = left;
                view_top_ = top;
                if (layout_ != null)
                {
                    runViewAction(new Runnable(){
                            @Override
                            public void run()
                            {
                                layout_.requestLayout();
                            }
                    });
                }
            }
        }
    }

    public boolean isViewCreated()
    {
        return getView() != null;
    }

    final long getNativePtr()
    {
        synchronized(native_ptr_mutex_)
        {
            return native_ptr_;
        }
    }

    //! Called from C++ to notify us that the associated C++ object is being destroyed.
    public void cppDestroyed()
    {
        synchronized(native_ptr_mutex_)
        {
            native_ptr_ = 0;
        }
        runOnUiThread(new Runnable(){
            @Override
            public void run()
            {
                if (is_attached_)
                {
                    uiDetachViewFromQtScreen();
                }
            }
        });
    }

    public void setFillColor(int a, int r, int g, int b)
    {
        synchronized(view_variables_mutex_)
        {
            fill_a_ = a;
            fill_r_ = r;
            fill_g_ = g;
            fill_b_ = b;
        }
    }

    public int getScrollX()
    {
        synchronized(view_variables_mutex_)
        {
            return scroll_x_;
        }
    }

    public int getScrollY()
    {
        synchronized(view_variables_mutex_)
        {
            return scroll_y_;
        }
    }

    public int getMeasuredWidth()
    {
        synchronized(view_variables_mutex_)
        {
            return measured_width_;
        }
    }

    public int getMeasuredHeight()
    {
        synchronized(view_variables_mutex_)
        {
            return measured_height_;
        }
    }
    // C++ function called from Java to tell that the texture has new contents.
    // abstract public native void nativeUpdate(long nativeptr);

    protected interface OffscreenRenderingSurface
    {
        abstract Canvas lockCanvas();
        abstract void unlockCanvas(Canvas canvas);

        //
        // OpenGL texture mode
        //
        abstract boolean updateTexture();
        abstract float getTextureTransformMatrix(final int index);
        abstract public boolean hasTexture();
        abstract public void setNewSize(final int w, final int h);

        //
        // Bitmap mode
        //
        abstract public void setBitmaps(final Bitmap bitmap_a, final Bitmap bitmap_b);
        abstract public int getQtPaintingTexture();
    }

    protected class OffscreenBitmapRenderingSurface implements OffscreenRenderingSurface
    {
        Bitmap bitmap_a_ = null;
        Bitmap bitmap_b_ = null;
        int draw_bitmap_ = 0;
        boolean has_texture_ = false;
        int last_drawn_bitmap_ = -1;

        public OffscreenBitmapRenderingSurface()
        {
        }

        @Override
        public Canvas lockCanvas()
        {
            synchronized(texture_mutex_)
            {
                if (bitmap_a_ == null || bitmap_b_ == null)
                {
                    return null;
                }
                Bitmap bitmap = (draw_bitmap_ == 0)? bitmap_a_: bitmap_b_;
                // Log.i(TAG, "lockCanvas: locking "+object_name_+" texture="+draw_bitmap_);
                last_drawn_bitmap_ = draw_bitmap_;
                return new Canvas(bitmap);
            }
        }

        @Override
        public void unlockCanvas(Canvas canvas)
        {
            synchronized(texture_mutex_)
            {
                // Marking that we have a painted texture
                has_texture_ = true;
                synchronized(texture_transform_mutex_)
                {
                    last_texture_width_ = last_painted_width_;
                    last_texture_height_ = last_painted_height_;
                }
            }
        }

        @Override
        public int getQtPaintingTexture()
        {
            synchronized(texture_mutex_)
            {
                if (!hasTexture())
                {
                    // Log.i(TAG, "getQtPaintingTexture "+object_name_+" -1!");
                    return -1;
                }
                // Swapping buffers, so Android won't paint on the Bitmap
                // which is currently being used by Qt.
                if (draw_bitmap_ != last_drawn_bitmap_)
                {
                    // Log.i(TAG, "getQtPaintingTexture "+object_name_+" -1, new bitmap is not ready yet!");
                    return -1;
                }
                int old_draw_bitmap = draw_bitmap_;
                draw_bitmap_ = (draw_bitmap_ == 0)? 1: 0;
                // Log.i(TAG, "getQtPaintingTexture: "+object_name_+" returning texture="+old_draw_bitmap);
                return old_draw_bitmap;
            }
        }

        @Override
        public boolean updateTexture()
        {
            return false;
        }

        @Override
        public float getTextureTransformMatrix(final int index)
        {
            return 0;
        }

        @Override
        public boolean hasTexture()
        {
            return has_texture_ && bitmap_a_ != null && bitmap_b_ != null;
        }

        @Override
        public void setNewSize(final int w, final int h)
        {
        }

        @Override
        public void setBitmaps(final Bitmap bitmap_a, final Bitmap bitmap_b)
        {
            synchronized(texture_mutex_)
            {
                // Log.i(TAG, "setBitmaps "+object_name_);
                if (bitmap_a_ != bitmap_a || bitmap_b_ != bitmap_b)
                {
                    bitmap_a_ = bitmap_a;
                    bitmap_b_ = bitmap_b;
                    draw_bitmap_ = 0;
                    has_texture_ = false;
                    last_drawn_bitmap_ = -1;
                }
            }
        }
    }

    /*!
     * This is a class which keeps the rendering infrastructure for GL mode.
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
            synchronized(texture_mutex_)
            {
                Log.i(TAG, "OffscreenGLTextureRenderingSurface(obj=\""+object_name_+"\", texture="+gl_texture_id_
                    +", w="+view_width_+", h="+view_height_+") tid="+Thread.currentThread().getId());
                surface_texture_ = new SurfaceTexture(gl_texture_id_);
                surface_ = new Surface(surface_texture_);
                setNewSize(view_width_, view_height_);
            }
        }

        @Override
        public Canvas lockCanvas()
        {
            try
            {
                return surface_.lockCanvas(new Rect(0, 0, texture_width_, texture_height_));
            }
            catch(Exception e)
            {
                Log.e(TAG, "Failed to lock canvas for "+object_name_, e);
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
            // Log.i(TAG, "updateTexture tid="+Thread.currentThread().getId()+", tex="+gl_texture_id_);
            try
            {
                // Note: with this check we don't seem to need to sync to texture_mutex_.
                if (!has_texture_) // Nothing ever has been painted in the texture yet
                {
                    return false;
                }
                surface_texture_.updateTexImage();
                synchronized(texture_transform_mutex_)
                {
                    surface_texture_.getTransformMatrix(mtx_);
                    last_texture_width_ = last_painted_width_;
                    last_texture_height_ = last_painted_height_;
                }
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
            synchronized(texture_transform_mutex_)
            {
                return mtx_[index];
            }
        }

        @Override
        final public boolean hasTexture()
        {
            return has_texture_;
        }

        @Override
        public void setNewSize(final int w, final int h)
        {
            texture_width_ = w;
            texture_height_ = h;
            surface_texture_.setDefaultBufferSize(w, h); // API 15
        }

        @Override
        public void setBitmaps(final Bitmap bitmap_a, final Bitmap bitmap_b)
        {
        }

        @Override
        public int getQtPaintingTexture()
        {
            return -1;
        }
    }

    //! Can be called from either C++ or here to get avaialble API level.
    private static int api_level_ = 0;
    final public static int getApiLevel()
    {
        if (api_level_ < 1)
        {
            Build b = new Build();
            Build.VERSION ver = new Build.VERSION();
            api_level_ = ver.SDK_INT;
        }
        return api_level_;
    }

    //! Called from C++ to get currently visibile rectangle of the view.
    public void queryVisibleRect()
    {
        runViewAction(new Runnable(){
            @Override
            public void run()
            {
                View v = getView();
                if (v != null)
                {
                    Rect rect = new Rect();
                    v.getWindowVisibleDisplayFrame(rect);
                    nativeOnVisibleRect(getNativePtr(), rect.left, rect.top, rect.right, rect.bottom);
                }
            }
        });
    }

    public native void nativeUpdate(long nativeptr);
    public native Activity getActivity();
    public native void nativeViewCreated(long nativeptr);
    public native void nativeOnVisibleRect(long nativeptr, int left, int top, int right, int bottom);
}
