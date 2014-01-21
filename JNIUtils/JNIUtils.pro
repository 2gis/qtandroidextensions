TEMPLATE = lib
CONFIG += staticlib
QT += core

# DEFINES += JNIUTILS_VERBOSE_LOG

android-g++ {

    CONFIG(android_autolink_grym): DEFINES += JNIUTILS_GRYM

    HEADERS += \
        JniEnvPtr.h \
        JclassPtr.h \
        jcGeneric.h \
        QAndroidQPAPluginGap.h \
        stdafx.h

    SOURCES += \
        JniEnvPtr.cpp \
        JclassPtr.cpp \
        jcGeneric.cpp \
        QAndroidQPAPluginGap.cpp
}
