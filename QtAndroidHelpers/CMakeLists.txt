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
    add_library(${MODULE_NAME} STATIC ${SRC_LIST})
    add_library(${PROJECT_NAME}::${MODULE_NAME} ALIAS ${MODULE_NAME})

    target_link_libraries(${MODULE_NAME}
        PUBLIC
            qtandroidextensions::QtJniHelpers
    )

    target_include_directories(${MODULE_NAME}
        PUBLIC
            ${PROJECT_SOURCE_DIR}
    )
endif(ANDROID)

