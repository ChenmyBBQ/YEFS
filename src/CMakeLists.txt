cmake_minimum_required(VERSION 3.16)

project(YEFS VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Quick Location Positioning)

qt_standard_project_setup(REQUIRES 6.5)

# ============================================================================
# 核心框架源文件
# ============================================================================
set(CORE_SOURCES
    core/Application.cpp
    core/Application.h
    core/IMapEngine.h
    core/IPlugin.h
)

# 兼容旧代码
set(LEGACY_SOURCES
    cpp/main.cpp
    cpp/customtheme.cpp
    cpp/customtheme.h
    cpp/datagenerator.cpp
    cpp/datagenerator.h
    cpp/themeswitchitem.cpp
    cpp/themeswitchitem.h
)

# ============================================================================
# 可执行文件
# ============================================================================
qt_add_executable(${PROJECT_NAME}
    ${CORE_SOURCES}
    ${LEGACY_SOURCES}
)

# Ensure plugins are built when main app is built
add_dependencies(${PROJECT_NAME} huskaruiimplplugin huskaruibasicplugin)

# Fix for HuskarUI plugins not being found by QML engine
# We copy them from the lib output directory to the QML module directory
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:huskaruiimplplugin>
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/HuskarUI/Impl/$<TARGET_FILE_NAME:huskaruiimplplugin>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:huskaruibasicplugin>
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/HuskarUI/Basic/$<TARGET_FILE_NAME:huskaruibasicplugin>
    COMMENT "Copying HuskarUI plugins to QML module directories"
)

# ============================================================================
# QML 模块
# ============================================================================

# 收集所有 QML 文件
file(GLOB_RECURSE QML_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "qml/*.qml")

qt_add_qml_module(${PROJECT_NAME}
    URI YEFSApp
    VERSION 1.0
    RESOURCE_PREFIX "/"
    QML_FILES
        ${QML_FILES}
    SOURCES
        core/MessageBus.h
        core/MessageBus.cpp
        core/PluginManager.h
        core/PluginManager.cpp
        core/MapLibreEngine.h
        core/MapLibreEngine.cpp
        core/MapSettings.h
        core/MapSettings.cpp
        core/SettingsManager.h
        core/SettingsManager.cpp
        core/UnitManager.h
        core/UnitManager.cpp
)

# ============================================================================
# 资源文件
# ============================================================================

qt_add_resources(${PROJECT_NAME} "images"
    PREFIX "/YEFSApp"
    FILES
        resources/images/YEFS.svg
        resources/images/switch_effect1.jpg
        resources/images/switch_effect2.jpg
)

# 着色器资源
qt_add_shaders(${PROJECT_NAME} "shaders"
    PREFIX "/YEFSApp"
    FILES
        resources/shaders/effect1.vert
        resources/shaders/effect1.frag
        resources/shaders/effect2.vert
        resources/shaders/effect2.frag
)

# ============================================================================
# 编译配置
# ============================================================================

target_compile_definitions(${PROJECT_NAME} PRIVATE
    $<$<BOOL:${BUILD_HUSKARUI_STATIC_LIBRARY}>:BUILD_HUSKARUI_STATIC_LIBRARY>
    YEFS_VERSION="${PROJECT_VERSION}"
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

# ============================================================================
# 包含目录
# ============================================================================

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/core
    ${CMAKE_CURRENT_SOURCE_DIR}/cpp
)

# ============================================================================
# 链接库
# ============================================================================

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Quick
    Qt6::Location
    Qt6::Positioning
    HuskarUIBasic
    QMapLibre::Core
    QMapLibre::Location
    $<$<BOOL:${BUILD_HUSKARUI_STATIC_LIBRARY}>:HuskarUIBasicPlugin>
)

# Ensure HuskarUI plugins are built before the main target
add_dependencies(${PROJECT_NAME} huskaruibasicplugin huskaruiimplplugin)

# Copy HuskarUI DLLs to output directory after build
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:HuskarUIBasic>
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    COMMENT "Copying HuskarUI DLL to output directory"
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:huskaruibasicplugin>
        ${HUSKARUI_PLUGIN_OUTPUT_DIRECTORY}
    COMMENT "Copying HuskarUI Basic plugin to output directory"
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:huskaruiimplplugin>
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/HuskarUI/Impl
    COMMENT "Copying HuskarUI Impl plugin to output directory"
)

# Copy HuskarUI QML module files to output directory
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_BINARY_DIR}/HuskarUI/qml/HuskarUI
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/HuskarUI
    COMMENT "Copying HuskarUI QML modules to output directory"
)




# Ensure MapLibre plugin is built and copied
if(TARGET qtgeoservices_maplibre)
    add_dependencies(${PROJECT_NAME} qtgeoservices_maplibre)

    # Copy MapLibre GeoServices plugin
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/geoservices
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:qtgeoservices_maplibre>
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/geoservices
        COMMENT "Copying MapLibre GeoServices plugin"
    )
endif()

# Ensure MapLibre QML modules are built and copied
if(TARGET declarative_maplibre)
    add_dependencies(${PROJECT_NAME} declarative_maplibre)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/MapLibre
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_BINARY_DIR}/3rdparty/maplibre-native-qt/src/quick/plugins/MapLibre
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/MapLibre
        COMMENT "Copying MapLibre QML module"
    )
endif()

if(TARGET declarative_maplibre_locationplugin)
    add_dependencies(${PROJECT_NAME} declarative_maplibre_locationplugin)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/MapLibre/Location
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_BINARY_DIR}/3rdparty/maplibre-native-qt/src/location/plugins/MapLibre/Location
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/MapLibre/Location
        COMMENT "Copying MapLibre Location QML module"
    )
endif()

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
        add_custom_target(YEFS-DeployRelease
                COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_SOURCE_DIR}/package
                COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} ${CMAKE_SOURCE_DIR}/package
                COMMAND ${QT_DEPLOY_QT} ${QT_DEPLOY_ARGS}
                COMMENT "MacOs Deploying Qt Dependencies After Build........."
                SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )
        add_dependencies(YEFS-DeployRelease ${PROJECT_NAME})
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
        add_custom_target(YEFS-DeployRelease
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
        add_dependencies(YEFS-DeployRelease ${PROJECT_NAME})
    endif()
endif()
