QT += core gui
SOURCES = main.cpp
RESOURCES = animatedtiles.qrc

android-g++ {
    # Reduce binary size by not exporting all symbols by default.
    # This causes stripping out symbols which are neither explicitly
    # exported nor used inside of the application.
    QMAKE_CXXFLAGS += -fvisibility=hidden

    CONFIG += dll

    TARGET = /../../libs/armeabi/libanimatedtiles
}
