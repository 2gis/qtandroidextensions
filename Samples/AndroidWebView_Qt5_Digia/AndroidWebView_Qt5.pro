QT += qml quick androidextras opengl

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android-sources
ANDROID_PACKAGE = ru.dublgis.offscreenview
ANDROID_MINIMUM_VERSION = 15
ANDROID_TARGET_VERSION = 15
ANDROID_APP_NAME = QML WebView

# Add more folders to ship with the application, here
folder_01.source = qml/AndroidWebView_Qt5
folder_01.target = qml
DEPLOYMENTFOLDERS = folder_01

# Additional import path used to resolve QML modules in Creator's code model
QML_IMPORT_PATH =

# If your application uses the Qt Mobility libraries, uncomment the following
# lines and add the respective components to the MOBILITY variable.
# CONFIG += mobility
# MOBILITY +=

# The .cpp file which was generated for your project. Feel free to hack it.
SOURCES += \
    main.cpp \
    fboinsgrenderer.cpp \
    ../../QtOffscreenViews/QAndroidOffscreenView.cpp \
    ../../QtOffscreenViews/QAndroidOffscreenWebView.cpp \
    ../../QtOffscreenViews/QAndroidQPAPluginGap.cpp \
    ../../QtOffscreenViews/QOpenGLTextureHolder.cpp \
    ../../JNIUtils/jcGeneric.cpp \
    ../../JNIUtils/JclassPtr.cpp \
    ../../JNIUtils/JniEnvPtr.cpp
HEADERS += \
    fboinsgrenderer.h \
    ../../QtOffscreenViews/QAndroidOffscreenView.h \
    ../../QtOffscreenViews/QAndroidOffscreenWebView.h \
    ../../QtOffscreenViews/QAndroidQPAPluginGap.h \
    ../../QtOffscreenViews/QOpenGLTextureHolder.h \
    ../../JNIUtils/jcGeneric.h \
    ../../JNIUtils/JclassPtr.h \
    ../../JNIUtils/JniEnvPtr.h \
    ../../JNIUtils/stdafx.h

# Installation path
# target.path =

# Please do not modify the following two lines. Required for deployment.
include(qtquick2applicationviewer/qtquick2applicationviewer.pri)
qtcAddDeployment()

OTHER_FILES += \
    main.qml \
    android-sources/src/ru/dublgis/offscreenview/OffscreenView.java \
    android-sources/src/ru/dublgis/offscreenview/OffscreenWebView.java

RESOURCES += AndroidWebView.qrc

INCLUDEPATH += ../../JNIUtils ../../QtOffscreenViews
LIBS += -L../../QtOffscreenViews -lQtOffscreenViews -L../../JNIUtils -lJNIUtils
POST_TARGETDEPS += ../../QtOffscreenViews/libQtOffscreenViews.a ../../JNIUtils/libJNIUtils.a

