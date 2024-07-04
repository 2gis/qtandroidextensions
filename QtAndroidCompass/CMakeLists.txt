set(CMAKE_AUTOMOC OFF)

set(MODULE_NAME QtAndroidCompass)

set(SRC_LIST
    QAndroidCompass.cpp
    QAndroidCompass.h
)

if (ANDROID)
    qt5_wrap_cpp(QtAndroidCompass_hdr_moc QAndroidCompass.h)
    add_library(${MODULE_NAME} STATIC ${SRC_LIST} ${QtAndroidCompass_hdr_moc})
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

