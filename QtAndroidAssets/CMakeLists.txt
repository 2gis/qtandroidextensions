set(MODULE_NAME QtAndroidAssets)

set(SRC_LIST
    AndroidAssetsFileEngineHandler_p.cpp
    AndroidAssetsFileEngineHandler_p.h
    QAndroidAssetHandler.cpp
    QAndroidAssetHandler.h
)


if (ANDROID)
    find_library(ANDROID_LOG_LIB log REQIRED)
    find_library(ANDROID_LIB android REQIRED)
    add_library(${MODULE_NAME} STATIC ${SRC_LIST})
    add_library(${PROJECT_NAME}::${MODULE_NAME} ALIAS ${MODULE_NAME})

    target_link_libraries(${MODULE_NAME}
        PUBLIC
            Qt5::Core
            Qt5::Network
            Qt5::AndroidExtras
            qtandroidextensions::QtJniHelpers
            ${ANDROID_LOG_LIB}
            ${ANDROID_LIB}
    )

    target_include_directories(${MODULE_NAME}
        PUBLIC
            ${PROJECT_SOURCE_DIR}
    )

    # Это нужно для инклуда приватного хедера qabstractfileengine_p.h
    target_include_directories(${MODULE_NAME}
        PRIVATE
            ${Qt5Core_PRIVATE_INCLUDE_DIRS}
    )

    target_compile_definitions(${MODULE_NAME}
        PUBLIC
            -DQTANDROIDASSETS_STATIC=1
    )

endif(ANDROID)
