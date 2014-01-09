#!/bin/sh

SDK="$DGIS_ALH_SOURCE"
SRCJAVA="$SDK/src/plugins/platforms/android/grym/java/src"
INSTDIR=$QT_INSTALL_DIR
JAVA_DIRS="lite core util"
STRIP="$DGIS_ANDROID_TOOLS_BIN/$DGIS_ANDROID_TOOLS_PREFIX-strip"


if [ ! -x "$STRIP" ] ; then
  echo "WARNING: arm-eabi-strip is not in PATH, the libs won't be stripped!" ;
  STRIP=
else
  echo "Using strip: $STRIP";
fi

mkdir -p libs/armeabi || exit 1

for LIB in QtCore QtGui QAndroidCore QtOpenGL QwpLiteApi9 QtOpenGL; do
  cp -vfL "$INSTDIR/lib/lib$LIB.so" libs/armeabi || exit 1
  if [ "$STRIP" != "" ] ; then
    "$STRIP" --strip-all libs/armeabi/lib$LIB.so ;
  fi ;
done

echo "Creating Qt Java symlinks..."
mkdir -p "src/org/qt"
for J in $JAVA_DIRS ; do
  rm -f "src/org/qt/$J"
  ln -sfv "$SRCJAVA/org/qt/$J" "src/org/qt/$J" || exit 1 ;
done

android update project -p . -t 8 --target "android-19" || exit 1

