#!/bin/sh

# This script will build Qt library, Java code, assemble
# apk file and install to Android device or emulator
# (must be connected / running).

#PATH="/data/local/qt/bin:$PATH"

MYDIR="$PWD"

cd "../../QtOffscreenViews" || exit 1
qmake || exit 1
make || exit 1
cd "$MYDIR" || exit 1

cd QtSources || exit 1
qmake || exit 1
make || exit 1
cd "$MYDIR" || exit 1

ant debug install || exit 1

echo "Starting app..."
adb shell am start -n ru.dublgis.androidwebviewdemo/ru.dublgis.androidwebviewdemo.AndroidWebViewDemo

