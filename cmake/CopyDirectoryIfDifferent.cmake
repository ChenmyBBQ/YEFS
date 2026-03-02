if(NOT DEFINED SRC_DIR OR NOT DEFINED DST_DIR)
    message(FATAL_ERROR "SRC_DIR and DST_DIR must be provided")
endif()

if(NOT EXISTS "${SRC_DIR}")
    message(FATAL_ERROR "Source directory does not exist: ${SRC_DIR}")
endif()

file(GLOB_RECURSE _src_files RELATIVE "${SRC_DIR}" "${SRC_DIR}/*")

foreach(_relative_path IN LISTS _src_files)
    set(_src_path "${SRC_DIR}/${_relative_path}")
    if(IS_DIRECTORY "${_src_path}")
        continue()
    endif()

    set(_dst_path "${DST_DIR}/${_relative_path}")
    get_filename_component(_dst_parent "${_dst_path}" DIRECTORY)
    file(MAKE_DIRECTORY "${_dst_parent}")

    execute_process(
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${_src_path}" "${_dst_path}"
        RESULT_VARIABLE _copy_result
    )

    if(NOT _copy_result EQUAL 0)
        message(FATAL_ERROR "Failed to copy file: ${_src_path} -> ${_dst_path}")
    endif()
endforeach()
