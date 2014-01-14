TEMPLATE = lib
CONFIG += staticlib
QT += core gui opengl

INCLUDEPATH += ../JNIUtils

HEADERS += \
    QOpenGLTextureHolder.h \
    QAndroidOffscreenView.h \
    QAndroidOffscreenWebView.h

SOURCES += \
    QOpenGLTextureHolder.cpp \
    QAndroidOffscreenView.cpp \
    QAndroidOffscreenWebView.cpp

OTHER_FILES += \
    ru.dublgis.offscreenview/OffscreenViewFactory.java \
    ru.dublgis.offscreenview/OffscreenView.java \
    ru.dublgis.offscreenview/OffscreenWebView.java
