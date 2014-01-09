QT += core gui opengl

HEADERS += \
    GrymQtAndroidViewGraphicsProxy.h \
    QOpenGLTextureHolder.h

SOURCES = \
	main.cpp \
    GrymQtAndroidViewGraphicsProxy.cpp \
    QOpenGLTextureHolder.cpp

RESOURCES = resources.qrc

INCLUDEPATH += JNIUtils

android-g++ {
    # Reduce binary size by not exporting all symbols by default.
    # This causes stripping out symbols which are neither explicitly
    # exported nor used inside of the application.
    QMAKE_CXXFLAGS += -fvisibility=hidden
    CONFIG += dll
    TARGET = /../../libs/armeabi/libandroidwebviewdemo
}

include("JNIUtils/JNIUtils.pri")

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












