cmake_minimum_required(VERSION 3.16)

project(AirspaceManagerPlugin VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Quick Positioning)

# ============================================================================
# 插件动态库 (MODULE 类型，供 QPluginLoader 加载)
# ============================================================================
add_library(${PROJECT_NAME} MODULE
    AirspaceManagerPlugin.h
    AirspaceManagerPlugin.cpp
    AirspaceDisplayMapper.h
    AirspaceDisplayMapper.cpp
    AirspaceDraftValidator.h
    AirspaceDraftValidator.cpp
    AirspaceMapLayerMapper.h
    AirspaceMapLayerMapper.cpp
    AirspacePayloadConverter.h
    AirspacePayloadConverter.cpp
    AirspaceSelectionController.h
    AirspaceSelectionController.cpp
    AirspaceDatabase.h
    AirspaceDatabase.cpp
    AirspaceGeometryConverter.h
    AirspaceGeometryConverter.cpp
    AirspaceModel.h
    AirspaceModel.cpp
    AirspaceRecordMapper.h
    AirspaceRecordMapper.cpp
    AirspaceTypes.h
    AirspaceTypes.cpp
    ShapeGenerator.h
    ShapeGenerator.cpp
    DrawingController.h
    DrawingController.cpp
    ${CMAKE_SOURCE_DIR}/src/core/IPlugin.h
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src/core
    ${CMAKE_SOURCE_DIR}/src/dbcompt
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Quick
    Qt6::Positioning
    DBCompt
)

# ============================================================================
# QML 资源嵌入
# ============================================================================
qt_add_qml_module(${PROJECT_NAME}
    URI AirspaceManagerPlugin
    VERSION 1.0
    RESOURCE_PREFIX "/"
    QML_FILES
        qml/AirspaceManagerPage.qml
        qml/AirspaceToolbar.qml
        qml/AirspaceEditDialog.qml
        qml/AirspaceListPanel.qml
        qml/AirspaceSettings.qml
        qml/components/ShapePropertyEditor.qml
        qml/components/DataPropertyEditor.qml
        qml/components/AirspaceInfoPanel.qml
    NO_PLUGIN
    NO_GENERATE_PLUGIN_SOURCE
)

# ============================================================================
# 输出到 plugins 目录
# ============================================================================
set_target_properties(${PROJECT_NAME} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/plugins"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/plugins"
)

# 确保输出目录存在
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/plugins"
    COMMENT "Ensuring plugins directory exists"
)
