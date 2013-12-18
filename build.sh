#!/bin/sh

# This script will build Qt library, Java code, assemble
# apk file and install to Android device or emulator
# (must be connected / running).

cd QtSources
#PATH="/data/local/qt/bin:$PATH" 
qmake && make || exit 1
cd ..

android update project -p . -t 3 --target "android-15"

ant debug install
