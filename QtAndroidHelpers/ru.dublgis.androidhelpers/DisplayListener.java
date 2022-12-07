/*
  Offscreen Android Views library for Qt

  Author:
  Sergey A. Galin <sergey.galin@gmail.com>

  Distrbuted under The BSD License

  Copyright (c) 2022, DoubleGIS, LLC.
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

import android.content.Context;
import android.hardware.display.DisplayManager;
import android.os.Handler;


public class DisplayListener implements DisplayManager.DisplayListener {
    private static final String TAG = "Grym/DisplayListener";
    private DisplayManager mDisplayManager = null;
    private long mNativePtr = 0;


    private static native void nativeDisplayAdded(long ptr, int id);
    private static native void nativeDisplayRemoved(long ptr, int id);
    private static native void nativeDisplayChanged(long ptr, int id);


    // C++
    public synchronized void register(final Context context, long nativePtr) {
        try {
            Log.i(TAG, "register");
            if (context == null) {
                Log.w(TAG, "Null Context n register()");
                return;
            }
            if (context == null) {
                Log.w(TAG, "Null nativePtr in register()");
                return;
            }
            mDisplayManager = (DisplayManager)context.getSystemService(Context.DISPLAY_SERVICE);
            mDisplayManager.registerDisplayListener(this, new Handler(context.getMainLooper()));
            mNativePtr = nativePtr;
        } catch (final Throwable e) {
            Log.e(TAG, "Exception in register(): ", e);
        }
    }


    // C++
    public synchronized void unregister() {
        try {
            Log.i(TAG, "unregister");
            mNativePtr = 0;
            if (mDisplayManager != null) {
                mDisplayManager.unregisterDisplayListener(this);
            }
        } catch (final Throwable e) {
            Log.e(TAG, "Exception in unregister(): ", e);
        }
    }


    @Override
    public synchronized void onDisplayAdded(int displayId) {
        Log.i(TAG, "onDisplayAdded " + displayId);
        if (mNativePtr != 0) {
            try {
                nativeDisplayAdded(mNativePtr, displayId);
            } catch (final Throwable e) {
                Log.e(TAG, "JNI call exception in onDisplayAdded: ", e);
            }
        }
    }


    @Override
    public synchronized void onDisplayRemoved(int displayId) {
        Log.i(TAG, "onDisplayRemoved " + displayId);
        if (mNativePtr != 0) {
            try {
                nativeDisplayRemoved(mNativePtr, displayId);
            } catch (final Throwable e) {
                Log.e(TAG, "JNI call exception in onDisplayRemoved: ", e);
            }
        }
    }


    @Override
    public synchronized void onDisplayChanged(int displayId) {
        Log.i(TAG, "onDisplayChanged " + displayId);
        if (mNativePtr != 0) {
            try {
                nativeDisplayChanged(mNativePtr, displayId);
            } catch (final Throwable e) {
                Log.e(TAG, "JNI call exception in onDisplayChanged: ", e);
            }
        }
    }
}

