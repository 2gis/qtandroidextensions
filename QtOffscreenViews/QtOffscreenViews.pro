TEMPLATE = lib
CONFIG += staticlib
QT += core gui opengl

include("androidjnigraphics.pri")

INCLUDEPATH += ../QJniHelpers

CONFIG(android_autolink_grym): DEFINES += QTOFFSCREENVIEWS_GRYM

include(files.pri)