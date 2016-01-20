
android-g++ {

INCLUDEPATH += $$PWD

HEADERS += \
	$$PWD/QLocationManagerProvidersListener.h \
	$$PWD/QAndroidGooglePlayServiceLocationProvider.h \
	$$PWD/QGeoPositionInfoSourceAndroidGPS.h \
	$$PWD/QNmeaListener.h

SOURCES += \
	$$PWD/QLocationManagerProvidersListener.cpp \
	$$PWD/QAndroidGooglePlayServiceLocationProvider.cpp \
	$$PWD/QGeoPositionInfoSourceAndroidGPS.cpp \
	$$PWD/QNmeaListener.cpp

}
