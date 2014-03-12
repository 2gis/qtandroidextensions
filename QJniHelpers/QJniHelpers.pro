TEMPLATE = lib
CONFIG += staticlib
QT += core

# DEFINES += QJNIHELPERS_VERBOSE_LOG

android-g++ {

    CONFIG(android_autolink_grym): DEFINES += QJNIHELPERS_GRYM
}

include(files.pri)