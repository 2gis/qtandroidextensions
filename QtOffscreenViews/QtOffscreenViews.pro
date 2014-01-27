TEMPLATE = lib
CONFIG += staticlib
QT += core gui opengl

INCLUDEPATH += ../JNIUtils

HEADERS += \
    QOpenGLTextureHolder.h \
    QAndroidOffscreenView.h \
    QAndroidOffscreenWebView.h \
    QAndroidOffscreenEditText.h

SOURCES += \
    QOpenGLTextureHolder.cpp \
    QAndroidOffscreenView.cpp \
    QAndroidOffscreenWebView.cpp \
    QAndroidOffscreenEditText.cpp

OTHER_FILES += \
    ru.dublgis.offscreenview/OffscreenViewFactory.java \
    ru.dublgis.offscreenview/OffscreenView.java \
    ru.dublgis.offscreenview/OffscreenWebView.java \
    ru.dublgis.offscreenview/OffscreenEditText.java

CONFIG(android_autolink_grym): DEFINES += QTOFFSCREENVIEWS_GRYM
