TEMPLATE = lib
CONFIG += staticlib
QT += core

# DEFINES += JNIUTILS_VERBOSE_LOG

android-g++ {

HEADERS += \
    JniEnvPtr.h \
    JclassPtr.h \
    jcGeneric.h \
    stdafx.h

SOURCES += \
    JniEnvPtr.cpp \
    JclassPtr.cpp \
    jcGeneric.cpp
}
