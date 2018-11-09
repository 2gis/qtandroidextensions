set(MODULE_NAME QtAndroidLocation)

set(SRC_LIST
    PositionInfoConvertor.cpp
    PositionInfoConvertor.h
    QAndroidGmsLocationProvider.cpp
    QAndroidGmsLocationProvider.h
    QAndroidPassiveGeoPosition.cpp
    QAndroidPassiveGeoPosition.h
    QGeoPositionInfoSourceAndroidGms.cpp
    QGeoPositionInfoSourceAndroidGms.h
    QGeoPositionInfoSourceAndroidPassive.cpp
    QGeoPositionInfoSourceAndroidPassive.h
    QLocationManagerProvidersListener.cpp
    QLocationManagerProvidersListener.h
    QNmeaListener.cpp
    QNmeaListener.h
)

if (ANDROID)
    add_library(${MODULE_NAME} STATIC ${SRC_LIST})
    add_library(${PROJECT_NAME}::${MODULE_NAME} ALIAS ${MODULE_NAME})

    target_link_libraries(${MODULE_NAME}
        PUBLIC
            Qt5::Positioning
            qtandroidextensions::QtJniHelpers
    )

    target_include_directories(${MODULE_NAME}
        PUBLIC
            ${PROJECT_SOURCE_DIR}
    )
endif(ANDROID)

