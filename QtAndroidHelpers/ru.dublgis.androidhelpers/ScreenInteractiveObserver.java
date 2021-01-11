package ru.dublgis.androidhelpers;

import android.content.Context;
import android.os.Build;
import android.os.PowerManager;
import android.content.Intent;
import android.content.IntentFilter;
import android.app.Activity;
import android.content.BroadcastReceiver;

public class ScreenInteractiveObserver extends BroadcastReceiver {
        public static final String TAG = "Grym/ScreenInteractiveObserver";
        private volatile long native_ptr_ = 0;

        public ScreenInteractiveObserver(final long native_ptr)
        {
                Log.i(TAG, "Constructed");
                native_ptr_ = native_ptr;

                IntentFilter filter = new IntentFilter();
                filter.addAction(Intent.ACTION_SCREEN_ON);
                filter.addAction(Intent.ACTION_SCREEN_OFF);
                getActivity().registerReceiver(this, filter);
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
                                return Build.VERSION.SDK_INT > 20 ? pm.isInteractive() : pm.isScreenOn();
                        }
                        else
                        {
                                Log.w(TAG, "PowerManager service not found.");
                        }
                }
                catch (final LinkageError e)
                {
                        Log.e(TAG, "ScreenInteractiveObserver linkage error: " + e);
                }
                catch (final Throwable e)
                {
                        Log.e(TAG, "ScreenInteractiveObserver exception: ", e);
                }
                return false;
        }

        @Override
        public void onReceive(Context context, Intent intent) {
                if (intent.getAction().equals(Intent.ACTION_SCREEN_OFF) || intent.getAction().equals(Intent.ACTION_SCREEN_ON))
                {
                        onInteractiveChanged(native_ptr_);
                }
        }

        public native Context getContext();
        private native Activity getActivity();
        private native void onInteractiveChanged(long nativeptr);

}

