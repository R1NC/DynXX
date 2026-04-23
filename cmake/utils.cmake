function(init_before_project)
    if(POLICY CMP0091)
        # CMP0091 controls MSVC runtime library selection behavior
        # (CMAKE_MSVC_RUNTIME_LIBRARY).
        cmake_policy(SET CMP0091 NEW)
    endif()

    if(NOT (EMSCRIPTEN AND CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows"))
        find_program(CCACHE_PROGRAM ccache)
    endif()
    if(CCACHE_PROGRAM)
        set(CMAKE_C_COMPILER_LAUNCHER "${CCACHE_PROGRAM}" PARENT_SCOPE)
        set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}" PARENT_SCOPE)
        set(CCACHE_FOUND TRUE PARENT_SCOPE)
    else()
        set(CCACHE_FOUND FALSE PARENT_SCOPE)
    endif()
endfunction()

function(init_after_project)
    set(CXX_EXTENSIONS_SETTING OFF PARENT_SCOPE)
    if(CMAKE_SYSTEM_NAME MATCHES "CYGWIN" OR CMAKE_SYSTEM_NAME MATCHES "MSYS" OR CMAKE_SYSTEM_NAME MATCHES "MINGW")
        set(CXX_EXTENSIONS_SETTING ON PARENT_SCOPE)
    endif()
    set(CMAKE_CXX_EXTENSIONS ${CXX_EXTENSIONS_SETTING} PARENT_SCOPE)

    include(ProcessorCount)
    ProcessorCount(N PARENT_SCOPE)
    if(NOT N EQUAL 0)
        set(CMAKE_BUILD_PARALLEL_LEVEL ${N} PARENT_SCOPE)
    endif()

    include(FetchContent)
    set(FETCHCONTENT_UPDATES_DISCONNECTED ON PARENT_SCOPE)
    set(FETCHCONTENT_QUIET OFF PARENT_SCOPE)
    set(FETCHCONTENT_TIMEOUT 600 PARENT_SCOPE)
    set(FETCHCONTENT_DOWNLOAD_TIMEOUT_SEC 600 PARENT_SCOPE)
endfunction()

function(check_apple_version_limit iosV macV RESULT_VAR)
    if(APPLE AND ((CMAKE_SYSTEM_NAME STREQUAL "iOS" AND CMAKE_OSX_DEPLOYMENT_TARGET VERSION_LESS "${iosV}") OR
                  (CMAKE_SYSTEM_NAME STREQUAL "Darwin" AND CMAKE_OSX_DEPLOYMENT_TARGET VERSION_LESS "${macV}")))
        set(${RESULT_VAR} OFF PARENT_SCOPE)
    else()
        set(${RESULT_VAR} ON PARENT_SCOPE)
    endif()
endfunction()

# Optional named args after required ones (recommended):
#   INC_DIR <path>
#   SRC_DIR <path>
#   MANUAL_ADD_SUBDIRECTORY <bool> (default: OFF)
function(add_git_lib TARGET_NAME lib url tag)
    if(NOT TARGET_NAME OR NOT lib OR NOT url OR NOT tag)
        message(FATAL_ERROR "`add_git_lib()`: `TARGET_NAME`, `lib`, `url`, `tag` are required!")
    endif()
    set(inc_dir "")
    set(src_dir "")
    set(manual_add_subdirectory OFF)

    cmake_parse_arguments(AGL
        ""
        "INC_DIR;SRC_DIR;MANUAL_ADD_SUBDIRECTORY"
        ""
        ${ARGN}
    )
    if(AGL_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "`add_git_lib()`: unknown args: ${AGL_UNPARSED_ARGUMENTS}")
    endif()
    if(DEFINED AGL_INC_DIR)
        set(inc_dir "${AGL_INC_DIR}")
    endif()
    if(DEFINED AGL_SRC_DIR)
        set(src_dir "${AGL_SRC_DIR}")
    endif()
    if(DEFINED AGL_MANUAL_ADD_SUBDIRECTORY)
        set(manual_add_subdirectory "${AGL_MANUAL_ADD_SUBDIRECTORY}")
    endif()

    set(fetchcontent_args
        GIT_REPOSITORY ${url}
        GIT_TAG ${tag}
        GIT_SHALLOW ON
        GIT_PROGRESS ON
        GIT_REMOTE_UPDATE_STRATEGY REBASE_CHECKOUT
    )

    FetchContent_Declare(${lib} ${fetchcontent_args})
    FetchContent_MakeAvailable(${lib})

    set(inc_path ${${lib}_SOURCE_DIR}/${inc_dir})
    set(src_path ${${lib}_SOURCE_DIR}/${src_dir})

    if(manual_add_subdirectory)
        if(NOT src_dir)
            message(FATAL_ERROR "`add_git_lib()`: `src_dir` is required when `manual_add_subdirectory` is ON")
        endif()
        if(TARGET ${lib})
            message(WARNING "${lib} already exists, skip `add_subdirectory()`")
        else()
            add_subdirectory(${src_path})
        endif()
    endif()

    if(EXISTS ${inc_path})
        target_include_directories(${TARGET_NAME} PRIVATE ${inc_path})
    else()
        message(WARNING "${inc_path} not exists, skip `target_include_directories()`")
    endif()

    set(${lib}_INC_PATH ${inc_path} PARENT_SCOPE)
    set(${lib}_SRC_PATH ${src_path} PARENT_SCOPE)
endfunction()

function(add_definitions_if TARGET_NAME NEED_CHECK)
    foreach(def ${ARGN})
        if(NEED_CHECK)
            if(DEFINED ${def} AND ${def})
                target_compile_definitions(${TARGET_NAME} PRIVATE ${def})
            endif()
        else()
            target_compile_definitions(${TARGET_NAME} PRIVATE ${def})
        endif()
    endforeach()
endfunction()

function(update_git_submodules)
    execute_process(
        COMMAND git submodule update --init --recursive
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        RESULT_VARIABLE git_submod_result
    )
endfunction()

function(install_headers SOURCE_DIR DESTINATION_DIR)
    include(GNUInstallDirs)
    install(DIRECTORY "${SOURCE_DIR}"
        DESTINATION "${DESTINATION_DIR}"
        COMPONENT headers
        FILES_MATCHING
        PATTERN "*.h"
        PATTERN "*.hxx"
    )
endfunction()

function(link_whole_archive CONSUMER_TARGET STATIC_LIB_TARGET)
    if(NOT TARGET ${CONSUMER_TARGET})
        message(FATAL_ERROR "`link_whole_archive()`: target `${CONSUMER_TARGET}` not found")
    endif()
    if(NOT TARGET ${STATIC_LIB_TARGET})
        message(FATAL_ERROR "`link_whole_archive()`: target `${STATIC_LIB_TARGET}` not found")
    endif()

    if(MSVC)
        target_link_options(${CONSUMER_TARGET} PRIVATE "/WHOLEARCHIVE:$<TARGET_FILE:${STATIC_LIB_TARGET}>")
    elseif(APPLE)
        target_link_options(${CONSUMER_TARGET} PRIVATE "-Wl,-force_load,$<TARGET_FILE:${STATIC_LIB_TARGET}>")
    else()
        target_link_options(${CONSUMER_TARGET} PRIVATE
            "-Wl,--whole-archive"
            "$<TARGET_FILE:${STATIC_LIB_TARGET}>"
            "-Wl,--no-whole-archive"
        )
    endif()
endfunction()

function(apply_coverage_flags TARGET_NAME)
    if(NOT TARGET ${TARGET_NAME})
        message(FATAL_ERROR "`apply_coverage_flags()`: target `${TARGET_NAME}` not found")
    endif()

    if(NOT DYNXX_ENABLE_COVERAGE)
        return()
    endif()

    if(MSVC)
        set(DYNXX_COVERAGE_COMPILE_OPTIONS
            "/clang:-fprofile-instr-generate"
            "/clang:-fcoverage-mapping"
        )
        set(DYNXX_COVERAGE_CXX_COMPILE_OPTIONS
            "/EHsc"
        )
    else()
        set(DYNXX_COVERAGE_COMPILE_OPTIONS
            "-fprofile-instr-generate"
            "-fcoverage-mapping"
        )
        set(DYNXX_COVERAGE_CXX_COMPILE_OPTIONS "")
    endif()

    target_compile_options(${TARGET_NAME}
        PRIVATE
            ${DYNXX_COVERAGE_COMPILE_OPTIONS}
            $<$<COMPILE_LANGUAGE:CXX>:${DYNXX_COVERAGE_CXX_COMPILE_OPTIONS}>
    )

    get_target_property(_target_type ${TARGET_NAME} TYPE)
    if(NOT MSVC AND NOT _target_type STREQUAL "STATIC_LIBRARY")
        target_link_options(${TARGET_NAME}
            PRIVATE
                ${DYNXX_COVERAGE_COMPILE_OPTIONS}
        )
    endif()
endfunction()
