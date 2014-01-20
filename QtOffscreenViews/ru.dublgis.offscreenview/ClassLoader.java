package ru.dublgis.offscreenview;

import android.util.Log;

public class ClassLoader
{
    public static final String TAG = "Grym/OffscreenView";

    static public void callJNIPreloadClass()
    {
       Log.i(TAG, "callJNIPreloadClass ***********************************************************************");
       (new ClassLoader()).nativeJNIPreloadClass();
    }

    private native void nativeJNIPreloadClass();
}

