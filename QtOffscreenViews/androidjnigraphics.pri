
# This file configures libs & include paths for various all Android NDK's and API levels

CONFIG(android_autolink_grym) {

    # Qt 4, Grym port
    LIBS += -ljnigraphics -L$$ANDROID_PLATFORM_PATH_8/lib
    LIBS += -L$$QT_SOURCE_TREE/src/3rdparty/android/precompiled/android-8/arch-arm/lib

} else {

    # Qt 5
    LIBS += -ljnigraphics
}

#!CONFIG(android-8) {
#    LIBS += -landroid
#}
