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

find_package(Qt6 6.3 COMPONENTS Core Quick #[[LinguistTools]] REQUIRED)

set(TS_FILES modelReviewTool_en_US.ts)

qt_add_executable(appmodelReviewTool
    main.cpp
    src/qbgfx.h
    src/qbgfx.cpp
    src/qsgbgfxnode/qsgbgfxnode.h
    src/qsgbgfxnode/qsgbgfxnode.cpp
    src/cubes.h
    ${TS_FILES}
)

qt_add_qml_module(appmodelReviewTool
    URI modelReviewTool
    VERSION 1.0
    QML_FILES main.qml
    SOURCES
    src/qquickbgfxitem/qquickbgfxitem.h
    src/qquickbgfxitem/qquickbgfxitem.cpp
)

#    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})

target_compile_definitions(appmodelReviewTool
    PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)
target_link_libraries(appmodelReviewTool
    PUBLIC
    Qt6::Core
    Qt6::Quick
    bgfx
    bimg
#    example-common
    glm)

target_compile_definitions(appmodelReviewTool PUBLIC SHADER_PARTH="${CMAKE_CURRENT_SOURCE_DIR}")

target_include_directories(appmodelReviewTool
    PUBLIC
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/qquickbgfxitem
    )

set_target_properties(appmodelReviewTool PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_import_qml_plugins(modelReviewTool)
    qt_finalize_executable(modelReviewTool)
endif()

install(TARGETS appmodelReviewTool
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)

add_subdirectory(3rdparty)
