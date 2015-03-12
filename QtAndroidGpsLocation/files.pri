
android-g++ {

INCLUDEPATH += $$PWD

HEADERS += \
	$$PWD/QAndroidGooglePlayServiceLocationProvider.h \
	$$PWD/QGeoPositionInfoSourceAndroidGPS.h

SOURCES += \
	$$PWD/QAndroidGooglePlayServiceLocationProvider.cpp \
	$$PWD/QGeoPositionInfoSourceAndroidGPS.cpp

}
