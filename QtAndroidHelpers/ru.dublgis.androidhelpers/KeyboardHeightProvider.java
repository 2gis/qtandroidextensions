/*
    Offscreen Android Views library for Qt

    Authors:
    Timur N. Artikov <t.artikov@gmail.com>
    Sergey A. Galin <sergey.galin@gmail.com>

    Distrbuted under The BSD License

    Copyright (c) 2019, DoubleGIS, LLC.
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
import android.content.res.Configuration;
import android.graphics.Point;
import android.graphics.Rect;
import android.os.Build;
import android.util.DisplayMetrics;
import android.view.Display;
import android.view.Gravity;
import android.view.View;
import android.view.ViewTreeObserver;
import android.view.WindowManager;
import android.view.WindowManager.LayoutParams;
import android.widget.PopupWindow;


/*
    The keyboard height provider.
    It uses a PopupWindow to calculate the window height when the floating keyboard is opened and closed.
*/
public class KeyboardHeightProvider
    extends PopupWindow
    implements ViewTreeObserver.OnGlobalLayoutListener 
{
    public static final String TAG = "Grym/KeyboardHeightProvider";

    private KeyboardHeightObserver mObserver = null;
    private View mPopupView = null;
    private Activity mActivity = null;


    public KeyboardHeightProvider(
        final Activity activity,
        final View parentView, 
        final KeyboardHeightObserver observer)
    {
        super(activity);
        try {
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
        } catch (final Throwable e) {
            Log.e(TAG, "constructor exception: " + e);
        }
    }


    public void stop() {
        try {
            mPopupView.getViewTreeObserver().removeOnGlobalLayoutListener(this);
            dismiss();
        } catch (final Throwable e) {
            Log.e(TAG, "stop exception: " + e);
        }
    }


    private int nominalScreenHeight() {
        final Point screenSize = new Point();
        mActivity.getWindowManager().getDefaultDisplay().getSize(screenSize);
        return screenSize.y;
    }


    private int visibleDisplayFrameHeight() {
        final Rect rect = new Rect();
        mPopupView.getWindowVisibleDisplayFrame(rect);
        return rect.bottom;
    }


    private int getKeyboardSize() {
        int keyboardHeight = 0;
        try {
            final int screenHeight = nominalScreenHeight();
            keyboardHeight = screenHeight - visibleDisplayFrameHeight();
            if (keyboardHeight == 0
                || !SystemNavigationBarInfo.deviceMayHaveFullscreenMode(mActivity) // Fast (cached)
                || mActivity.getResources().getConfiguration().orientation ==
                    Configuration.ORIENTATION_LANDSCAPE)
            {
                return keyboardHeight;
            }

            // Below are additional calculations and workarounds for devices that have dynamically
            // shown and hidden navigation panels.
            // These panels have non-standard implementation by Samsung, Huawei, ViVo and some
            // others so the code to support them all is quite weird and may cause problems
            // in the future. But we really have no choice.

            /*
            Log.d(TAG"getNavigationBarHeightFromConfiguration=" +
                SystemNavigationBarInfo.getNavigationBarHeightFromConfiguration(
                    Configuration.ORIENTATION_PORTRAIT) +
                ", screenHeight=" + screenHeight +
                ", isInFullscreenMode=" + SystemNavigationBarInfo.isInFullscreenMode(mActivity) +
                ", hasVerticalNavBarSpace=" +
                SystemNavigationBarInfo.hasVerticalNavBarSpace(mActivity) +
                ", getActualNavigationBarControlHeight=" +
                SystemNavigationBarInfo.getActualNavigationBarControlHeight(mActivity) +
                ", deviceMayHaveFullscreenMode=" +
                SystemNavigationBarInfo.deviceMayHaveFullscreenMode(mActivity));
            */

            final boolean fullscreen = SystemNavigationBarInfo.isInFullscreenMode(mActivity);
            final boolean hasNavBarSpace = SystemNavigationBarInfo.hasVerticalNavBarSpace(mActivity);

            // Correcting for "control is too high above the keyboard" case
            if (!fullscreen && !hasNavBarSpace) {
                return keyboardHeight - SystemNavigationBarInfo.getActualNavigationBarControlHeight(mActivity);
            }

            // Correcting for "control is partially or fully covered by the keyboard" case
            if (fullscreen &&
                hasNavBarSpace &&
                SystemNavigationBarInfo.getActualNavigationBarControlHeight(mActivity) == 0)
            {
                return keyboardHeight + SystemNavigationBarInfo.getNavigationBarHeightFromConfiguration(
                    Configuration.ORIENTATION_PORTRAIT);
            }
        } catch (final Throwable e) {
            Log.e(TAG, "getKeyboardSize exception: " + e);
        }
        return keyboardHeight;
    }


    @Override
    public void onGlobalLayout()
    {
        try {
            int keyboardHeight = getKeyboardSize();
            if (keyboardHeight < 0) {
                Log.w(TAG, "Invalid keyboard height calculated: " + keyboardHeight);
                keyboardHeight = 0;
            }
            mObserver.onKeyboardHeightChanged(keyboardHeight);
        } catch (final Throwable e) {
            Log.e(TAG, "onGlobalLayout exception: " + e);
        }
    }
}
