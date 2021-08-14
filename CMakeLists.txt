cmake_minimum_required(VERSION 3.17)

project(modelReviewTool VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(INSTALL_EXAMPLEDIR "modelReviewTool")

# QtCreator supports the following variables for Android, which are identical to qmake Android variables.
# Check https://doc.qt.io/qt/deployment-android.html for more information.
# They need to be set before the find_package(...) calls below.

#if(ANDROID)
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
#    if (ANDROID_ABI STREQUAL "armeabi-v7a")
#        set(ANDROID_EXTRA_LIBS
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libcrypto.so
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libssl.so)
#    endif()
#endif()

find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Quick #[[LinguistTools]] REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Quick #[[LinguistTools]] REQUIRED)
#find_package(EnTT CONFIG REQUIRED)

set(TS_FILES modelReviewTool_en_US.ts)

set(PROJECT_SOURCES
        main.cpp
        qml.qrc
        src/qquickbgfxitem/qquickbgfxitem.h
        src/qquickbgfxitem/qquickbgfxitem.cpp
        src/qbgfx.h
        src/qbgfx.cpp
        src/qsgbgfxnode/qsgbgfxnode.h
        src/qsgbgfxnode/qsgbgfxnode.cpp
        src/openfbx/ofbx.h
        src/openfbx/ofbx.cpp
        src/cubes.h
        ${TS_FILES}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(modelReviewTool
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )

#    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    if(ANDROID)
        add_library(modelReviewTool SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(modelReviewTool
          ${PROJECT_SOURCES}
        )
    endif()

#    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

target_compile_definitions(modelReviewTool
  PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)

target_link_libraries(modelReviewTool
    PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Quick
    bgfx
    bimg
    example-common
    glm
    EnTT::EnTT
    )
target_include_directories(modelReviewTool
    PUBLIC
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/qquickbgfxitem
    )

set_target_properties(modelReviewTool PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    QT_QML_MODULE_VERSION 1.0
    QT_QML_MODULE_URI QQuickBgfx
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_import_qml_plugins(modelReviewTool)
    qt_finalize_executable(modelReviewTool)
endif()

install(TARGETS modelReviewTool
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)

qt6_qml_type_registration(modelReviewTool)
add_subdirectory(3rdparty)
add_subdirectory(src/3dparts)
