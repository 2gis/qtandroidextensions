android-g++ {

    INCLUDEPATH += $$PWD
    QT += androidextras

    HEADERS += \
        $$PWD/QJniHelpers.h \
        $$PWD/QAndroidQPAPluginGap.h \
        $$PWD/IJniObjectLinker.h \
        $$PWD/TJniObjectLinker.h \

    SOURCES += \
        $$PWD/QJniHelpers.cpp \
        $$PWD/QAndroidQPAPluginGap.cpp \
}
