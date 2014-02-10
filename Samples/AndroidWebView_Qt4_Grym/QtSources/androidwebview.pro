QT += core gui opengl

#HEADERS +=

SOURCES += \
    main.cpp

# Necessary for Bitmap-based mode of the offscreen views
include("../../../QtOffscreenViews/androidjnigraphics.pri")

RESOURCES = resources.qrc

INCLUDEPATH += ../../../QJniHelpers ../../../QtOffscreenViews
LIBS += -L../../../QtOffscreenViews -lQtOffscreenViews -L../../../QJniHelpers -lQJniHelpers
POST_TARGETDEPS += ../../../QtOffscreenViews/libQtOffscreenViews.a ../../../QJniHelpers/libQJniHelpers.a

android-g++ {
    # Reduce binary size by not exporting all symbols by default.
    # This causes stripping out symbols which are neither explicitly
    # exported nor used inside of the application.
    QMAKE_CXXFLAGS += -fvisibility=hidden
    CONFIG += dll
    TARGET = /../../libs/armeabi/libandroidwebviewdemo
}

OTHER_FILES += \
    images/Time-For-Lunch-2.jpg \
    images/tile.png \
    images/random.png \
    images/kinetic.png \
    images/figure8.png \
    images/ellipse.png \
    images/centered.png \
    ../build.sh \
    ../qtcreator.sh \
    ../update.sh \
    ../AndroidManifest.xml \
    images/kotik.png

