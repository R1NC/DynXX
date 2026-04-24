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
