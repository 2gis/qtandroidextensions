set(CMAKE_AUTOMOC OFF)

set(MODULE_NAME QtAndroidAccelerometer)

set(SRC_LIST
    QAndroidAccelerometer.cpp
    QAndroidAccelerometer.h
)

if (ANDROID)
    qt5_wrap_cpp(QtAndroidAccelerometer_hdr_moc QAndroidAccelerometer.h)
    add_library(${MODULE_NAME} STATIC ${SRC_LIST} ${QtAndroidAccelerometer_hdr_moc})
    add_library(${PROJECT_NAME}::${MODULE_NAME} ALIAS ${MODULE_NAME})

    target_link_libraries(${MODULE_NAME}
        PUBLIC
            Qt5::Core
            qtandroidextensions::QtAndroidSensorManager
    )

    target_include_directories(${MODULE_NAME}
        PUBLIC
            ${PROJECT_SOURCE_DIR}
    )
endif(ANDROID)

