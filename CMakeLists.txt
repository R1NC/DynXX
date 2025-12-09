cmake_minimum_required(VERSION 3.20)

include(cmake/DynXX.cmake)

initBeforeProject()

project(DynXX)

initAfterProject()

## Declare Options:

option(USE_VCPKG "Use vcpkg" ON)
if(OHOS)
    set(USE_VCPKG OFF)
endif()

set(USE_STD_TO_CHARS ON)
checkAppleVersionLimit("16.5" "13.4" USE_STD_TO_CHARS)

option(USE_CURL "Use CURL" ON)
if(EMSCRIPTEN)
    set(USE_CURL OFF)
endif()

option(USE_LUA "Use Lua" ON)
if(EMSCRIPTEN)
    set(USE_LUA OFF)
endif()

option(USE_QJS "Use QuickJS" ON)
if(EMSCRIPTEN)
    set(USE_QJS OFF)
endif()

option(USE_KV "Use MMKV" ON)
if(EMSCRIPTEN)
    set(USE_KV OFF)
endif()

option(USE_DB "Use SQLite" ON)
if(EMSCRIPTEN)
    set(USE_DB OFF)
endif()

option(USE_DEVICE "Use Device" ON)
if(EMSCRIPTEN)
    set(USE_DEVICE OFF)
endif()

option(USE_LIBUV "Use LibUV" ON)
if(EMSCRIPTEN)
    set(USE_LIBUV OFF)
endif()

option(USE_SPDLOG "Use SpdLog" ON)
if(EMSCRIPTEN)
    set(USE_SPDLOG OFF)
endif()

# ada uses `std::ranges` in C++20
set(USE_ADA OFF)
checkAppleVersionLimit("16.3" "13.3" USE_ADA)
if(OHOS OR EMSCRIPTEN)
    set(USE_ADA OFF)
endif()

## Source & Output:

set(SRC_FILES "")

file(GLOB_RECURSE CPP_FILES "src/*.cxx")
list(APPEND SRC_FILES ${CPP_FILES})

if(APPLE)
    file(GLOB_RECURSE OBJCXX_FILES "src/*.mm")
    list(APPEND SRC_FILES ${OBJCXX_FILES})
    set_source_files_properties(${OBJCXX_FILES} PROPERTIES
        COMPILE_FLAGS "-fobjc-arc"
    )
endif()

if(EMSCRIPTEN)
    set(EXPORT_FUNCS "'_dynxx_init','_dynxx_release'")
    if(USE_LUA)
        set(EXPORT_FUNCS "${EXPORT_FUNCS},'_dynxx_lua_loadS','_dynxx_lua_call'")
    endif()
    addWasmExe(${PROJECT_NAME} 
        SRC_FILES ${SRC_FILES} 
        LINK_FLAGS "-s FETCH -s ASYNCIFY=1"
        RUNTIME_METHODS "'addFunction'"
        FUNCS ${EXPORT_FUNCS}
    )
else()
    add_library(${PROJECT_NAME} STATIC ${SRC_FILES})
endif()

## Add Common Definitions:

addDefinitions(${PROJECT_NAME} ON
    USE_VCPKG
    USE_STD_TO_CHARS
    USE_CURL
    USE_LUA
    USE_QJS
    USE_KV
    USE_DB
    USE_DEVICE
    USE_ADA
    USE_LIBUV
    USE_SPDLOG
)

## Add Definitions for Windows:

if(WIN32)
    addDefinitions(${PROJECT_NAME} OFF
        NOMINMAX
        WIN32_LEAN_AND_MEAN
    )
endif()

## Add Compile Options:

if(WIN32)
    # force UTF-8
    if(MSVC OR (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_CXX_SIMULATE_ID STREQUAL "MSVC"))
        target_compile_options(${PROJECT_NAME} PRIVATE /utf-8)
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        target_compile_options(${PROJECT_NAME} PRIVATE -finput-charset=UTF-8)
    endif()
endif()

## Include Directories:

target_include_directories(${PROJECT_NAME} PRIVATE include)

## Setup dependencies:

if(USE_VCPKG)
    find_package(cJSON CONFIG REQUIRED)
else()
    addGitLib(${PROJECT_NAME}
        cjson
        https://github.com/R1NC/cJSON
        dev
    )
endif()
list(APPEND LINK_LIBS cjson)

if(USE_VCPKG)
    find_package(OpenSSL REQUIRED)
else()
    addGitLib(${PROJECT_NAME}
        openssl
        https://github.com/R1NC/openssl
        dev
        "include"
    )
    ## Config OpenSSL for libcurl to find libssl and libcrypto
    set(OPENSSL_ROOT_DIR ${openssl_SRC_PATH})
    set(OPENSSL_INCLUDE_DIR ${openssl_INC_PATH})
    add_library(OpenSSL::SSL ALIAS ssl)
    add_library(OpenSSL::Crypto ALIAS crypto)
    set(OPENSSL_SSL_LIBRARY ssl)
    set(OPENSSL_CRYPTO_LIBRARY crypto)
endif()
list(APPEND LINK_LIBS OpenSSL::SSL OpenSSL::Crypto)

if(USE_CURL)
    if(USE_VCPKG)
        find_package(CURL REQUIRED)
    else()
        addGitLib(${PROJECT_NAME}
            curl
            https://github.com/R1NC/curl
            dev
            "include"
        )
    endif()
    list(APPEND LINK_LIBS CURL::libcurl)
endif()

# Windows & WASM have no built-in zlib;
if(WIN32 OR EMSCRIPTEN)
    if(USE_VCPKG)
        find_package(ZLIB REQUIRED)
        add_library(zlib ALIAS ZLIB::ZLIB)
    else()
        addGitLib(${PROJECT_NAME}
            zlib
            https://github.com/R1NC/zlib
            dev
        )
    endif()
    list(APPEND LINK_LIBS zlib)
endif()

if(USE_LUA)
    if(USE_VCPKG)
        find_package(Lua REQUIRED)
    else()
        addGitLib(${PROJECT_NAME}
            lua
            https://github.com/R1NC/lua
            dev
        )
    endif()
    list(APPEND LINK_LIBS lua)
endif()

if(USE_QJS)
    addGitLib(${PROJECT_NAME}
        quickjs
        https://github.com/R1NC/quickjs-ng
        dev
    )
    list(APPEND LINK_LIBS qjs)
endif()

if(USE_KV)
    addGitLib(${PROJECT_NAME}
        mmkv
        https://github.com/R1NC/MMKV
        dev
        "Core"
        "POSIX/src"
        ON
    )
    list(APPEND LINK_LIBS mmkv)
endif()

# The buillt-in sqlite in iOS/macOS is available，no need to compile；
# For Android, it's only accessible in Java/kotlin: https://stackoverflow.com/questions/5523067/sqlite-with-android-ndk；
if(USE_DB)
    if(NOT APPLE)
        if(USE_VCPKG)
            find_package(unofficial-sqlite3 CONFIG REQUIRED)
            add_library(sqlite3 ALIAS unofficial::sqlite3::sqlite3)
        else()
            addGitLib(${PROJECT_NAME}
                sqlite
                https://github.com/R1NC/sqlite
                dev
            )
            add_library(sqlite3 ALIAS sqlite3-static)
        endif()
    endif()
    list(APPEND LINK_LIBS sqlite3)
endif()

# HarmonyOS has built-in libuv;
if(USE_LIBUV AND NOT OHOS)
    if(USE_VCPKG)
        find_package(libuv REQUIRED)
        list(APPEND LINK_LIBS $<IF:$<TARGET_EXISTS:libuv::uv_a>,libuv::uv_a,libuv::uv>)
    else()
        addGitLib(${PROJECT_NAME}
            libuv
            https://github.com/R1NC/libuv
            dev
            "include"
        )
        list(APPEND LINK_LIBS libuv)
    endif()
endif()

if(USE_ADA)
    if(USE_VCPKG)
        find_package(ada REQUIRED)
    else()
        addGitLib(${PROJECT_NAME}
            ada
            https://github.com/R1NC/AdaURL
            dev
            "include"
        )
    endif()
    list(APPEND LINK_LIBS ada::ada)
endif()

if(USE_SPDLOG)
    if(USE_VCPKG)
        find_package(spdlog REQUIRED)
    else()
        addGitLib(${PROJECT_NAME}
            spdlog
            https://github.com/R1NC/spdlog
            dev
            "include"
        )
    endif()
endif()

if(ANDROID)
    list(APPEND LINK_LIBS 
        android
        log
        z
    )
elseif(APPLE)
    list(APPEND LINK_LIBS 
        apple_nghttp2
        z
    )
    if(USE_DB)
        list(APPEND LINK_LIBS 
            sqlite3
        )
    endif()
elseif(WIN32)
    list(APPEND LINK_LIBS 
        ws2_32 
        Wldap32
    )
elseif(OHOS)
    list(APPEND LINK_LIBS
        libace_napi.z.so
        libhilog_ndk.z.so
        libdeviceinfo_ndk.z.so
        z
        uv
    )
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE ${LINK_LIBS})

## install headers

installHeaders(include/DynXX include)

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
message("")
message("${PROJECT_NAME}")
printBaseBuildInfo()
message(" └─ OPTIONS:")
message("     └─ USE_STD_TO_CHARS: ${USE_STD_TO_CHARS}")
message("     └─ USE_CURL: ${USE_CURL}")
message("     └─ USE_LUA: ${USE_LUA}")
message("     └─ USE_QJS: ${USE_QJS}")
message("     └─ USE_KV: ${USE_KV}")
message("     └─ USE_DB: ${USE_DB}")
message("     └─ USE_DEVICE: ${USE_DEVICE}")
message("     └─ USE_LIBUV: ${USE_LIBUV}")
message("     └─ USE_SPDLOG: ${USE_SPDLOG}")
message("     └─ USE_ADA: ${USE_ADA}")
message(" └─ LINK_LIBS:")
foreach(x IN LISTS LINK_LIBS)
    message("     └─ ${x}")
endforeach()
message("")
