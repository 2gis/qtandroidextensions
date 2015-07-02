
android-g++ {

INCLUDEPATH += $$PWD

HEADERS += \
    $$PWD/QSharedPreferences.h \
    $$PWD/QAndroidScreenLocker.h \
    $$PWD/QAndroidWiFiLocker.h \
    $$PWD/QLocks/QLockBase.h  \
    $$PWD/QLocks/QLockedObjectBase_p.h  \
    $$PWD/QLocks/QLockedObject.h  \
    $$PWD/QLocks/QLockHandler_p.h  \
    $$PWD/QLocks/QLock_p.h \
    $$PWD/QAndroidConfiguration.h \
    $$PWD/QAndroidDialog.h \
    $$PWD/QAndroidDisplayMetrics.h \
    $$PWD/QAndroidFilePaths.h \
    $$PWD/QAndroidScreenOrientation.h \
    $$PWD/QAndroidScreenLayoutHandler.h \
    $$PWD/QAndroidToast.h \
    $$PWD/QAndroidDesktopUtils.h \
    $$PWD/QAndroidStorages_.h \
    $$PWD/QAndroidStorages_p.h

SOURCES += \
    $$PWD/QSharedPreferences.cpp \
    $$PWD/QAndroidScreenLocker.cpp \
    $$PWD/QAndroidWiFiLocker.cpp \
    $$PWD/QLocks/QLock.cpp  \
    $$PWD/QLocks/QLockedObject.cpp  \
    $$PWD/QLocks/QLockHandler.cpp \
    $$PWD/QAndroidConfiguration.cpp \
    $$PWD/QAndroidDialog.cpp \
    $$PWD/QAndroidDisplayMetrics.cpp \
    $$PWD/QAndroidFilePaths.cpp \
    $$PWD/QAndroidScreenOrientation.cpp \
    $$PWD/QAndroidScreenLayoutHandler.cpp \
    $$PWD/QAndroidToast.cpp \
    $$PWD/QAndroidDesktopUtils.cpp \
    $$PWD/QAndroidStorages.cpp
}
