
cmake_minimum_required(VERSION 3.16)

project(YEFS VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Quick)

qt_standard_project_setup(REQUIRES 6.5)

# C++ 源文件
qt_add_executable(${PROJECT_NAME}
    cpp/main.cpp
    cpp/customtheme.cpp
    cpp/datagenerator.cpp
    cpp/themeswitchitem.cpp
    cpp/customtheme.h
    cpp/datagenerator.h
    cpp/themeswitchitem.h
)

# QML 文件
file(GLOB_RECURSE QML_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} CONFIGURE_DEPENDS "qml/*.qml")

qt_add_qml_module(${PROJECT_NAME}
    URI Gallery
    VERSION 1.0
    OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    RESOURCE_PREFIX "/"
    QML_FILES
        ${QML_FILES}
)

# 图片资源
qt_add_resources(${PROJECT_NAME} "images"
    PREFIX "/Gallery"
    FILES
        resources/images/huskarui_icon.svg
        resources/images/switch_effect1.jpg
        resources/images/switch_effect2.jpg
)
  
# 着色器资源
qt_add_shaders(${PROJECT_NAME} "shaders"
    PREFIX "/Gallery"
    FILES
        resources/shaders/effect1.vert
        resources/shaders/effect1.frag
        resources/shaders/effect2.vert
        resources/shaders/effect2.frag
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    $<$<BOOL:${BUILD_HUSKARUI_STATIC_LIBRARY}>:BUILD_HUSKARUI_STATIC_LIBRARY>
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

# 包含目录
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Quick
    HuskarUIBasic
    $<$<BOOL:${BUILD_HUSKARUI_STATIC_LIBRARY}>:HuskarUIBasicPlugin>
)

# from build output import <HuskarUI.Basic>
if(NOT HUSKARUI_PLUGIN_OUTPUT_DIRECTORY)
    set(QML_IMPORT_PATH ${CMAKE_BINARY_DIR}/HuskarUI/qml CACHE STRING "" FORCE)
else()
    set(QML_IMPORT_PATH ${HUSKARUI_PLUGIN_OUTPUT_DIRECTORY}/.. CACHE STRING "" FORCE)
endif()
target_compile_definitions(${PROJECT_NAME} PRIVATE HUSKARUI_IMPORT_PATH="${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/HuskarUI")

include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME}
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Deploy Script
if(CMAKE_BUILD_TYPE MATCHES "Release")
    if(APPLE)
        find_program(QT_DEPLOY_QT NAMES macdeployqt)
        set(QT_DEPLOY_ARGS
            ${CMAKE_SOURCE_DIR}/package/${PROJECT_NAME}.app
            --qmldir=${CMAKE_CURRENT_LIST_DIR}
        )
        add_custom_target(Script-DeployRelease
                COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_SOURCE_DIR}/package
                COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} ${CMAKE_SOURCE_DIR}/package
                COMMAND ${QT_DEPLOY_QT} ${QT_DEPLOY_ARGS}
                COMMENT "MacOs Deploying Qt Dependencies After Build........."
                SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )
        add_dependencies(Script-DeployRelease ${PROJECT_NAME})
    endif()
    if(WIN32)
        find_program(QT_DEPLOY_QT NAMES windeployqt)
        set(QT_DEPLOY_ARGS
            --qmldir=${CMAKE_CURRENT_LIST_DIR}
            --plugindir=${CMAKE_SOURCE_DIR}/package/plugins
            --no-opengl-sw
            --no-widgets
            --no-virtualkeyboard
            --no-quick3dutils
            --no-quickcontrols2windowsstyleimpl
            --no-quickcontrols2fusion
            --no-quickcontrols2fusionstyleimpl
            --no-quickcontrols2material
            --no-quickcontrols2materialstyleimpl
            --no-quickcontrols2universal
            --no-quickcontrols2universalstyleimpl
            --no-quickcontrols2imagine
            --no-quickcontrols2imaginestyleimpl
            --no-translations
            --compiler-runtime
            ${CMAKE_SOURCE_DIR}/package/${PROJECT_NAME}.exe
        )
        add_custom_target(Script-DeployRelease
                COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_SOURCE_DIR}/package
                COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} ${CMAKE_SOURCE_DIR}/package
                COMMAND ${CMAKE_COMMAND} -E rm -f "${CMAKE_SOURCE_DIR}/package/${PROJECT_NAME}.qmltypes"
                COMMAND ${CMAKE_COMMAND} -E rm -f "${CMAKE_SOURCE_DIR}/package/${PROJECT_NAME}_qml_module_dir_map.qrc"
                COMMAND ${CMAKE_COMMAND} -E rm -f "${CMAKE_SOURCE_DIR}/package/qmldir"
                COMMAND ${QT_DEPLOY_QT} ${QT_DEPLOY_ARGS}
                COMMENT "Windows Deploying Qt Dependencies After Build........."
                SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )
        add_dependencies(Script-DeployRelease ${PROJECT_NAME})
    endif()
endif()
