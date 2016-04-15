
android-g++ {

INCLUDEPATH += $$PWD

HEADERS += \
	$$PWD/QLocationManagerProvidersListener.h \
	$$PWD/QAndroidGmsLocationProvider.h \
	$$PWD/QGeoPositionInfoSourceAndroidGms.h \
	$$PWD/QNmeaListener.h

SOURCES += \
	$$PWD/QLocationManagerProvidersListener.cpp \
	$$PWD/QAndroidGmsLocationProvider.cpp \
	$$PWD/QGeoPositionInfoSourceAndroidGms.cpp \
	$$PWD/QNmeaListener.cpp

}
