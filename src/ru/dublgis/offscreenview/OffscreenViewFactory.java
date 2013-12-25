package ru.dublgis.offscreenview;

import java.lang.Thread;
import java.util.Set;
import java.util.List;
import java.util.LinkedList;
import java.util.Arrays;
import java.util.TreeSet;
import java.util.Locale;
import java.util.List;
import java.util.LinkedList;
import java.io.File;
import android.app.Activity;
import android.app.ActivityManager;
import android.app.AlertDialog;
import android.content.res.Configuration;
import android.content.Context;
import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.content.pm.ConfigurationInfo;
import android.content.DialogInterface;
import android.os.Bundle;
import android.os.Environment;
import android.os.Looper;
import android.text.method.MetaKeyKeyListener;
import android.text.InputType;
import android.util.Log;
import android.util.DisplayMetrics;
import android.view.ContextMenu;
import android.view.ContextMenu.ContextMenuInfo;
import android.view.View;
import android.view.KeyEvent;
import android.view.KeyCharacterMap;
import android.view.Menu;
import android.view.MenuItem;
import android.view.Window;
import android.view.WindowManager;
import android.view.WindowManager.LayoutParams;
import android.view.inputmethod.InputMethodManager;

class OffscreenViewFactory
{
    public static final String TAG = "Grym/OffscreenView";

    // This block is only to walkaround limitations in jcGeneric.
    private String default_class_name_ = "OffscreenWebView";
    private String default_object_name_ = "MyWebView";
    private int default_texture_id_ = 0;
    private int default_texture_width_ = 512;
    private int default_texture_height_ = 512;
    private long default_native_ptr_ = 0;
    public void SetClassName(String name) { default_class_name_ = name; }
    public void SetObjectName(String name) { default_object_name_ = name; }
    public void SetTexture(int tex) { default_texture_id_ = tex; }
    public void SetTextureWidth(int w) { default_texture_width_ = w; }
    public void SetTextureHeight(int h) { default_texture_height_ = h; }
    public void SetNativePtr(long ptr) { default_native_ptr_ = ptr; }
    public OffscreenView DoCreateView()
    {
        Log.i(TAG, "DoCreateView tid="+Thread.currentThread().getId()+" class=\""+default_class_name_+"\", object=\""+default_object_name_+
            "\", texid="+default_texture_id_+", texsize="+
            default_texture_width_+"x"+default_texture_height_);
        return CreateOffscreenView(
            default_class_name_,
            default_object_name_,
            default_native_ptr_,
            default_texture_id_,
            default_texture_width_,
            default_texture_height_);
    }

    OffscreenViewFactory()
    {
    }

    public static String Test()
    {
        return "TestString";
    }

    public static OffscreenView CreateOffscreenView(String classname, String objectname, long nativeptr, int gltextureid, int width, int height)
    {
        Log.i(TAG, "CreateOffscreenView");
        // TODO: Use classname to create various views
        return new OffscreenWebView(objectname, nativeptr, gltextureid, width, height);
    }

}
