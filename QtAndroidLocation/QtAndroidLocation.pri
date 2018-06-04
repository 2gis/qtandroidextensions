
android-g++ {

INCLUDEPATH += $$PWD

HEADERS += \
	$$PWD/QLocationManagerProvidersListener.h \
	$$PWD/QAndroidGmsLocationProvider.h \
	$$PWD/QGeoPositionInfoSourceAndroidGms.h \
	$$PWD/QGeoPositionInfoSourceAndroidPassive.h \
	$$PWD/PositionInfoConvertor.h \
	$$PWD/QNmeaListener.h \
	$$PWD/QAndroidPassiveGeoPosition.h

SOURCES += \
	$$PWD/QLocationManagerProvidersListener.cpp \
	$$PWD/QAndroidGmsLocationProvider.cpp \
	$$PWD/QGeoPositionInfoSourceAndroidGms.cpp \
	$$PWD/QGeoPositionInfoSourceAndroidPassive.cpp \
	$$PWD/PositionInfoConvertor.cpp \
	$$PWD/QNmeaListener.cpp \
	$$PWD/QAndroidPassiveGeoPosition.cpp
}
