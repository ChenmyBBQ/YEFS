cmake_minimum_required(VERSION 3.16)

project(DBCompt VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Sql)

# ============================================================================
# DBCompt 动态库
# ============================================================================
add_library(DBCompt SHARED
    dbcompt_global.h
    DBCompt.h
    DBCompt.cpp
    core/DBCore.h
    core/DBCore.cpp
    schema/SchemaTypes.h
    schema/SchemaLoader.h
    schema/SchemaLoader.cpp
    migration/MigrationBackup.h
    migration/MigrationBackup.cpp
    migration/MigrationManager.h
    migration/MigrationManager.cpp
    driver/IDriver.h
    driver/SQLiteDriver.h
    driver/SQLiteDriver.cpp
    driver/PostgresDriver.h
    driver/PostgresDriver.cpp
    query/QueryResult.h
    query/QueryBuilder.h
    query/QueryBuilder.cpp
    utils/DBLogger.h
    utils/DBLogger.cpp
)

target_compile_definitions(DBCompt PRIVATE DBCOMPT_LIBRARY)

target_include_directories(DBCompt PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include/dbcompt>
)

target_link_libraries(DBCompt PRIVATE
    Qt6::Core
    Qt6::Sql
)

# ============================================================================
# 构建后将 DLL 复制到运行输出目录
# ============================================================================
add_custom_command(TARGET DBCompt POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:DBCompt>
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    COMMENT "Copying DBCompt to runtime output directory"
)

# ============================================================================
# 将默认 schema 文件部署到 AppConfig 目录
# ============================================================================
set(DBCOMPT_SCHEMA_SRC "${CMAKE_SOURCE_DIR}/AppConfig/db_schema.json")
set(DBCOMPT_SCHEMA_DST "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/AppConfig/db_schema.json")

add_custom_command(TARGET DBCompt POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
        "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/AppConfig"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${DBCOMPT_SCHEMA_SRC}"
        "${DBCOMPT_SCHEMA_DST}"
    COMMENT "Deploying db_schema.json to AppConfig"
)
