
# This file configures libs & include paths for various all Android NDK's and API levels

CONFIG(android_ndk4) {

    #
    # NDK4
    #

    # message("**** Android JNI graphics: using NDK4")
    CONFIG(android-8) | CONFIG(android-9){
        LIBS += -ljnigraphics -L$$ANDROID_PLATFORM_PATH_8/lib
        LIBS += -L$$QT_SOURCE_TREE/src/3rdparty/android/precompiled/android-8/arch-arm/lib
    }else{
        # API Level 4-5
        INCLUDEPATH += $$QT_SOURCE_TREE/src/plugins/platforms/android/common/native/include
        SOURCES += $$QT_SOURCE_TREE/src/plugins/platforms/android/common/native/graphics/jni/bitmap.cpp
        CONFIG(android-4){
            INCLUDEPATH += $$QT_SOURCE_TREE/src/3rdparty/android/precompiled/android-4/arch-arm/include
            INCLUDEPATH += $$QT_SOURCE_TREE/src/3rdparty/android/precompiled/android-4/arch-arm/include/core
            LIBS += -landroid_runtime -lsgl -L$$ANDROID_PLATFORM_PATH_4/lib \
                -L$$QT_SOURCE_TREE/src/3rdparty/android/precompiled/android-4/arch-arm/lib
        }else{
            INCLUDEPATH += $$QT_SOURCE_TREE/src/3rdparty/android/precompiled/android-5/arch-arm/include
            INCLUDEPATH += $$QT_SOURCE_TREE/src/3rdparty/android/precompiled/android-5/arch-arm/include/core
            LIBS += -landroid_runtime -lskia -L$$ANDROID_PLATFORM_PATH_5/lib
        }
    }

} else {

    #
    # NDK5+
    #

    # message("**** Android JNI graphics: using NDK5+")
    CONFIG(android-4) | CONFIG(android-5) {
        #
        # Android 4 & 5
        #
        INCLUDEPATH += $$QT_SOURCE_TREE/src/plugins/platforms/android/common/native/include
        SOURCES += $$QT_SOURCE_TREE/src/plugins/platforms/android/common/native/graphics/jni/bitmap.cpp
        CONFIG(android-4){
            INCLUDEPATH += $$QT_SOURCE_TREE/src/3rdparty/android/precompiled/android-4/arch-arm/include
            INCLUDEPATH += $$QT_SOURCE_TREE/src/3rdparty/android/precompiled/android-4/arch-arm/include/core
            LIBS += -L$$QT_SOURCE_TREE/src/3rdparty/android/precompiled/android-4/arch-arm/lib -landroid_runtime -lsgl
        }

        CONFIG(android-5){
            INCLUDEPATH += $$QT_SOURCE_TREE/src/3rdparty/android/precompiled/android-5/arch-arm/include
            INCLUDEPATH += $$QT_SOURCE_TREE/src/3rdparty/android/precompiled/android-5/arch-arm/include/core
            LIBS += -L$$QT_SOURCE_TREE/src/3rdparty/android/precompiled/android-5/arch-arm/lib -landroid_runtime -lskia
        }

        # LIBS += -lutils
    }
    else {
        #
        # Android 8 & 9
        #
        LIBS += -ljnigraphics -L$$ANDROID_PLATFORM_PATH_8/lib
        LIBS += -L$$QT_SOURCE_TREE/src/3rdparty/android/precompiled/android-8/arch-arm/lib

        CONFIG(android-8) {
            # LIBS += -lutils
        } else {
            LIBS += -landroid
        }
    }

    # message("======================= $$LIBS =======================")
}
