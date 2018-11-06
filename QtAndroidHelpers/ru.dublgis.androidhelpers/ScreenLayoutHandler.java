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

package ru.dublgis.androidhelpers;

import android.app.Activity;
import android.view.View;
import android.view.ViewTreeObserver;
import android.view.Window;


public class ScreenLayoutHandler implements 
    ViewTreeObserver.OnGlobalLayoutListener, 
    ViewTreeObserver.OnScrollChangedListener,
    KeyboardHeightObserver
{
    public static final String TAG = "Grym/ScrnLayoutHandler";
    private volatile long native_ptr_ = 0;
    private KeyboardHeightProvider mKeyboardHeightProvider;

    public ScreenLayoutHandler(long native_ptr)
    {
        Log.i(TAG, "ScreenLayoutHandler constructor");
        native_ptr_ = native_ptr;
        subscribeToLayoutEvents();
    }

    //! Called from C++ to notify us that the associated C++ object is being destroyed.
    public void cppDestroyed()
    {
        unsubscribeFromLayoutEvents();
        native_ptr_ = 0;
    }

    public void subscribeToLayoutEvents()
    {
        runOnUiThread(new Runnable(){
            @Override
            public void run()
            {
                View view = getDecorView();
                if (view != null)
                {
                    try
                    {
                        view.getViewTreeObserver().addOnGlobalLayoutListener(ScreenLayoutHandler.this);
                        view.getViewTreeObserver().addOnScrollChangedListener(ScreenLayoutHandler.this);
                        if (mKeyboardHeightProvider == null)
                        {
                            mKeyboardHeightProvider = new KeyboardHeightProvider(getActivity(), view, ScreenLayoutHandler.this);
                        }
                    }
                    catch (Exception e)
                    {
                        Log.e(TAG, "Exception when add on glogal layout listener:", e);
                    }
                }
            }
        });
    }

    public void unsubscribeFromLayoutEvents()
    {
        runOnUiThread(new Runnable(){
            @Override
            public void run()
            {
                View view = getDecorView();
                if (view != null)
                {
                    try
                    {
                        view.getViewTreeObserver().removeOnGlobalLayoutListener(ScreenLayoutHandler.this);
                        view.getViewTreeObserver().removeOnScrollChangedListener(ScreenLayoutHandler.this);
                        if (mKeyboardHeightProvider != null)
                        {
                            mKeyboardHeightProvider.stop();
                            mKeyboardHeightProvider = null;
                        }
                    }
                    catch (Exception e)
                    {
                        Log.e(TAG, "Exception when remove on glogal layout listener:", e);
                    }
                }
            }
        });
    }

    @Override
    public void onGlobalLayout()
    {
        nativeGlobalLayoutChanged(native_ptr_);
    }

    @Override
    public void onScrollChanged()
    {
        nativeScrollChanged(native_ptr_);
    }

    @Override
    public void onKeyboardHeightChanged(int height)
    {
        nativeKeyboardHeightChanged(native_ptr_, height);
    }

    private View getDecorView()
    {
        final Activity context = getActivity();
        if (context == null)
        {
            return null;
        }

        Window window = context.getWindow();
        if (window == null)
        {
            return null;
        }

        return window.getDecorView();
    }

    final public boolean runOnUiThread(final Runnable runnable)
    {
        try
        {
            if (runnable == null)
            {
                Log.e(TAG, "ScreenLayoutHandler.runOnUiThread: null runnable!");
                return false;
            }
            final Activity context = getActivity();
            if (context == null)
            {
                Log.e(TAG, "ScreenLayoutHandler.runOnUiThread: cannot schedule task because of the null context!");
                return false;
            }
            context.runOnUiThread(runnable);
            return true;
        }
        catch (Exception e)
        {
            Log.e(TAG, "Exception when posting a runnable:", e);
            return false;
        }
    }

    public native Activity getActivity();
    public native void nativeGlobalLayoutChanged(long nativeptr);
    public native void nativeScrollChanged(long nativeptr);
    public native void nativeKeyboardHeightChanged(long nativeptr, int height);
}
