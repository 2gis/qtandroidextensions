/*
    Offscreen Android Views library for Qt

    Author:
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
import android.content.ContentResolver;
import android.content.Context;
import android.content.res.Configuration;
import android.content.res.Resources;
import android.graphics.Point;
import android.os.Build;
import android.provider.Settings;
import android.util.DisplayMetrics;
import android.view.Display;
import android.view.View;
import android.view.WindowManager;


public class SystemNavigationBarInfo {
    private static final String TAG = "Grym/SystemNavBarInfo";
    private static volatile boolean mDeviceMayHaveFullscreenModeCache = false;
    private static volatile boolean mDeviceMayHaveFullscreenModeCacheSet = false;


    // Check if application is running in full screen mode (i.e. nav bar is hidden).
    // Uses vendor-specific code so may not work properly on future devices.
    // Call deviceMayHaveFullscreenMode(Context) first for optimized behaviour.
    public static boolean isInFullscreenMode(final Context context) {
        if (Build.VERSION.SDK_INT < 21) {
            return false;
        }
        try {
            final ContentResolver cr = context.getContentResolver();
            return
                // Huawei
                Settings.System.getInt(cr, "navigationbar_is_min", 0) == 1 ||
                Settings.Global.getInt(cr, "navigationbar_is_min", 0) == 1 ||
                // Samsung
                Settings.System.getInt(cr, "navigationbar_hide_bar_enabled", 0) == 1 ||
                Settings.Global.getInt(cr, "navigationbar_hide_bar_enabled", 0) == 1 ||
                // Xiaomi MIUI
                Settings.Global.getInt(cr, "force_fsg_nav_bar", 0) != 0 ||
                // ViVo
                Settings.Secure.getInt(cr, "navigation_gesture_on", 0) != 0 ||
                // Other
                "immersive.navigation=*".equals(Settings.System.getString(cr, "policy_control")) ||
                "immersive.navigation=*".equals(Settings.Global.getString(cr, "policy_control"));
        } catch (final Throwable e) {
            Log.e(TAG, "getFullscreenMode exception: ", e);
            return false;
        }
    }


    // Check if real screen size is bigger than nominal screen size, e.g. contains
    // additional system panels not included into the nominal size.
    // This may also return true for phones with non-hiding nav bar,
    // and also when device is emulating smaller display (adb shell wm size).
    // Some devices like Huawei P20 may not allocate nav bar space when activity is started
    // with hidden nav bar but may (or may not) resize the screen later.
    public static boolean hasVerticalNavBarSpace(final Activity activity) {
        try {
            if (Build.VERSION.SDK_INT < 17) { // 4.2
                return false;
            }
            final Display display = activity.getWindowManager().getDefaultDisplay();
            final DisplayMetrics displayMetrics = new DisplayMetrics();
            final DisplayMetrics realDisplayMetrics = new DisplayMetrics();
            display.getMetrics(displayMetrics);
            display.getRealMetrics(realDisplayMetrics);
            return realDisplayMetrics.heightPixels > displayMetrics.heightPixels;
        } catch (final Throwable e) {
            Log.e(TAG, "hasVerticalNavBarSpace exception: ", e);
            return false;
        }
    }


    // Get height of the navigation bar by looking up "navigationBarBackground" control.
    // The function may return 0 if the nav bar is hidden.
    // Obviosly, this function may break in some future version of Android.
    public static int getActualNavigationBarControlHeight(final Activity activity) {
        try {
            final View bar = activity.getWindow().getDecorView().findViewById(
                android.R.id.navigationBarBackground);
            if (bar != null) {
                return bar.getMeasuredHeight();
            }
        } catch (final Throwable e) {
            Log.e(TAG, "getActualNavigationBarControlHeight exception: ", e);
        }
        return 0;
    }


    // Get navigation bar height from system configuration.
    // Works on "normal" devices with fixed nav bar as well.
    // Obviosly, this function may break in some future version of Android.
    public static int getNavigationBarHeightFromConfiguration(final int orientation) {
        try {
            final int resourceId = Resources.getSystem().getIdentifier(
                (orientation == Configuration.ORIENTATION_LANDSCAPE)
                    ? "navigation_bar_height_landscape"
                    : "navigation_bar_height",
                "dimen",
                "android");
            if (resourceId > 0) {
                return Resources.getSystem().getDimensionPixelSize(resourceId);
            }
        } catch (final Resources.NotFoundException e) {
            Log.d(TAG, "getNavigationBarHeightFromConfiguration: key not found.");
        } catch (final Throwable e) {
            Log.e(TAG, "getNavigationBarHeightFromConfiguration exception: ", e);
        }
        return 0;
    }


    // Detecting device that may have full screen mode (collapsible nav panel).
    // This is just a fast heuristic-based check that currently looks solely on the aspect ratio
    // of the screen and Android version. It will work improperly if someone release
    // a device with wide screen and collapsible navbar. TODO...
    public static boolean deviceMayHaveFullscreenMode(final Context context) {
        if (mDeviceMayHaveFullscreenModeCacheSet) {
            return mDeviceMayHaveFullscreenModeCache;
        }
        if (Build.VERSION.SDK_INT < 21) { // 5.0
            mDeviceMayHaveFullscreenModeCache = false;
            mDeviceMayHaveFullscreenModeCacheSet = true;
            return false;
        }
        // TODO: Here is some wonderful magic. Assuming that so far only the devices with
        // very high aspect ratio screen may have a hiding nav bar. Obviously this check may not
        // work properly for some current or future devices.
        try {
            final Point sz = new Point();
            ((WindowManager)context.getSystemService(Context.WINDOW_SERVICE))
                .getDefaultDisplay().getRealSize(sz);
            if (((sz.x >= sz.y) ? sz.x / sz.y : sz.y / sz.x) > 1.97f) {
                mDeviceMayHaveFullscreenModeCache = true;
            }
        } catch (final Throwable e) {
            Log.e(TAG, "deviceMayHaveFullscreenMode exception: ", e);
        }
        mDeviceMayHaveFullscreenModeCacheSet = true;
        return mDeviceMayHaveFullscreenModeCache;
    }
}
