package ru.dublgis.qjnihelpers;

public class ClassLoader
{
    static public void callJNIPreloadClass(final String classname)
    {
        (new ClassLoader()).nativeJNIPreloadClass(classname);
    }

    private native void nativeJNIPreloadClass(String classname);
}
