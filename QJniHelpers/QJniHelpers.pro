TEMPLATE = lib
CONFIG += staticlib
QT += core

# DEFINES += QJNIHELPERS_VERBOSE_LOG

android-g++ {

    CONFIG(android_autolink_grym): DEFINES += QJNIHELPERS_GRYM

    HEADERS += \
        QJniHelpers.h \
        QAndroidQPAPluginGap.h \

    SOURCES += \
        QJniHelpers.cpp \
        QAndroidQPAPluginGap.cpp
}
