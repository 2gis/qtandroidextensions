package ru.dublgis.jniutils;

import android.app.Activity;
import android.util.Log;

public class ClassLoader
{
    public static final String TAG = "Grym/JNIUtils";

    static public void callJNIPreloadClass(final String classname) throws Exception
    {
        Log.i(TAG, "callJNIPreloadClass ***********************************************************************");
        try
        {
            (new ClassLoader()).nativeJNIPreloadClass(classname);
        }
        catch(Exception e)
        {
            Log.e(TAG, "callJNIPreloadClass nativeJNIPreloadClass exception:", e);
        }

        throw new Exception("This thing makes me crazy!");
    }

    private native void nativeJNIPreloadClass(String classname);
}
