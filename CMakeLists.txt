cmake_minimum_required(VERSION 3.16)

project(YEFS_Solution)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/out/${CMAKE_BUILD_TYPE}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/out/${CMAKE_BUILD_TYPE}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/out/${CMAKE_BUILD_TYPE}/lib)

set(HUSKARUI_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/out/${CMAKE_BUILD_TYPE})
# IMPORTANT: point to a common HuskarUI root so both QML modules end up in:
#   <bin>/HuskarUI/Basic and <bin>/HuskarUI/Impl
# HuskarUI's src_impl computes Impl output as ${HUSKARUI_PLUGIN_OUTPUT_DIRECTORY}/../Impl
set(HUSKARUI_PLUGIN_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/HuskarUI/Basic)

set(BUILD_HUSKARUI_STATIC_LIBRARY OFF)
set(BUILD_HUSKARUI_ON_DESKTOP_PLATFORM ON)
set(INSTALL_HUSKARUI_IN_DEFAULT_LOCATION ON)

# MapLibre Native Qt options
set(MLN_QT_WITH_LOCATION ON)
set(MLN_QT_WITH_WIDGETS OFF)
set(MLN_QT_WITH_QUICK_PLUGIN ON)
set(MLN_WITH_OPENGL ON)

# Build third-party dependencies
add_subdirectory(3rdparty/HuskarUI)
add_subdirectory(3rdparty/maplibre-native-qt)

# Fix MapLibre include paths from the top-level project
if(TARGET MLNQtLocation)
    target_include_directories(MLNQtLocation PRIVATE
        ${CMAKE_SOURCE_DIR}/3rdparty/maplibre-native-qt/src/core
        ${CMAKE_SOURCE_DIR}/3rdparty/maplibre-native-qt/src/core/style
        ${CMAKE_SOURCE_DIR}/3rdparty/maplibre-native-qt/src/quick
        ${CMAKE_BINARY_DIR}/3rdparty/maplibre-native-qt/src/core/include
    )
endif()
if(TARGET MLNQtQuickPrivate)
    target_include_directories(MLNQtQuickPrivate PRIVATE
        ${CMAKE_SOURCE_DIR}/3rdparty/maplibre-native-qt/src/quick
        ${CMAKE_BINARY_DIR}/3rdparty/maplibre-native-qt/src/quick/include
    )
endif()
if(TARGET declarative_maplibre)
    target_include_directories(declarative_maplibre PRIVATE
        ${CMAKE_SOURCE_DIR}/3rdparty/maplibre-native-qt/src/quick
        ${CMAKE_BINARY_DIR}/3rdparty/maplibre-native-qt/src/quick/include
        ${CMAKE_SOURCE_DIR}/3rdparty/maplibre-native-qt/src/core
        ${CMAKE_SOURCE_DIR}/3rdparty/maplibre-native-qt/src/core/style
        ${CMAKE_BINARY_DIR}/3rdparty/maplibre-native-qt/src/core/include
    )
endif()

if(TARGET qtgeoservices_maplibre)
    target_include_directories(qtgeoservices_maplibre PRIVATE
        ${CMAKE_SOURCE_DIR}/3rdparty/maplibre-native-qt/src/core
        ${CMAKE_SOURCE_DIR}/3rdparty/maplibre-native-qt/src/core/style
        ${CMAKE_BINARY_DIR}/3rdparty/maplibre-native-qt/src/core/include
    )
endif()

if(TARGET declarative_maplibre_locationplugin)
    target_include_directories(declarative_maplibre_locationplugin PRIVATE
        ${CMAKE_SOURCE_DIR}/3rdparty/maplibre-native-qt/src/core
        ${CMAKE_SOURCE_DIR}/3rdparty/maplibre-native-qt/src/core/style
        ${CMAKE_BINARY_DIR}/3rdparty/maplibre-native-qt/src/core/include
    )
endif()

#Your project
add_subdirectory(src)
