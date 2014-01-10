TEMPLATE = lib
CONFIG += staticlib
QT += core gui opengl

INCLUDEPATH += ../JNIUtils

HEADERS += QOpenGLTextureHolder.h QAndroidOffscreenView.h
SOURCES += QOpenGLTextureHolder.cpp QAndroidOffscreenView.cpp
