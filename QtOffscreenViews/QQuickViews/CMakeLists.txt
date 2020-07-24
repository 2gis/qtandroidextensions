set(CMAKE_AUTOMOC OFF)

set(MODULE_NAME QtOffscreenViewsQuick)

set(SRC_LIST
    QQuickAndroidOffscreenView.cpp
    QQuickAndroidOffscreenView.h
    QQuickOffscreenEditText.cpp
    QQuickOffscreenEditText.h
    QQuickOffscreenWebView.cpp
    QQuickOffscreenWebView.h
)

if (ANDROID)
    qt5_wrap_cpp(
        QQuickViews/QQuickAndroidOffscreenView.h
        QQuickViews/QQuickOffscreenEditText.h
        QQuickViews/QQuickOffscreenWebView.h
    )

    add_library(${MODULE_NAME} STATIC ${SRC_LIST} ${QtOffscreenViewsQuick_hdr_moc})
    add_library(${PROJECT_NAME}::${MODULE_NAME} ALIAS ${MODULE_NAME})

    target_link_libraries(${MODULE_NAME}
        PUBLIC
            qtandroidextensions::QtOffscreenViews
        PRIVATE
            Qt5::OpenGL
            Qt5::Qml
            Qt5::Quick
    )

    target_include_directories(${MODULE_NAME}
        PUBLIC
            ${PROJECT_SOURCE_DIR}
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}
    )
endif(ANDROID)

