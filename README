2GIS Extensions for Qt on Android


Developers:
    Sergey A. Galin <sergey.galin@gmail.com>
    Vyacheslav O. Koscheev <vok1980@gmail.com>
    Ivan Ryabchenko <i.ryabchenko@2gis.ru>
    Ivan Avdeev <marflon@gmail.com>
    Alexander Saytgalin <pomidir@gmail.com>

Also uses code by:
    BogDan Vatra <bogdan@kde.org>


Please note that the modules are published under different open-source
licenses. Most part is distributed under BSD License, parts are under GNU LGPL
License (please refer to the headers in the source files).

This library supports Qt 5 and (partially) Qt 4.8 port to Android made by 2GIS.

The library is currently distributed only as a set of C++ and Java source files;
you have to create build projects or include the sources into your project.


CONTENTS


QJniHelpers
===============================================================================

Qt/C++ JNI helper classes.
Classes to get Android Activity object under Qt 4.8 and Qt 5.


QtOffscreenViews
===============================================================================

This library allows to embed native Android controls right into your Qt
application, whether it uses QML, QWidget or QGraphicsWidget UI.

Currently implemented Android controls are:

- TextEdit with full support for all text editing features like edit bar,
  selection markers and etc.

- WebView (works best for embedding static content; does not support video
  playback and has performance issues with dynamic content).


QtAndroidHelpers
===============================================================================

This part contains access to Android system things in a more Android-specific
manner than the cross-platform Qt can do.

- Device and screen configuration
- Filesystem information (standard directories, external cards and etc.)
- Android system message dialogs
- Notification on screen relayout (e.g. opening software keyboard)
- Access to Shared Preferences
- Toasts
- Lockers to keep WiFi and/or screen on


QtAndroidAssets
===============================================================================

A library which allows to use assets from a non-UI application, e.g. Android
background service.


QtAndroidCompass
===============================================================================

Homebrewn access to Android compass which addresses some problems with Qt 5
implementation.


QtAndroidLocation
===============================================================================

Homebrewn location provider which uses Google Fused location (when it is
available) and also standard Android positioning methods. It is faster and more
reliable than the location provider included in Qt.

