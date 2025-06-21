cmake_minimum_required(VERSION 3.10)

string(
    COMPARE EQUAL
    "${CMAKE_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}"
    IS_ROOT
)

#if(IS_ROOT)
    set(CMAKE_C_COMPILER "clang")
    set(CMAKE_CXX_COMPILER "clang++")
    set(CMAKE_C_STANDARD 99)
    set(CMAKE_CXX_STANDARD 23)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
#endif()

project(NGenXX)

include(build-tools/toolchains/CPM/cmake/CPM.cmake)

function(checkAppleVersionLimit iosV macV RESULT_VAR)
    if(APPLE AND ((CMAKE_SYSTEM_NAME STREQUAL "iOS" AND CMAKE_OSX_DEPLOYMENT_TARGET VERSION_LESS "${iosV}") OR 
                  (CMAKE_SYSTEM_NAME STREQUAL "Darwin" AND CMAKE_OSX_DEPLOYMENT_TARGET VERSION_LESS "${macV}")))
        set(${RESULT_VAR} FALSE PARENT_SCOPE)
    else()
        set(${RESULT_VAR} TRUE PARENT_SCOPE)
    endif()
endfunction()

## Definitions:

set(USE_STD_TO_CHARS FALSE)
checkAppleVersionLimit("16.5" "13.4" USE_STD_TO_CHARS)
if(USE_STD_TO_CHARS)
    add_definitions(-DUSE_STD_TO_CHARS)
endif()

option(USE_LUA "Use Lua" ON)
if(USE_LUA)
    add_definitions(-DUSE_LUA)
endif()

option(USE_QJS "Use QuickJS" ON)
if(USE_QJS)
    add_definitions(-DUSE_QJS -DCONFIG_BIGNUM)
endif()

if(EMSCRIPTEN OR (NOT USE_LUA))
    option(USE_LIBUV "Use LibUV" OFF)
else()
    option(USE_LIBUV "Use LibUV" ON)
    add_definitions(-DUSE_LIBUV)
endif()

option(USE_SPDLOG "Use SpdLog" ON)
if(USE_SPDLOG)
    add_definitions(-DUSE_SPDLOG)
endif()

# ada uses `std::ranges` in C++20
if(OHOS OR (APPLE AND ((CMAKE_SYSTEM_NAME STREQUAL "iOS" AND CMAKE_OSX_DEPLOYMENT_TARGET VERSION_LESS "16.3") OR (CMAKE_SYSTEM_NAME STREQUAL "Darwin" AND CMAKE_OSX_DEPLOYMENT_TARGET VERSION_LESS "13.3"))))
    option(USE_ADA "Use Ada" OFF)
else()
    option(USE_ADA "Use Ada" ON)
    add_definitions(-DUSE_ADA)
endif()

## Source & Output:

file(GLOB_RECURSE SRC_FILES "src/*.cxx" "src/*.mm")
if(ANDROID) # Android can not find built-in `ifaddrs.h`;
    CPMAddPackage(
        NAME ifaddrs
        GIT_TAG dev
        GITHUB_REPOSITORY R1NC/android-ifaddrs
        DOWNLOAD_ONLY YES
    )
    list(APPEND SRC_FILES ${ifaddrs_SOURCE_DIR}/ifaddrs.c)
endif()

if(EMSCRIPTEN)
    add_compile_options(
        -Wno-error=conversion
        -Wno-error=old-style-cast
    )
    add_executable(${PROJECT_NAME} ${SRC_FILES})
    set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".html")
    # WARNING: Export with `EMSCRIPTEN_KEEPALIVE` will cause Lua running automatically.
    if(USE_LUA)
        set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "-s ALLOW_TABLE_GROWTH -s EXPORTED_RUNTIME_METHODS=['getValue','setValue','UTF8ToString','lengthBytesUTF8','stringToUTF8','addFunction'] -s EXPORTED_FUNCTIONS=['_malloc','_free','_ngenxx_init','_ngenxx_release','_ngenxx_lua_loadS','_ngenxx_lua_call']")
    else()
        set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "-s ALLOW_TABLE_GROWTH -s EXPORTED_RUNTIME_METHODS=['getValue','setValue','UTF8ToString','lengthBytesUTF8','stringToUTF8','addFunction'] -s EXPORTED_FUNCTIONS=['_malloc','_free','_ngenxx_init','_ngenxx_release']")
    endif()
    message(STATUS "\nLINK_FLAGS:\n ${CMAKE_LINK_FLAGS}\n")
else()
    add_library(${PROJECT_NAME} STATIC ${SRC_FILES})
endif()

## Include Directories:

list(APPEND INC_DIRS include)

CPMAddPackage(
    NAME cjson
    GIT_TAG dev
    GITHUB_REPOSITORY R1NC/cjson
)
list(APPEND INC_DIRS ${cjson_SOURCE_DIR})

CPMAddPackage(
    NAME mmkv
    GIT_TAG dev
    GITHUB_REPOSITORY R1NC/mmkv
    DOWNLOAD_ONLY YES
)
add_subdirectory(${mmkv_SOURCE_DIR}/POSIX/src _deps/mmkv-build)
list(APPEND INC_DIRS ${mmkv_SOURCE_DIR}/Core)

CPMAddPackage(
    NAME openssl
    GIT_TAG dev
    GITHUB_REPOSITORY R1NC/openssl
)
list(APPEND INC_DIRS ${openssl_SOURCE_DIR}/include)

## Config OpenSSL for libcurl to find libssl and libcrypto
set(OPENSSL_ROOT_DIR ${openssl_SOURCE_DIR})
set(OPENSSL_INCLUDE_DIR ${openssl_SOURCE_DIR}/include)
add_library(OpenSSL::SSL ALIAS ssl)
add_library(OpenSSL::Crypto ALIAS crypto)
set(OPENSSL_SSL_LIBRARY ssl)
set(OPENSSL_CRYPTO_LIBRARY crypto)

CPMAddPackage(
    NAME curl
    GIT_TAG dev
    GITHUB_REPOSITORY R1NC/curl
)
list(APPEND INC_DIRS ${curl_SOURCE_DIR}/include)

if(USE_LUA)
    CPMAddPackage(
        NAME lua
        GIT_TAG dev
        GITHUB_REPOSITORY R1NC/lua
    )
    list(APPEND INC_DIRS ${lua_SOURCE_DIR})
endif()

if(USE_QJS)
    CPMAddPackage(
        NAME quickjs
        GIT_TAG dev
        GITHUB_REPOSITORY R1NC/quickjs-ng
    )
    list(APPEND INC_DIRS ${quickjs_SOURCE_DIR})
endif()

if(USE_SPDLOG)
    CPMAddPackage(
        NAME spdlog
        GIT_TAG dev
        GITHUB_REPOSITORY R1NC/spdlog
    )
    list(APPEND INC_DIRS ${spdlog_SOURCE_DIR}/include)
endif()

# The buillt-in sqlite in iOS/macOS is available，no need to compile；
# For Android, it's only accessible in Java/kotlin: https://stackoverflow.com/questions/5523067/sqlite-with-android-ndk；
if(NOT APPLE)
    CPMAddPackage(
        NAME sqlite
        GIT_TAG dev
        GITHUB_REPOSITORY R1NC/sqlite
    )
    list(APPEND INC_DIRS ${sqlite_SOURCE_DIR})
endif()

# WASM has no built-in zlib;
if(EMSCRIPTEN OR WIN32)
    CPMAddPackage(
        NAME zlib
        GIT_TAG dev
        GITHUB_REPOSITORY R1NC/zlib
    )
    list(APPEND INC_DIRS ${zlib_SOURCE_DIR})
endif()

# HarmonyOS has built-in libuv;
if(USE_LIBUV AND NOT OHOS)
    CPMAddPackage(
        NAME libuv
        GIT_TAG dev
        GITHUB_REPOSITORY R1NC/libuv
    )
    list(APPEND INC_DIRS ${libuv_SOURCE_DIR}/include)
endif()

if(USE_ADA)
    CPMAddPackage(
        NAME adaurl
        GIT_TAG dev
        GITHUB_REPOSITORY R1NC/adaurl
    )
    list(APPEND INC_DIRS ${adaurl_SOURCE_DIR}/include)
endif()

if(ANDROID)
    list(APPEND INC_DIRS ${ifaddrs_SOURCE_DIR})
endif()

target_include_directories(${PROJECT_NAME} PRIVATE ${INC_DIRS})

## Link Libraries:

list(APPEND LINK_LIBS
    mmkv
    cjson
    OpenSSL::SSL 
    OpenSSL::Crypto
    CURL::libcurl
)

if(USE_LUA)
    list(APPEND LINK_LIBS lua)
endif()

if(USE_QJS)
    if(EMSCRIPTEN)
        list(APPEND LINK_LIBS qjs_wasm)
    else()
        list(APPEND LINK_LIBS quickjs)
    endif()
endif()

if(USE_SPDLOG)
    list(APPEND LINK_LIBS spdlog::spdlog)
endif()

if(APPLE)
    list(APPEND LINK_LIBS sqlite3)
else()
    list(APPEND LINK_LIBS sqlite3-static)
endif()

if(EMSCRIPTEN OR WIN32)
    list(APPEND LINK_LIBS zlib)
endif()

if(USE_LIBUV AND NOT OHOS)
    list(APPEND LINK_LIBS libuv)
endif()

if(USE_ADA)
    list(APPEND LINK_LIBS ada::ada)
endif()

target_link_libraries(${PROJECT_NAME} ${LINK_LIBS})


## Custom commands

execute_process(
    COMMAND git submodule update --init --recursive
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    RESULT_VARIABLE GIT_SUBMOD_RESULT
)

execute_process(
    COMMAND tsc
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/scripts/JS
    RESULT_VARIABLE TSC_RESULT
)

## Print Messages:

message("\n")
message(STATUS "${PROJECT_NAME}")
message(STATUS "  SYS_NAME: ${CMAKE_SYSTEM_NAME}")
message(STATUS "  IS_ROOT: ${IS_ROOT}")
message(STATUS "  USE_STD_TO_CHARS: ${USE_STD_TO_CHARS}")
message(STATUS "  USE_LUA: ${USE_LUA}")
message(STATUS "  USE_QJS: ${USE_QJS}")
message(STATUS "  USE_LIBUV: ${USE_LIBUV}")
message(STATUS "  USE_SPDLOG: ${USE_SPDLOG}")
message(STATUS "  USE_ADA: ${USE_ADA}")
message(STATUS "  LINK_LIBS:")
foreach(x IN LISTS LINK_LIBS)
    message(STATUS "    ${x}")
endforeach()
message("\n")
