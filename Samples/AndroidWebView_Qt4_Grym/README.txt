
Sorry, this only a developer / proof of concept thing at the moment, so don't expect it to be friendly ;-)

Legal:
This example is based on QtAnimatedTiles example from Qt, so in case you want to use some of its code you can do it under terms of the GNU GPL license.


HOWTO:

1. The example is built and tested under Linux (Ubuntu) host ONLY. If you want you can provide instructions / code for other hosts.

2. This example currenlty supports only DoubleGIS "Grym" port of Qt 4.8 which can be downloaded from:
https://qt.gitorious.org/qt/grym-android-lighthouse/
Use "production-4.8" branch.

3. Also, the 1_run_qtcreator.sh script assumes that you have patches Qt creator installed into /bin:
https://qt.gitorious.org/qt-creator/grym-qt-creator/


To build:

- You must have Android NDK 9 and SDK.
- Compile and install Qt.
- Set the following environment variables:

DGIS_ALH_SOURCE - source code of Qt for Android
QT_INSTALL_DIR - install dir of Qt for Android
DGIS_ANDROID_TOOLS_BIN - path to bin of the toolchain you want to use, e.g.:: .../android-ndk-r9/toolchains/arm-linux-androideabi-4.8/prebuilt/linux-x86_64/bin
DGIS_ANDROID_TOOLS_PREFIX - e.g. arm-linux-androideabi

- Run 0_update_project.sh
- Run 2_build_and_run.sh

