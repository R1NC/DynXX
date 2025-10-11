cmake_minimum_required(VERSION 3.18)

include(cmake/DynXX.cmake)

#if(IS_ROOT)
    set(CMAKE_C_COMPILER "clang")
    set(CMAKE_CXX_COMPILER "clang++")
    set(CMAKE_C_STANDARD 99)
    set(CMAKE_CXX_STANDARD 23)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
#endif()

initBeforeProject()

project(DynXX)

initAfterProject()

## Definitions:

set(USE_STD_TO_CHARS OFF)
checkAppleVersionLimit("16.5" "13.4" USE_STD_TO_CHARS)
if(USE_STD_TO_CHARS)
    add_definitions(-DUSE_STD_TO_CHARS)
endif()

option(USE_CURL "Use CURL" ON)
if(EMSCRIPTEN)
    set(USE_CURL OFF)
endif()
if(USE_CURL)
    add_definitions(-DUSE_CURL)
endif()

option(USE_LUA "Use Lua" ON)
if(EMSCRIPTEN)
    set(USE_LUA OFF)
endif()
if(USE_LUA)
    add_definitions(-DUSE_LUA)
endif()

option(USE_QJS "Use QuickJS" ON)
if(EMSCRIPTEN)
    set(USE_QJS OFF)
endif()
if(USE_QJS)
    add_definitions(-DUSE_QJS -DCONFIG_BIGNUM)
endif()

option(USE_LIBUV "Use LibUV" ON)
if(EMSCRIPTEN)
    set(USE_LIBUV OFF)
endif()
if(USE_LIBUV)
    add_definitions(-DUSE_LIBUV)
endif()

option(USE_KV "Use MMKV" ON)
if(EMSCRIPTEN)
    set(USE_KV OFF)
endif()
if(USE_KV)
    add_definitions(-DUSE_KV)
endif()

option(USE_DB "Use SQLite" ON)
if(EMSCRIPTEN)
    set(USE_DB OFF)
endif()
if(USE_DB)
    add_definitions(-DUSE_DB)
endif()

option(USE_DEVICE "Use Device" ON)
if(EMSCRIPTEN)
    set(USE_DEVICE OFF)
endif()
if(USE_DEVICE)
    add_definitions(-DUSE_DEVICE)
endif()

option(USE_SPDLOG "Use SpdLog" ON)
if(EMSCRIPTEN)
    set(USE_SPDLOG OFF)
endif()
if(USE_SPDLOG)
    add_definitions(-DUSE_SPDLOG)
endif()

# ada uses `std::ranges` in C++20
set(USE_ADA OFF)
checkAppleVersionLimit("16.3" "13.3" USE_ADA)
if(OHOS)
    set(USE_ADA OFF)
endif()
if(USE_ADA)
    add_definitions(-DUSE_ADA)
endif()

# vcpkg doesn't support ohos
option(USE_VCPKG "Use vcpkg" ON)
if(OHOS)
    set(USE_VCPKG OFF)
endif()
if(USE_VCPKG)
    add_definitions(-DUSE_VCPKG)
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

if(ANDROID) # Android can not find built-in `ifaddrs.h`;
    set(ANDROID_IFADDRS_ROOT third_party/android-ifaddrs)
    list(APPEND SRC_FILES ${ANDROID_IFADDRS_ROOT}/ifaddrs.c)
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

## Include Directories:

target_include_directories(${PROJECT_NAME} PRIVATE include)

## Find dependencies:

if(USE_VCPKG)
    find_package(cJSON CONFIG REQUIRED)
else()
    addGitLib(${PROJECT_NAME}
        cjson
        https://github.com/R1NC/cJSON
        dev
    )
endif()

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

if(USE_KV)
    addGitLib(${PROJECT_NAME}
        mmkv
        https://github.com/R1NC/MMKV
        dev
        "Core"
        "POSIX/src"
        ON
    )
endif()

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
endif()

if(USE_QJS)
    addGitLib(${PROJECT_NAME}
        quickjs
        https://github.com/R1NC/quickjs-ng
        dev
    )
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

# The buillt-in sqlite in iOS/macOS is available，no need to compile；
# For Android, it's only accessible in Java/kotlin: https://stackoverflow.com/questions/5523067/sqlite-with-android-ndk；
if(USE_DB AND NOT APPLE)
    if(USE_VCPKG)
        find_package(unofficial-sqlite3 CONFIG REQUIRED)
    else()
        addGitLib(${PROJECT_NAME}
            sqlite
            https://github.com/R1NC/sqlite
            dev
        )
    endif()
endif()

# Windows & WASM have no built-in zlib;
if(WIN32 OR EMSCRIPTEN)
    if(USE_VCPKG)
        find_package(ZLIB REQUIRED)
    else()
        addGitLib(${PROJECT_NAME}
            zlib
            https://github.com/R1NC/zlib
            dev
        )
    endif()
endif()

# HarmonyOS has built-in libuv;
if(USE_LIBUV AND NOT OHOS)
    if(USE_VCPKG)
        find_package(libuv REQUIRED)
    else()
        addGitLib(${PROJECT_NAME}
            libuv
            https://github.com/R1NC/libuv
            dev
            "include"
        )
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
endif()

## Link Libraries:

list(APPEND LINK_LIBS
    OpenSSL::SSL 
    OpenSSL::Crypto
    cjson
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
    elseif(USE_VCPKG)
        list(APPEND LINK_LIBS unofficial::sqlite3::sqlite3)
    else()
        list(APPEND LINK_LIBS sqlite3-static)
    endif()
endif()

if(WIN32 OR EMSCRIPTEN)
    if(USE_VCPKG)
        list(APPEND LINK_LIBS ZLIB::ZLIB)
    else()
        list(APPEND LINK_LIBS zlib)
    endif()
endif()

if(USE_LIBUV AND NOT OHOS)
    if(USE_VCPKG)
        list(APPEND LINK_LIBS $<IF:$<TARGET_EXISTS:uv>,uv,uv_a>)
    else()
        list(APPEND LINK_LIBS libuv)
    endif()
endif()

if(USE_ADA)
    list(APPEND LINK_LIBS ada::ada)
endif()

target_link_libraries(${PROJECT_NAME} ${LINK_LIBS})

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
