package ru.dublgis.offscreenview;

import android.app.Activity;
import android.util.Log;

public class ClassLoader
{
    public static final String TAG = "Grym/OffscreenView";

    static public void callJNIPreloadClass(Activity activity)
    {
       Log.i(TAG, "callJNIPreloadClass ***********************************************************************");
       (new ClassLoader()).nativeJNIPreloadClass();

       activity.runOnUiThread(new Runnable(){
          @Override
          public void run(){
              Log.i(TAG, "callJNIPreloadClass SUCCESSS +++++++++++++++++++++++++++++++++");
          }
       });
    }

    private native void nativeJNIPreloadClass();
}

