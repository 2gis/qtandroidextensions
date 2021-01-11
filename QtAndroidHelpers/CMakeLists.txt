set(CMAKE_AUTOMOC OFF)

set(MODULE_NAME QtAndroidHelpers)

set(SRC_LIST
    QAndroidAction.cpp
    QAndroidAction.h
    QAndroidConfiguration.cpp
    QAndroidConfiguration.h
    QAndroidDesktopUtils.cpp
    QAndroidDesktopUtils.h
    QAndroidDialog.cpp
    QAndroidDialog.h
    QAndroidDisplayMetrics.cpp
    QAndroidDisplayMetrics.h
    QAndroidFilePaths.cpp
    QAndroidFilePaths.h
    QAndroidPartialWakeLocker.cpp
    QAndroidPartialWakeLocker.h
    QAndroidScreenLayoutHandler.cpp
    QAndroidScreenLayoutHandler.h
    QAndroidScreenLocker.cpp
    QAndroidScreenLocker.h
    QAndroidScreenOrientation.cpp
    QAndroidScreenOrientation.h
    QAndroidSharedPreferences.cpp
    QAndroidSharedPreferences.h
    QAndroidSpeechRecognizer.cpp
    QAndroidSpeechRecognizer.h
    QAndroidStorages.cpp
    QAndroidStorages.h
    QAndroidStorages_p.h
    QAndroidToast.cpp
    QAndroidToast.h
    QAndroidVibrator.cpp
    QAndroidVibrator.h
    QAndroidScreenInteractiveObserver.cpp
    QAndroidScreenInteractiveObserver.h
    QAndroidWiFiLocker.cpp
    QAndroidWiFiLocker.h
    Mobility/CellData.cpp
    Mobility/CellData.h
    Mobility/QAndroidBatteryDataProvider.cpp
    Mobility/QAndroidBatteryDataProvider.h
    Mobility/QAndroidCellDataProvider.cpp
    Mobility/QAndroidCellDataProvider.h
    Mobility/QAndroidWifiDataProvider.cpp
    Mobility/QAndroidWifiDataProvider.h
    Mobility/WifiData.cpp
    Mobility/WifiData.h
    QLocks/QLockBase.h
    QLocks/QLock.cpp
    QLocks/QLockedObjectBase_p.h
    QLocks/QLockedObject.cpp
    QLocks/QLockedObject.h
    QLocks/QLockHandler.cpp
    QLocks/QLockHandler_p.h
    QLocks/QLock_p.h
)

if (ANDROID)
    qt5_wrap_cpp(
        QtAndroidHelpers_hdr_moc
        QAndroidAction.h
        QAndroidConfiguration.h
        QAndroidDialog.h
        QAndroidDisplayMetrics.h
        QAndroidScreenLayoutHandler.h
        QAndroidScreenOrientation.h
        QAndroidSharedPreferences.h
        QAndroidSpeechRecognizer.h
        QAndroidVibrator.h
        QAndroidScreenInteractiveObserver.h
        Mobility/QAndroidBatteryDataProvider.h
        Mobility/QAndroidCellDataProvider.h
        Mobility/QAndroidWifiDataProvider.h
        QLocks/QLock_p.h
    )
    add_library(${MODULE_NAME} STATIC ${SRC_LIST} ${QtAndroidHelpers_hdr_moc})
    add_library(${PROJECT_NAME}::${MODULE_NAME} ALIAS ${MODULE_NAME})

    target_link_libraries(${MODULE_NAME}
        PUBLIC
            Qt5::Core
            qtandroidextensions::QtJniHelpers
    )

    target_include_directories(${MODULE_NAME}
        PUBLIC
            ${PROJECT_SOURCE_DIR}
    )
endif(ANDROID)

