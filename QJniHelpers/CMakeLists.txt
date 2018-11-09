set(MODULE_NAME QtJniHelpers)

set(SRC_LIST
    IJniObjectLinker.h
    QAndroidQPAPluginGap.cpp
    QAndroidQPAPluginGap.h
    QJniHelpers.cpp
    QJniHelpers.h
    QJniHelpers.pri
    QJniHelpers.pro
    QJniLangUtils.cpp
    QJniLangUtils.h
    TJniObjectLinker.h
)

if (ANDROID)
    add_library(${MODULE_NAME} STATIC ${SRC_LIST})
    add_library(${PROJECT_NAME}::${MODULE_NAME} ALIAS ${MODULE_NAME})

    target_link_libraries(${MODULE_NAME}
        PUBLIC
            Qt5::Core
    )

    target_include_directories(${MODULE_NAME}
        PUBLIC
            ${PROJECT_SOURCE_DIR}
    )
endif(ANDROID)

