QT += qml quick androidextras opengl

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android-sources
ANDROID_PACKAGE = ru.dublgis.offscreenview
ANDROID_MINIMUM_VERSION = 15
ANDROID_TARGET_VERSION = 15
ANDROID_APP_NAME = QML WebView

# DEFINES += QJNIHELPERS_VERBOSE_LOG

LIBS += -ljnigraphics

# Additional import path used to resolve QML modules in Creator's code model
QML_IMPORT_PATH =

# If your application uses the Qt Mobility libraries, uncomment the following
# lines and add the respective components to the MOBILITY variable.
# CONFIG += mobility
# MOBILITY +=

# The .cpp file which was generated for your project. Feel free to hack it.
SOURCES += \
    main.cpp \
    ../../QtOffscreenViews/QAndroidOffscreenView.cpp \
    ../../QtOffscreenViews/QAndroidOffscreenWebView.cpp \
    ../../QtOffscreenViews/QOpenGLTextureHolder.cpp \
	../../QJniHelpers/QAndroidQPAPluginGap.cpp \
    ../../QtOffscreenViews/QAndroidOffscreenEditText.cpp \
	../../QJniHelpers/QJniHelpers.cpp \
    ../../QtOffscreenViews/QAndroidJniImagePair.cpp \
    ../../QtOffscreenViews/QQuickViews/QQuickAndroidOffscreenView.cpp \
    ../../QtOffscreenViews/QQuickViews/QQuickOffscreenEditText.cpp \
    ../../QtOffscreenViews/QQuickViews/QQuickOffscreenWebView.cpp \
    ../../QtOffscreenViews/QApplicationActivityObserver.cpp
HEADERS += \
    ../../QtOffscreenViews/QAndroidOffscreenView.h \
    ../../QtOffscreenViews/QAndroidOffscreenWebView.h \
    ../../QtOffscreenViews/QOpenGLTextureHolder.h \
	../../QJniHelpers/QAndroidQPAPluginGap.h \
    ../../QtOffscreenViews/QAndroidOffscreenEditText.h \
	../../QJniHelpers/QJniHelpers.h \
    ../../QtOffscreenViews/QAndroidJniImagePair.h \
    ../../QtOffscreenViews/QQuickViews/QQuickAndroidOffscreenView.h \
    ../../QtOffscreenViews/QQuickViews/QQuickOffscreenEditText.h \
    ../../QtOffscreenViews/QQuickViews/QQuickOffscreenWebView.h \
    ../../QtOffscreenViews/QApplicationActivityObserver.h

# Please do not modify the following two lines. Required for deployment.
include(qtquick2applicationviewer/qtquick2applicationviewer.pri)
qtcAddDeployment()

OTHER_FILES += \
    main.qml \
    android-sources/src/ru/dublgis/offscreenview/OffscreenView.java \
    android-sources/src/ru/dublgis/offscreenview/OffscreenWebView.java

RESOURCES += AndroidWebView.qrc

INCLUDEPATH += ../../QJniHelpers ../../QtOffscreenViews
#LIBS += -L../../QtOffscreenViews -lQtOffscreenViews -L../../QJniHelpers -lQJniHelpers
#POST_TARGETDEPS += ../../QtOffscreenViews/libQtOffscreenViews.a ../../QJniHelpers/libQJniHelpers.a

