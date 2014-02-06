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
    private Boolean painting_now_ = new Boolean(false);
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
    private boolean invalidated_ = true;


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
            setMeasuredDimension(view_left_+view_width_, view_top_+view_height_);
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
            child.layout(view_left_, view_top_, view_left_+view_width_, view_top_+view_height_);
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
                    view.setLeft(0);
                    view.setTop(0);
                    view.setRight(view_width_);
                    view.setBottom(view_height_);

                    // Insert the View into layout.
                    // Note: functions of many views will crash if they are not inserted into layout.
                    layout_ = new MyLayout(activity);
                    layout_.setRight(view_width_);
                    layout_.setBottom(view_height_);
                    layout_.addView(view);
                    attachViewToQtScreen();

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
                    doNativeViewCreated();
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

    private boolean attachViewToQtScreen()
    {
        try
        {
            if (!attaching_mode_ || is_attached_)
            {
                return false;
            }
            final View view = layout_;
            if (view == null)
            {
                Log.e(TAG, "Failed to insert "+object_name_+" into the ViewGroup because View is null!");
                return false;
            }
            ViewGroup vg = getMainLayout();
            if (vg != null)
            {
                Log.i(TAG, "Inserting "+object_name_+" (view id="+view.getId()+") into the ViewGroup...");
                vg.addView(view);
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
            Log.e(TAG, "Exception in attachViewToQtScreen:", e);
            return false;
        }
    }

    /*!
     * \note This function doesn't check attaching_mode_/is_attached_.
     */
    private boolean detachViewFromQtScreen()
    {
        try
        {
            final Activity activity = getActivity();
            final View view = layout_;
            if (activity == null || view == null)
            {
                Log.e(TAG, "Failed to insert "+object_name_+" into the ViewGroup because Activity or View is null!");
            }
            ViewGroup vg = (ViewGroup)activity.findViewById(android.R.id.content);
            if (vg != null)
            {
                Log.i(TAG, "Removing "+object_name_+" (view id="+view.getId()+") from the ViewGroup...");
                vg.removeView(view);
                is_attached_ = false;
                return true;
            }
            else
            {
                Log.w(TAG, "Failed to remove "+object_name_+" from the ViewGroup because it was not found!");
                return false;
            }
        }
        catch(Exception e)
        {
            Log.e(TAG, "Exception in detachViewFromQtScreen:", e);
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
                            attachViewToQtScreen();
                        }
                        else if (!attaching && is_attached_)
                        {
                            detachViewFromQtScreen();
                        }
                    }
                }
            });
        }
    }

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
                        layout_.postInvalidate();
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
                        layout_.postInvalidate();
                    }
                    invalidateOffscreenView();
                }
            }
        });
    }

    abstract public void callViewPaintMethod(Canvas canvas);
    abstract public void doNativeUpdate();
    abstract public void doCreateView();
    abstract public void doNativeViewCreated();
    abstract public Activity getActivity();

    //! Note: all views are hidden by default (after creation).
    public boolean isVisible()
    {
        return last_visibility_;
    }

    //! Note: all views are hidden by default.
    public void setVisible(final boolean visible)
    {
        if (visible != last_visibility_)
        {
            last_visibility_ = visible;
            runViewAction(new Runnable(){
                @Override
                public void run()
                {
                    View v = getView();
                    int vis = last_visibility_? View.VISIBLE: View.INVISIBLE;
                    if (v != null && v.getVisibility() != vis)
                    {
                        if (!visible)
                        {
                            uiHideKeyboardFromView();
                        }
                        v.setVisibility(vis);
                    }
                }
            });
        }
    }

    public boolean isEnabled()
    {
        return last_enabled_;
    }

    public void setEnabled(final boolean enabled)
    {
        if (enabled != last_enabled_)
        {
            last_enabled_ = enabled;
            runViewAction(new Runnable(){
                @Override
                public void run()
                {
                    View v = getView();
                    if (v != null && v.isEnabled() != enabled)
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
    }

    //! Schedules doDrawViewOnTexture() with filtering out subsequent calls.
    protected void invalidateTexture()
    {
        invalidated_ = true;
        // A little dance with a tambourine to filter out subsequent invalidations happened before a single paint
        new Handler().post(new Runnable(){
            @Override
            public void run()
            {
                // Log.i(TAG, "invalidateTexture: processing with invalidated_="+invalidated_);
                if (invalidated_)
                {
                    invalidated_ = false;
                    doDrawViewOnTexture();
                }
            }
        });
    }

    //! Schedule painting of the view (will be done in Android UI thread).
    protected void drawViewOnTexture()
    {
        runOnUiThread(new Runnable()
        {
            @Override
            public void run()
            {
                doDrawViewOnTexture();
            }
        });
    }

    final protected boolean isInOffscreenDraw()
    {
        return painting_now_;
    }

    //! Performs actual painting of the view. Should be called in Android UI thread.
    protected void doDrawViewOnTexture()
    {
        synchronized(painting_now_)
        {
            painting_now_ = true;
        }
        synchronized(texture_mutex_)
        {
            // Note: with a null View we will continue, but will only fill
            // background with the fill color.
            if (rendering_surface_ == null || getNativePtr() == 0)
            {
                Log.i(TAG, "doDrawViewOnTexture: surface or native ptr is null. NP="+getNativePtr()
                    +", RS? "+((rendering_surface_ == null)?"null":"not null"));
                synchronized(painting_now_)
                {
                     painting_now_ = false;
                }
                return;
            }
            try
            {
                // TODO: disable time measurement
                // long t = System.nanoTime();
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
                            // Log.i(TAG, "doDrawViewOnTexture view size:"+v.getWidth()+"x"+v.getHeight());

                            // Prepare canvas.
                            // Take View scroll into account.
                            canvas.translate(-v.getScrollX(), -v.getScrollY());

                            callViewPaintMethod(canvas);

                            synchronized(texture_transform_mutex_)
                            {
                                last_painted_width_ = v.getWidth();
                                last_painted_height_ = v.getHeight();
                            }
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
                    doNativeUpdate();

                    // Log.i(TAG, "doDrawViewOnTexture: success, t="+t/1000000.0+"ms");
                }
            }
            catch(Exception e)
            {
                Log.e(TAG, "doDrawViewOnTexture exception:", e);
            }
        }
        synchronized(painting_now_)
        {
            painting_now_ = false;
        }
    }

    //! Called from C++ to get current texture.
    public boolean updateTexture(boolean synced)
    {
        if (synced)
        {
            // If Android UI thread is drawing the View now, the sync will pause execution it finishes.
            synchronized(texture_mutex_)
            {
                return unsyncedUpdateTexture();
            }
        }
        else
        {
            return unsyncedUpdateTexture();
        }
    }

    private boolean unsyncedUpdateTexture()
    {
        // Async (non-blocking) update, or already blocked painting
        if (rendering_surface_ == null)
        {
            return false;
        }
        // If the view is being currently painted, just back off
        synchronized(painting_now_)
        {
            if (painting_now_)
            {
                return false;
            }
        }
        // Note: the painting may sometimes start right here.
        // We assume that it's safe to call rendering surfaces's updateTexture()
        // in parallel to painting, it will simply leave the previous texture.
        return rendering_surface_.updateTexture();
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
    }

    //! Called from C++
    public int getQtPaintingTexture()
    {
        if (rendering_surface_ != null)
        {
            return rendering_surface_.getQtPaintingTexture();
        }
        else
        {
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

    // Called from C++ to force the view
    public void invalidateOffscreenView()
    {
        Log.i(TAG, "invalidateOffscreenView");
        if (getView() != null)
        {
            runOnUiThread(new Runnable() {
                @Override
                public void run()
                {
                    // if (getView() != null)... assuming that view cannot be unassigned once created
                    invalidateTexture();
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
                Log.i(TAG, "setFocused("+focused+"): run");
                final View v = getView();
                if (v != null)
                {
                    if (focused)
                    {
                        v.setFocusable(true);
                        v.setFocusableInTouchMode(true);
                        v.requestFocus();
                    }
                    else
                    {
                        v.setFocusable(false);
                        v.setFocusableInTouchMode(false);
                        // Simply passing focus to someone else or calling ClearFocus()
                        // does not hide SIP for some unknown reason. We have to do that expliciltly.
                        uiHideKeyboardFromView();
                    }
                }
            }
        });
    }

    /*!
     * We need to workaround a problem: when View is defocused it doesn't hide SIP (weird but no ideas how to fix it another way).
     */
    protected void uiHideKeyboardFromView()
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
                            v.setLeft(0);
                            v.setTop(0);
                            v.setRight(w);
                            v.setBottom(h);
                            invalidateTexture();
                        }
                        else
                        {
                            v.forceLayout();
                            v.requestLayout();
                        }
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
                    layout_.postInvalidate();
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
                    detachViewFromQtScreen();
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
        //! \todo FIXME remove this function?
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
                return new Canvas((draw_bitmap_ == 0)? bitmap_a_: bitmap_b_);
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
                    return -1;
                }
                // Swapping buffers, so Android won't paint on the Bitmap
                // which is currently being used by Qt.
                int old_draw_bitmap = draw_bitmap_;
                draw_bitmap_ = (draw_bitmap_ == 0)? 1: 0;
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
                if (bitmap_a_ != bitmap_a || bitmap_b_ != bitmap_b)
                {
                    bitmap_a_ = bitmap_a;
                    bitmap_b_ = bitmap_b;
                    draw_bitmap_ = 0;
                    has_texture_ = false;
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

}
