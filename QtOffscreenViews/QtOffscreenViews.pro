TEMPLATE = lib
CONFIG += staticlib
QT += core gui opengl

include("androidjnigraphics.pri")

INCLUDEPATH += ../QJniHelpers

HEADERS += \
    QOpenGLTextureHolder.h \
    QAndroidOffscreenView.h \
    QAndroidOffscreenWebView.h \
    QAndroidOffscreenEditText.h \
    QAndroidJniImagePair.h

SOURCES += \
    QOpenGLTextureHolder.cpp \
    QAndroidOffscreenView.cpp \
    QAndroidOffscreenWebView.cpp \
    QAndroidOffscreenEditText.cpp \
    QAndroidJniImagePair.cpp

OTHER_FILES += \
    ru.dublgis.offscreenview/OffscreenViewFactory.java \
    ru.dublgis.offscreenview/OffscreenView.java \
    ru.dublgis.offscreenview/OffscreenWebView.java \
    ru.dublgis.offscreenview/OffscreenEditText.java

CONFIG(android_autolink_grym): DEFINES += QTOFFSCREENVIEWS_GRYM
