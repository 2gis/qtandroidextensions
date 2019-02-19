/*
    Offscreen Android Views library for Qt

    Author:
    Timur N. Artikov <t.artikov@gmail.com>

    Distrbuted under The BSD License

    Copyright (c) 2018, DoubleGIS, LLC.
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
import android.graphics.Point;
import android.graphics.Rect;
import android.view.Gravity;
import android.view.View;
import android.view.ViewTreeObserver;
import android.view.WindowManager.LayoutParams;
import android.widget.PopupWindow;
import android.util.DisplayMetrics;
import android.view.Display;


/*
    The keyboard height provider. 
    It uses a PopupWindow to calculate the window height when the floating keyboard is opened and closed.
*/
public class KeyboardHeightProvider 
    extends PopupWindow
    implements ViewTreeObserver.OnGlobalLayoutListener 
{
    public static final String TAG = "Grym/KeyboardHeightProvider";

    private KeyboardHeightObserver mObserver;
    private View mPopupView;
    private Activity mActivity;

    public KeyboardHeightProvider(Activity activity, View parentView, KeyboardHeightObserver observer)
    {
        super(activity);
        try
        {
            mActivity = activity;
            mObserver = observer;

            mPopupView = new View(activity);
            mPopupView.setLayoutParams(new LayoutParams(LayoutParams.MATCH_PARENT, LayoutParams.MATCH_PARENT));
            setContentView(mPopupView);

            setSoftInputMode(LayoutParams.SOFT_INPUT_ADJUST_RESIZE | LayoutParams.SOFT_INPUT_STATE_ALWAYS_VISIBLE);
            setInputMethodMode(PopupWindow.INPUT_METHOD_NEEDED);

            setWidth(0);
            setHeight(LayoutParams.MATCH_PARENT);

            mPopupView.getViewTreeObserver().addOnGlobalLayoutListener(this);
            showAtLocation(parentView, Gravity.NO_GRAVITY, 0, 0);
        }
        catch (final Throwable e)
        {
            Log.e(TAG, "constructor exception: " + e);
        }
    }

    public void stop()
    {
        try
        {
            mPopupView.getViewTreeObserver().removeOnGlobalLayoutListener(this);
            dismiss();
        }
        catch (final Throwable e)
        {
            Log.e(TAG, "stop exception: " + e);
        }
    }

    @Override
    public void onGlobalLayout() 
    {
        try
        {
            //Getting dsiplay size
            //https://stackoverflow.com/questions/1016896/get-screen-dimensions-in-pixels/15699681#15699681
            Point screenSize = new Point();
            Display d = mActivity.getWindowManager().getDefaultDisplay();
            d.getSize(screenSize);
            int heightPixels = screenSize.y;

            try
            {
                DisplayMetrics metrics = new DisplayMetrics();
                d.getMetrics(metrics);
                heightPixels = metrics.heightPixels;
            }
            catch(final Throwable e)
            {
                Log.e(TAG, "onGlobalLayout exception (1): " + e);
            }

            if (android.os.Build.VERSION.SDK_INT >= 14 && android.os.Build.VERSION.SDK_INT < 17) {
                try
                {
                    heightPixels = (Integer) Display.class.getMethod("getRawHeight").invoke(d);
                }
                catch(final Throwable e)
                {
                    Log.e(TAG, "onGlobalLayout exception (2): " + e);
                }
            }

            if (android.os.Build.VERSION.SDK_INT >= 17) {
                try
                {
                    Point realSize = new Point();
                    Display.class.getMethod("getRealSize", Point.class).invoke(d, realSize);
                    heightPixels = realSize.y;
                }
                catch(final Throwable e)
                {
                    Log.e(TAG, "onGlobalLayout exception (3): " + e);
                }
            }

            Rect rect = new Rect();
            mPopupView.getWindowVisibleDisplayFrame(rect);

            int keyboardHeight = heightPixels - rect.bottom;

            // keyboardHeight can be negative, when navigation panel is hided
            // screenSize doesn't take into account the possibility of hiding panel
            if (keyboardHeight < 0) {
                keyboardHeight = 0;
            }
            mObserver.onKeyboardHeightChanged(keyboardHeight);
        }
        catch (final Throwable e)
        {
            Log.e(TAG, "onGlobalLayout exception: " + e);
        }
    }
}
