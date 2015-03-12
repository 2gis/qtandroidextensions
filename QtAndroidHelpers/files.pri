
android-g++ {

INCLUDEPATH += $$PWD

HEADERS += \
    $$PWD/QAndroidConfiguration.h \
    $$PWD/QAndroidDialog.h \
    $$PWD/QAndroidDisplayMetrics.h \
    $$PWD/QAndroidFilePaths.h \
    $$PWD/QAndroidScreenOrientation.h \
    $$PWD/QAndroidScreenLayoutHandler.h \
    $$PWD/QAndroidToast.h \
    $$PWD/QAndroidDesktopUtils.h

SOURCES += \
    $$PWD/QAndroidConfiguration.cpp \
    $$PWD/QAndroidDialog.cpp \
    $$PWD/QAndroidDisplayMetrics.cpp \
    $$PWD/QAndroidFilePaths.cpp \
    $$PWD/QAndroidScreenOrientation.cpp \
    $$PWD/QAndroidScreenLayoutHandler.cpp \
    $$PWD/QAndroidToast.cpp \
    $$PWD/QAndroidDesktopUtils.cpp
}
