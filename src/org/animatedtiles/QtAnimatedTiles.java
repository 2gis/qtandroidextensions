package org.animatedtiles;

import org.qt.core.QtLibraryLoader;
import org.qt.lite.QtActivity;

public class QtAnimatedTiles extends QtActivity
{
    public QtAnimatedTiles()
    {
        super("animatedtiles"); // Name of the library containing your Qt application
                                // (without "lib" and ".so")

        // You could add more libraries here, e.g.:
        // addLibrary("QtNetwork");

        // QwpLiteApi8 is the default plugin for now, you can set another plugin if necessary.
        // Please note that plugins with GL support require QtXml, QtSvg and QtOpenGl libs.
        // setPlugin("QwpLiteApi8");

        // Prevent OS from unloading the application just because of its inactivity for some time.
        // It is recommended to use this option and, if necessary, implement serializing current
        // state & exiting from inactive application on the native side of the force.
        setKeepaliveService(true);
        QtLibraryLoader.addLibrary("QtOpenGL");
        setEnableOpenGlModeFor(2, 0, 4);
    }
}

