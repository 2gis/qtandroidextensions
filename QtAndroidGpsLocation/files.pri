
android-g++ {

INCLUDEPATH += $$PWD

HEADERS += \
	$$PWD/LocationManagerProvidersListener.h \
	$$PWD/QAndroidGooglePlayServiceLocationProvider.h \
	$$PWD/QGeoPositionInfoSourceAndroidGPS.h

SOURCES += \
	$$PWD/LocationManagerProvidersListener.cpp \
	$$PWD/QAndroidGooglePlayServiceLocationProvider.cpp \
	$$PWD/QGeoPositionInfoSourceAndroidGPS.cpp

}
