set(CMAKE_AUTOMOC OFF)

set(MODULE_NAME QtOffscreenViews)

set(SRC_LIST
    QAndroidJniImagePair.cpp
    QAndroidJniImagePair.h
    QAndroidOffscreenEditText.cpp
    QAndroidOffscreenEditText.h
    QAndroidOffscreenView.cpp
    QAndroidOffscreenView.h
    QAndroidOffscreenWebView.cpp
    QAndroidOffscreenWebView.h
    QApplicationActivityObserver.cpp
    QApplicationActivityObserver.h
    QOpenGLTextureHolder.cpp
    QOpenGLTextureHolder.h
)

if (ANDROID)
    qt6_wrap_cpp(
        QtOffscreenViews_hdr_moc
        QAndroidJniImagePair.h
        QAndroidOffscreenEditText.h
        QAndroidOffscreenView.h
        QAndroidOffscreenWebView.h
        QApplicationActivityObserver.h
    )

    find_library(ANDROID_JNIGRAPHICS jnigraphics)
    add_library(${MODULE_NAME} STATIC ${SRC_LIST} ${QtOffscreenViews_hdr_moc})
    add_library(${PROJECT_NAME}::${MODULE_NAME} ALIAS ${MODULE_NAME})

    target_link_libraries(${MODULE_NAME}
        PUBLIC
            Qt6::Core
            Qt6::Gui
            qtandroidextensions::QtJniHelpers
        PRIVATE
            Qt6::OpenGL
            Qt6::OpenGLWidgets
            ${ANDROID_JNIGRAPHICS}
    )

    target_include_directories(${MODULE_NAME}
        PUBLIC
            ${PROJECT_SOURCE_DIR}
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}
    )
endif(ANDROID)

