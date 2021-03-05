package ru.dublgis.androidhelpers;

import android.content.Context;
import android.os.Build;
import android.os.PowerManager;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.BroadcastReceiver;

public class PowerManagerHelper extends BroadcastReceiver {
        public static final String TAG = "Grym/PowerManagerManager";
        private volatile long native_ptr_ = 0;

        public PowerManagerHelper(final long native_ptr)
        {
                native_ptr_ = native_ptr;
                try
                {
                        IntentFilter filter = new IntentFilter();
                        filter.addAction(Intent.ACTION_SCREEN_ON);
                        filter.addAction(Intent.ACTION_SCREEN_OFF);
                        getContext().registerReceiver(this, filter);
                }
                catch (final Throwable e)
                {
                        Log.e(TAG, "QAndroidPowerManagerHelper exception: ", e);
                }
        }

        //! Called from C++ to notify us that the associated C++ object is being destroyed.
        public void cppDestroyed()
        {
                native_ptr_ = 0;
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
                        Log.e(TAG, "PowerManagerHelper exception: ", e);
                }
                return false;
        }

        @Override
        public void onReceive(Context context, Intent intent) {
                try
                {
                        if (native_ptr_ != 0 && (intent.getAction().equals(Intent.ACTION_SCREEN_OFF) || intent.getAction().equals(Intent.ACTION_SCREEN_ON)))
                        {
                                onInteractiveChanged(native_ptr_);
                        }
                }
                catch (final Throwable e)
                {
                        Log.e(TAG, "PowerManagerHelper exception: ", e);
                }
        }

        public native Context getContext();
        private native void onInteractiveChanged(long nativeptr);
}

