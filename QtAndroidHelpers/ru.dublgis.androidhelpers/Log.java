package ru.dublgis.androidhelpers;

import java.lang.reflect.Method;


public final class Log {
    private static final String TAG = "Log";

    private static Method sMethodV2 = null;
    private static Method sMethodV3 = null;
    private static Method sMethodD2 = null;
    private static Method sMethodD3 = null;
    private static Method sMethodI2 = null;
    private static Method sMethodI3 = null;
    private static Method sMethodW2 = null;
    private static Method sMethodW3 = null;
    private static Method sMethodW4 = null;
    private static Method sMethodE2 = null;
    private static Method sMethodE3 = null;

    private Log() {
    }

    static {
        setLogger(android.util.Log.class);
    }

    public static void setLogger(final Class<?> clazz) {
        try {
            final Method MethodV2 = clazz.getMethod("v", String.class, String.class);
            final Method MethodV3 = clazz.getMethod("v", String.class, String.class, Throwable.class);
            final Method MethodD2 = clazz.getMethod("d", String.class, String.class);
            final Method MethodD3 = clazz.getMethod("d", String.class, String.class, Throwable.class);
            final Method MethodI2 = clazz.getMethod("i", String.class, String.class);
            final Method MethodI3 = clazz.getMethod("i", String.class, String.class, Throwable.class);
            final Method MethodW2 = clazz.getMethod("w", String.class, String.class);
            final Method MethodW3 = clazz.getMethod("w", String.class, String.class, Throwable.class);
            final Method MethodW4 = clazz.getMethod("w", String.class, Throwable.class);
            final Method MethodE2 = clazz.getMethod("e", String.class, String.class);
            final Method MethodE3 = clazz.getMethod("e", String.class, String.class, Throwable.class);

            sMethodV2 = MethodV2;
            sMethodV3 = MethodV3;
            sMethodD2 = MethodD2;
            sMethodD3 = MethodD3;
            sMethodI2 = MethodI2;
            sMethodI3 = MethodI3;
            sMethodW2 = MethodW2;
            sMethodW3 = MethodW3;
            sMethodW4 = MethodW4;
            sMethodE2 = MethodE2;
            sMethodE3 = MethodE3;
        } catch (final Throwable tr) {
            android.util.Log.e(TAG, "setLogger failure for class " + clazz.toString(), tr);
        }
    }

    private static void call(Method method, Object... params) {
        try {
            method.invoke(null, params);
        } catch (final Throwable tr) {
            android.util.Log.e(TAG, "invoke failure", tr);
        }
    }

    public static void v(String tag, String msg) {
        call(sMethodV2, tag, msg);
    }

    public static void v(String tag, String msg, Throwable tr) {
        call(sMethodV3, tag, msg, tr);
    }

    public static void d(String tag, String msg) {
        call(sMethodD2, tag, msg);
    }

    public static void d(String tag, String msg, Throwable tr) {
        call(sMethodD3, tag, msg, tr);
    }

    public static void i(String tag, String msg) {
        call(sMethodI2, tag, msg);
    }

    public static void i(String tag, String msg, Throwable tr) {
        call(sMethodI3, tag, msg, tr);
    }

    public static void w(String tag, String msg) {
        call(sMethodW2, tag, msg);
    }

    public static void w(String tag, String msg, Throwable tr) {
        call(sMethodW3, tag, msg, tr);
    }

    public static void w(String tag, Throwable tr) {
        call(sMethodW4, tag, tr);
    }

    public static void e(String tag, String msg) {
        call(sMethodE2, tag, msg);
    }

    public static void e(String tag, String msg, Throwable tr) {
        call(sMethodE3, tag, msg, tr);
    }
}
