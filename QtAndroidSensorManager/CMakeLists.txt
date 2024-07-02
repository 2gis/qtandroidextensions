set(CMAKE_AUTOMOC OFF)

set(MODULE_NAME QtAndroidSensorManager)

set(SRC_LIST
    QAndroidSensorManager.cpp
    QAndroidSensorManager.h
)

if (ANDROID)
    qt5_wrap_cpp(QtAndroidSensorManager_hdr_moc QAndroidSensorManager.h)
    add_library(${MODULE_NAME} STATIC ${SRC_LIST} ${QtAndroidSensorManager_hdr_moc})
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

