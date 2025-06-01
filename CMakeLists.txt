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

## Definitions:

option(USE_LUA "Use Lua" ON)
if(USE_LUA)
    add_definitions(-DUSE_LUA)
endif()

option(USE_QJS "Use QuickJS" ON)
if(USE_QJS)
    add_definitions(-DUSE_QJS -DCONFIG_BIGNUM)
endif()

if(EMSCRIPTEN)
    option(USE_LIBUV "Use LibUV" OFF)
else()
    option(USE_LIBUV "Use LibUV" ON)
    add_definitions(-DUSE_LIBUV)
endif()

option(USE_SPDLOG "Use SpdLog" ON)
if(USE_SPDLOG)
    add_definitions(-DUSE_SPDLOG)
endif()

# Support of `std::from_chars`(for integral types) in C++17
if(APPLE AND ((CMAKE_SYSTEM_NAME STREQUAL "iOS" AND CMAKE_OSX_DEPLOYMENT_TARGET VERSION_LESS "16.5") OR (CMAKE_SYSTEM_NAME STREQUAL "Darwin" AND CMAKE_OSX_DEPLOYMENT_TARGET VERSION_LESS "13.4")))
    option(USE_STD_FROM_CHARS "Use std::from_chars for int" OFF)
else()
    option(USE_STD_FROM_CHARS "Use std::from_chars for int" ON)
    add_definitions(-DUSE_STD_FROM_CHARS)
endif()

# Support of `std::from_chars`(for floating-point types) in C++17
option(USE_STD_FROM_CHARS_FLOAT "Use std::from_chars for float" OFF)
#add_definitions(-DUSE_STD_FROM_CHARS_FLOAT)

# Support of `std::format` in C++20
if(OHOS OR (APPLE AND ((CMAKE_SYSTEM_NAME STREQUAL "iOS" AND CMAKE_OSX_DEPLOYMENT_TARGET VERSION_LESS "16.3") OR (CMAKE_SYSTEM_NAME STREQUAL "Darwin" AND CMAKE_OSX_DEPLOYMENT_TARGET VERSION_LESS "13.3"))))
    option(USE_STD_FORMAT "Use std::format" OFF)
else()
    option(USE_STD_FORMAT "Use std::format" ON)
    add_definitions(-DUSE_STD_FORMAT)
endif()

# Support of `std::ranges` in C++20
if(OHOS OR (APPLE AND ((CMAKE_SYSTEM_NAME STREQUAL "iOS" AND CMAKE_OSX_DEPLOYMENT_TARGET VERSION_LESS "16.3") OR (CMAKE_SYSTEM_NAME STREQUAL "Darwin" AND CMAKE_OSX_DEPLOYMENT_TARGET VERSION_LESS "13.3"))))
    option(USE_STD_RANGES "Use std::ranges" OFF)
else()
    option(USE_STD_RANGES "Use std::ranges" ON)
    add_definitions(-DUSE_STD_RANGES)
endif()

# Support of `std::ranges::views::enumerate` in C++23
option(USE_STD_RANGES_ENUMERATE "std::ranges::views::enumerate" OFF)
#add_definitions(-DUSE_STD_RANGES_ENUMERATE)

# Support of `std::ranges::views::trunk` in C++23
option(USE_STD_RANGES_CHUNK "Use std::ranges::views::trunk" OFF)
#add_definitions(-DUSE_STD_RANGES_CHUNK)

## Source & Output:

file(GLOB_RECURSE SRC_FILES "src/*.cxx" "src/*.mm")
if(ANDROID) # Android can not find built-in `ifaddrs.h`;
    set(ANDROID_IFADDRS_ROOT external/android-ifaddrs)
    list(APPEND SRC_FILES ${ANDROID_IFADDRS_ROOT}/ifaddrs.c)
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

set(CJSON_ROOT external/cjson)
add_subdirectory(${CJSON_ROOT} cjson.output)
list(APPEND INC_DIRS ${CJSON_ROOT})

set(MMKV_ROOT external/mmkv)
add_subdirectory(${MMKV_ROOT}/POSIX/src mmkv.output)
list(APPEND INC_DIRS ${MMKV_ROOT}/Core)

set(OPENSSL_ROOT external/openssl)
add_subdirectory(${OPENSSL_ROOT} openssl.output)
list(APPEND INC_DIRS ${OPENSSL_ROOT}/include)

## Config OpenSSL for libcurl to find libssl and libcrypto
set(OPENSSL_ROOT_DIR ${OPENSSL_ROOT})
set(OPENSSL_INCLUDE_DIR ${OPENSSL_ROOT_DIR}/include)
add_library(OpenSSL::SSL ALIAS ssl)
add_library(OpenSSL::Crypto ALIAS crypto)
set(OPENSSL_SSL_LIBRARY ssl)
set(OPENSSL_CRYPTO_LIBRARY crypto)

set(CURL_ROOT external/curl)
add_subdirectory(${CURL_ROOT} curl.output)
list(APPEND INC_DIRS ${CURL_ROOT}/include)

if(USE_LUA)
    set(LUA_ROOT external/lua)
    add_subdirectory(${LUA_ROOT} lua.output)
    list(APPEND INC_DIRS ${LUA_ROOT})
endif()

if(USE_QJS)
    set(QJS_ROOT external/quickjs)
    add_subdirectory(${QJS_ROOT} quickjs.output)
    list(APPEND INC_DIRS ${QJS_ROOT})
endif()

if(USE_SPDLOG)
    set(SPDLOG_ROOT external/spdlog)
    add_subdirectory(${SPDLOG_ROOT} spdlog.output)
    list(APPEND INC_DIRS ${SPDLOG_ROOT}/include)
endif()

# The buillt-in sqlite in iOS/macOS is available，no need to compile；
# For Android, it's only accessible in Java/kotlin: https://stackoverflow.com/questions/5523067/sqlite-with-android-ndk；
if(NOT APPLE)
    set(SQLITE_ROOT external/sqlite)
    add_subdirectory(${SQLITE_ROOT} sqlite.output)
    list(APPEND INC_DIRS ${SQLITE_ROOT})
endif()

# WASM has no built-in zlib;
if(EMSCRIPTEN OR WIN32)
    set(Z_ROOT external/zlib)
    add_subdirectory(${Z_ROOT} zlib.output)
    list(APPEND INC_DIRS ${Z_ROOT})
endif()

# HarmonyOS has built-in libuv;
if(USE_LIBUV AND NOT OHOS)
    set(UV_ROOT external/libuv)
    add_subdirectory(${UV_ROOT} libuv.output)
    list(APPEND INC_DIRS ${UV_ROOT}/include)
endif()

if(USE_STD_RANGES)
    set(ADAURL_ROOT external/AdaURL)
    add_subdirectory(${ADAURL_ROOT} AdaURL.output)
    list(APPEND INC_DIRS ${ADAURL_ROOT}/include)
endif()

if(ANDROID)
    list(APPEND INC_DIRS ${ANDROID_IFADDRS_ROOT})
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

if(USE_STD_RANGES)
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
message(STATUS "  USE_LUA: ${USE_LUA}")
message(STATUS "  USE_QJS: ${USE_QJS}")
message(STATUS "  USE_LIBUV: ${USE_LIBUV}")
message(STATUS "  USE_SPDLOG: ${USE_SPDLOG}")
message(STATUS "  USE_STD_FROM_CHARS: ${USE_STD_FROM_CHARS}")
message(STATUS "  USE_STD_FROM_CHARS_FLOAT: ${USE_STD_FROM_CHARS_FLOAT}")
message(STATUS "  USE_STD_FORMAT: ${USE_STD_FORMAT}")
message(STATUS "  USE_STD_RANGES: ${USE_STD_RANGES}")
message(STATUS "  LINK_LIBS:")
foreach(x IN LISTS LINK_LIBS)
    message(STATUS "    ${x}")
endforeach()
message("\n")
