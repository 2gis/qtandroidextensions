package ru.dublgis.jniutils;

// import android.util.Log;

public class ClassLoader
{
    // public static final String TAG = "Grym/JNIUtils";

    static public void callJNIPreloadClass(final String classname)
    {
        // Log.i(TAG, "callJNIPreloadClass ***********************************************************************");
        (new ClassLoader()).nativeJNIPreloadClass(classname);
    }

    private native void nativeJNIPreloadClass(String classname);
}
