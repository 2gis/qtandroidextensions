set(CMAKE_AUTOMOC OFF)

set(MODULE_NAME QtAndroidRuStoreHelper)


set(SRC_LIST
    QAndroidRuStoreInAppUpdate.cpp
    QAndroidRuStoreInAppUpdate.h
)

if (ANDROID)
    qt5_wrap_cpp(
        QtAndroidRuStoreHelper_hdr_moc
        QAndroidRuStoreInAppUpdate.h
    )

    add_library(${MODULE_NAME} STATIC ${SRC_LIST} ${QtAndroidRuStoreHelper_hdr_moc})
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

