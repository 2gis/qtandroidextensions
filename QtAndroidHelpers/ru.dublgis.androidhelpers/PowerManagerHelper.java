/*
  Offscreen Android Views library for Qt

  Author:
  Semen Zinkov <zinjkov.su@gmail.com>

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

import android.content.Context;
import android.os.Build;
import android.os.PowerManager;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.BroadcastReceiver;


public class PowerManagerHelper extends BroadcastReceiver {
    public static final String TAG = "Grym/PowerManagerManager";
    private long mNativePtr = 0;

    public PowerManagerHelper(final long native_ptr)
    {
        mNativePtr = native_ptr;
        try
        {
            IntentFilter filter = new IntentFilter();
            filter.addAction(Intent.ACTION_SCREEN_ON);
            filter.addAction(Intent.ACTION_SCREEN_OFF);
            getContext().registerReceiver(this, filter);
        }
        catch (final Throwable e)
        {
            Log.e(TAG, "registerReceiver failed: ", e);
        }
    }

    //! Called from C++ to notify us that the associated C++ object is being destroyed.
    public void cppDestroyed()
    {
        synchronized(this) {
            mNativePtr = 0;
        }
    }

    public boolean isInteractive()
    {
        try
        {
            final Context context = getContext();
            PowerManager pm = (PowerManager)context.getSystemService(Context.POWER_SERVICE);
            if (pm != null)
            {
                return (Build.VERSION.SDK_INT >= 21)
                    ? pm.isInteractive() // Android 5+
                    : pm.isScreenOn(); // Android < 5
            }
            else
            {
                Log.w(TAG, "PowerManager service not found.");
            }
        }
        catch (final Throwable e)
        {
            Log.e(TAG, "Get PowerManager failed: ", e);
        }
        return false;
    }

    @Override
    public void onReceive(Context context, Intent intent) {
        try
        {
            synchronized(this) {
                if (mNativePtr != 0 && (intent.getAction().equals(Intent.ACTION_SCREEN_OFF) || intent.getAction().equals(Intent.ACTION_SCREEN_ON)))
                {
                    onInteractiveChanged(mNativePtr);
                }
            }
        }
        catch (final Throwable e)
        {
            Log.e(TAG, "onInteractiveChanged failed: ", e);
        }
    }

    public native Context getContext();
    private native void onInteractiveChanged(long nativeptr);
}

