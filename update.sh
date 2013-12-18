#!/bin/sh

INSTDIR=/data/local/qt
STRIP=`which arm-eabi-strip`

if [ ! -x "$STRIP" ] ; then
  echo "WARNING: arm-eabi-strip is not in PATH, the libs won't be stripped!" ;
  STRIP=
else
  echo "Using strip: $STRIP";
fi

mkdir -p libs/armeabi || exit 1

for LIB in QtCore QtGui QAndroidCore QtOpenGL QwpLiteApi9 ; do
  cp -vfL "$INSTDIR/lib/lib$LIB.so" libs/armeabi || exit 1
  if [ "$STRIP" != "" ] ; then
    "$STRIP" --strip-all libs/armeabi/lib$LIB.so ;
  fi ;
done
