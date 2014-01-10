TEMPLATE = lib
CONFIG += staticlib
QT += core

android-g++ {

HEADERS += \
	JniEnvPtr.h \
	debug.h \
	JclassPtr.h \
	jcGeneric.h \
	stdafx.h

SOURCES += \
	JniEnvPtr.cpp \
	JclassPtr.cpp \
	jcGeneric.cpp
}
