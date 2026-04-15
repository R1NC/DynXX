function(init_before_project)
    # Policy CMP0091: Controls MSVC runtime library flags.
    # NEW behavior: Use CMAKE_MSVC_RUNTIME_LIBRARY to set /MD, /MT, etc.
    # Available since CMake 3.15
    if(POLICY CMP0091)
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

# Args:
#   TARGET_NAME - Required, target name for include directories
#   lib         - Required, library name for FetchContent
#   url         - Required, git repository URL
#   tag         - Required, git tag/branch or commit hash
#   INC_DIR     - Optional, include directory path relative to repo root
#   SRC_DIR     - Optional, source directory path relative to repo root
#   MANUAL_ADD  - Optional, BOOL, call add_subdirectory manually, default OFF
function(add_git_lib TARGET_NAME lib url tag)
    if(NOT TARGET_NAME OR NOT lib OR NOT url OR NOT tag)
        message(FATAL_ERROR "`add_git_lib()`: `TARGET_NAME`, `lib`, `url`, `tag` are required!")
    endif()

    cmake_parse_arguments(ARG "" "INC_DIR;SRC_DIR;MANUAL_ADD" "" ${ARGN})

    if(NOT ARG_INC_DIR)
        set(ARG_INC_DIR "")
    endif()
    if(NOT ARG_SRC_DIR)
        set(ARG_SRC_DIR "")
    endif()
    if(NOT DEFINED ARG_MANUAL_ADD)
        set(ARG_MANUAL_ADD OFF)
    endif()

    FetchContent_Declare(
        ${lib}
        GIT_REPOSITORY ${url}
        GIT_TAG ${tag}
        GIT_SHALLOW ON
        GIT_PROGRESS ON
        GIT_REMOTE_UPDATE_STRATEGY REBASE_CHECKOUT
    )
    FetchContent_MakeAvailable(${lib})

    set(inc_path ${${lib}_SOURCE_DIR}/${ARG_INC_DIR})
    set(src_path ${${lib}_SOURCE_DIR}/${ARG_SRC_DIR})

    if(ARG_MANUAL_ADD)
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
