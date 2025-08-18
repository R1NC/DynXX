cmake_minimum_required(VERSION 3.18)

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

project(DynXX)

# Enable Parallel Build
include(ProcessorCount)
ProcessorCount(N)
if(NOT N EQUAL 0)
    set(CMAKE_BUILD_PARALLEL_LEVEL ${N})
endif()

## Check Apple Version for C++ new features
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

option(USE_CURL "Use CURL" ON)
if(EMSCRIPTEN)
    set(USE_CURL FALSE)
endif()
if(USE_CURL)
    add_definitions(-DUSE_CURL)
endif()

option(USE_LUA "Use Lua" ON)
if(EMSCRIPTEN)
    set(USE_LUA FALSE)
endif()
if(USE_LUA)
    add_definitions(-DUSE_LUA)
endif()

option(USE_QJS "Use QuickJS" ON)
if(EMSCRIPTEN)
    set(USE_QJS FALSE)
endif()
if(USE_QJS)
    add_definitions(-DUSE_QJS -DCONFIG_BIGNUM)
endif()

option(USE_LIBUV "Use LibUV" ON)
if(EMSCRIPTEN)
    set(USE_LIBUV FALSE)
endif()
if(USE_LIBUV)
    add_definitions(-DUSE_LIBUV)
endif()

option(USE_KV "Use MMKV" ON)
if(EMSCRIPTEN)
    set(USE_KV FALSE)
endif()
if(USE_KV)
    add_definitions(-DUSE_KV)
endif()

option(USE_DB "Use SQLite" ON)
if(EMSCRIPTEN)
    set(USE_DB FALSE)
endif()
if(USE_DB)
    add_definitions(-DUSE_DB)
endif()

option(USE_SPDLOG "Use SpdLog" ON)
if(EMSCRIPTEN)
    set(USE_SPDLOG FALSE)
endif()
if(USE_SPDLOG)
    add_definitions(-DUSE_SPDLOG)
endif()

# ada uses `std::ranges` in C++20
set(USE_ADA FALSE)
checkAppleVersionLimit("16.3" "13.3" USE_ADA)
if(OHOS)
    set(USE_ADA FALSE)
endif()
if(USE_ADA)
    add_definitions(-DUSE_ADA)
endif()

## Source & Output:

file(GLOB_RECURSE SRC_FILES "src/*.cxx" "src/*.mm")
if(ANDROID) # Android can not find built-in `ifaddrs.h`;
    set(ANDROID_IFADDRS_ROOT third_party/android-ifaddrs)
    list(APPEND SRC_FILES ${ANDROID_IFADDRS_ROOT}/ifaddrs.c)
endif()

if(EMSCRIPTEN)
    add_executable(${PROJECT_NAME} ${SRC_FILES})
    set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".html")
    set(WASM_LINK_FLAGS "-s ALLOW_TABLE_GROWTH -s FETCH -s ASYNCIFY=1")
    set(WASM_EXPORTS_RUNTIME_METHODS "['getValue','setValue','UTF8ToString','lengthBytesUTF8','stringToUTF8','addFunction']")
    set(WASM_BASE_EXPORTS_FUNCS "'_malloc','_free','_dynxx_init','_dynxx_release'")
    if(USE_LUA)
        # WARNING: Export with `EMSCRIPTEN_KEEPALIVE` will cause Lua running automatically.
        set(WASM_EXTRA_EXPORTS_FUNCS ",'_dynxx_lua_loadS','_dynxx_lua_call'")
    else()
        set(WASM_EXTRA_EXPORTS_FUNCS "")
    endif()
    set(WASM_EXPORTED_FUNCS "[${WASM_BASE_EXPORTS_FUNCS}${WASM_EXTRA_EXPORTS_FUNCS}]")
    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "${WASM_LINK_FLAGS} -s EXPORTED_RUNTIME_METHODS=${WASM_EXPORTS_RUNTIME_METHODS} -s EXPORTED_FUNCTIONS=${WASM_EXPORTED_FUNCS}")
    message(STATUS "\nLINK_FLAGS:\n ${CMAKE_LINK_FLAGS}\n")
else()
    add_library(${PROJECT_NAME} STATIC ${SRC_FILES})
endif()

## Include Directories:

list(APPEND INC_DIRS include)

set(CJSON_ROOT third_party/cJSON)
add_subdirectory(${CJSON_ROOT} cjson-build)
list(APPEND INC_DIRS ${CJSON_ROOT})

if(USE_KV)
    set(MMKV_ROOT third_party/MMKV)
    add_subdirectory(${MMKV_ROOT}/POSIX/src mmkv-build)
    list(APPEND INC_DIRS ${MMKV_ROOT}/Core)
endif()

set(OPENSSL_ROOT third_party/openssl)
add_subdirectory(${OPENSSL_ROOT} openssl-build)
list(APPEND INC_DIRS ${OPENSSL_ROOT}/include)

## Config OpenSSL for libcurl to find libssl and libcrypto
set(OPENSSL_ROOT_DIR ${OPENSSL_ROOT})
set(OPENSSL_INCLUDE_DIR ${OPENSSL_ROOT_DIR}/include)
add_library(OpenSSL::SSL ALIAS ssl)
add_library(OpenSSL::Crypto ALIAS crypto)
set(OPENSSL_SSL_LIBRARY ssl)
set(OPENSSL_CRYPTO_LIBRARY crypto)

if(USE_CURL)
    set(CURL_ROOT third_party/curl)
    add_subdirectory(${CURL_ROOT} curl-build)
    list(APPEND INC_DIRS ${CURL_ROOT}/include)
endif()

if(USE_LUA)
    set(LUA_ROOT third_party/lua)
    add_subdirectory(${LUA_ROOT} lua-build)
    list(APPEND INC_DIRS ${LUA_ROOT})
endif()

if(USE_QJS)
    set(QJS_ROOT third_party/quickjs)
    add_subdirectory(${QJS_ROOT} quickjs-build)

    if(TARGET quickjs)
        target_compile_options(quickjs PRIVATE
            -Wno-error=conversion
            -Wno-error=old-style-cast
            -Wno-shorten-64-to-32
        )
    endif()
    
    list(APPEND INC_DIRS ${QJS_ROOT})
endif()

if(USE_SPDLOG)
    set(SPDLOG_ROOT third_party/spdlog)
    add_subdirectory(${SPDLOG_ROOT} spdlog-build)
    list(APPEND INC_DIRS ${SPDLOG_ROOT}/include)
endif()

# The buillt-in sqlite in iOS/macOS is available，no need to compile；
# For Android, it's only accessible in Java/kotlin: https://stackoverflow.com/questions/5523067/sqlite-with-android-ndk；
if(USE_DB AND NOT APPLE)
    set(SQLITE_ROOT third_party/sqlite)
    add_subdirectory(${SQLITE_ROOT} sqlite-build)
    list(APPEND INC_DIRS ${SQLITE_ROOT})
endif()

# WASM has no built-in zlib;
if(EMSCRIPTEN OR WIN32)
    set(Z_ROOT third_party/zlib)
    add_subdirectory(${Z_ROOT} zlib-build)
    list(APPEND INC_DIRS ${Z_ROOT})
endif()

# HarmonyOS has built-in libuv;
if(USE_LIBUV AND NOT OHOS)
    set(UV_ROOT third_party/libuv)
    add_subdirectory(${UV_ROOT} libuv-build)
    list(APPEND INC_DIRS ${UV_ROOT}/include)
endif()

if(USE_ADA)
    set(ADAURL_ROOT third_party/AdaURL)
    add_subdirectory(${ADAURL_ROOT} AdaURL-build)
    list(APPEND INC_DIRS ${ADAURL_ROOT}/include)
endif()

if(ANDROID)
    list(APPEND INC_DIRS ${ANDROID_IFADDRS_ROOT})
endif()

target_include_directories(${PROJECT_NAME} PRIVATE ${INC_DIRS})

## Link Libraries:

list(APPEND LINK_LIBS
    cjson
    OpenSSL::SSL 
    OpenSSL::Crypto
)

if(USE_CURL)
    list(APPEND LINK_LIBS CURL::libcurl)
endif()

if(USE_LUA)
    list(APPEND LINK_LIBS lua)
endif()

if(USE_QJS)
    list(APPEND LINK_LIBS quickjs)
endif()

if(USE_SPDLOG)
    list(APPEND LINK_LIBS spdlog::spdlog)
endif()

if(USE_KV)
    list(APPEND LINK_LIBS mmkv)
endif()

if(USE_DB)
    if(APPLE)
        list(APPEND LINK_LIBS sqlite3)
    else()
        list(APPEND LINK_LIBS sqlite3-static)
    endif()
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

## install headers

include(GNUInstallDirs)
install(DIRECTORY include/DynXX 
    DESTINATION include 
    COMPONENT headers
    FILES_MATCHING 
    PATTERN "*.h"
    PATTERN "*.hxx"
)

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
message(STATUS "  USE_CURL: ${USE_CURL}")
message(STATUS "  USE_LUA: ${USE_LUA}")
message(STATUS "  USE_QJS: ${USE_QJS}")
message(STATUS "  USE_KV: ${USE_KV}")
message(STATUS "  USE_DB: ${USE_DB}")
message(STATUS "  USE_LIBUV: ${USE_LIBUV}")
message(STATUS "  USE_SPDLOG: ${USE_SPDLOG}")
message(STATUS "  USE_ADA: ${USE_ADA}")
message(STATUS "  LINK_LIBS:")
foreach(x IN LISTS LINK_LIBS)
    message(STATUS "    ${x}")
endforeach()
message("\n")
