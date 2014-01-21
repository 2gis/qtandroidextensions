package ru.dublgis.jniutils;

import android.app.Activity;
import android.util.Log;

public class ClassLoader
{
    public static final String TAG = "Grym/JNIUtils";

    static public void callJNIPreloadClass(final Activity activity, final String classname)
    {
       Log.i(TAG, "callJNIPreloadClass ***********************************************************************");
       (new ClassLoader()).nativeJNIPreloadClass(classname);

       // SGEXP
       activity.runOnUiThread(new Runnable(){
          @Override
          public void run(){
              Log.i(TAG, "callJNIPreloadClass SUCCESSS +++++++++++++++++++++++++++++++++");
          }
       });
    }

    private native void nativeJNIPreloadClass(String classname);
}
