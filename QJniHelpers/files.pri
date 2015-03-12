android-g++ {

    INCLUDEPATH += $$PWD
    QT += androidextras

    HEADERS += \
        $$PWD/QJniHelpers.h \
        $$PWD/QAndroidQPAPluginGap.h \

    SOURCES += \
        $$PWD/QJniHelpers.cpp \
        $$PWD/QAndroidQPAPluginGap.cpp
}
