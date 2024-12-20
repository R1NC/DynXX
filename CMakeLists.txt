cmake_minimum_required(VERSION 3.10)

string(
    COMPARE EQUAL
    "${CMAKE_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}"
    IS_ROOT
)

if(IS_ROOT)
    set(CMAKE_C_COMPILER "clang")
    set(CMAKE_CXX_COMPILER "clang++")
    set(CMAKE_C_STANDARD 99)
    set(CMAKE_CXX_STANDARD 20)
endif()

project(NGenXX)

option(USE_LUA "Use Lua or not" ON)
option(USE_QJS "Use QuickJS or not" ON)
if(USE_LUA)
    add_definitions(-DUSE_LUA)
endif()
if(USE_QJS)
    add_definitions(-DUSE_QJS -DCONFIG_BIGNUM)
endif()
message("\n")
message(STATUS "${PROJECT_NAME}")
message(STATUS "  SYS_NAME: ${CMAKE_SYSTEM_NAME}")
message(STATUS "  IS_ROOT: ${IS_ROOT}")
message(STATUS "  USE_LUA: ${USE_LUA}")
message(STATUS "  USE_QJS: ${USE_QJS}")
message("\n")

list(APPEND INC_DIRS include)
list(APPEND SRC_FILES
    src/log/Log.cxx
    src/device/DeviceInfo-android.cxx
    src/device/DeviceInfo-harmony.cxx
    src/device/DeviceInfo-ios.mm
    src/device/DeviceInfo-mac.mm
    src/device/DeviceInfo-wasm.cxx
    src/device/DeviceInfo-windows.cxx
    src/device/DeviceInfo-linux.cxx
    src/net/HttpClient.cxx
    src/store/SQLite.cxx
    src/store/KV.cxx
    src/json/JsonDecoder.cxx
    src/zip/Zip.cxx
    src/coding/Coding.cxx
    src/crypto/Crypto-OpenSSL.cxx
    src/NGenXX-X.cxx
    src/NGenXX-Script.cxx
    src/NGenXX-C.cxx
)
if(USE_LUA)
    list(APPEND SRC_FILES
        src/lua/LuaBridge.cxx
        src/NGenXX-Lua.cxx
    )
endif()
if(USE_QJS)
    list(APPEND SRC_FILES
        src/js/JsBridge.cxx
        src/NGenXX-Js.cxx
    )
endif()
message("\n")
message(STATUS "SRC_FILES:")
foreach(x IN LISTS SRC_FILES)
    message(STATUS "  ${x}")
endforeach()
message("\n")

set(LIB_TYPE STATIC)
if(EMSCRIPTEN)
    add_executable(${PROJECT_NAME} ${SRC_FILES})
    set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".html")
    # WARNING: Export with `EMSCRIPTEN_KEEPALIVE` will cause Lua running automatically.
    if(USE_LUA)
        set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "-s ALLOW_TABLE_GROWTH -s EXPORTED_RUNTIME_METHODS=['getValue','setValue','UTF8ToString','lengthBytesUTF8','stringToUTF8','addFunction'] -s EXPORTED_FUNCTIONS=['_malloc','_free','_ngenxx_init','_ngenxx_release','_ngenxx_lua_loadS','_ngenxx_lua_call']")
    else()
        set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "-s ALLOW_TABLE_GROWTH -s EXPORTED_RUNTIME_METHODS=['getValue','setValue','UTF8ToString','lengthBytesUTF8','stringToUTF8','addFunction'] -s EXPORTED_FUNCTIONS=['_malloc','_free','_ngenxx_init','_ngenxx_release']")
    endif()
else()
    add_library(${PROJECT_NAME} ${LIB_TYPE} ${SRC_FILES})
endif()

# WARNING:
# 1). The buillt-in sqlite in iOS/macOS is available，no need to compile；
# 2). The buillt-in sqlite in Android is only accessible in Java/kotlin: https://stackoverflow.com/questions/5523067/sqlite-with-android-ndk；
# 3). WASM has no built-in zlib;
# 4). HarmonyOS has built-in libuv;

set(CJSON_ROOT external/cjson)
add_subdirectory(${CJSON_ROOT} cjson.output)
list(APPEND INC_DIRS ${CJSON_ROOT})

set(MMKV_ROOT external/mmkv)
add_subdirectory(${MMKV_ROOT}/POSIX/src mmkv.output)
list(APPEND INC_DIRS ${MMKV_ROOT}/Core)

set(OPENSSL_ROOT external/openssl)
add_subdirectory(${OPENSSL_ROOT} openssl.output)
list(APPEND INC_DIRS ${OPENSSL_ROOT}/include)

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

if(NOT APPLE)
    set(SQLITE_ROOT external/sqlite)
    add_subdirectory(${SQLITE_ROOT} sqlite.output)
    list(APPEND INC_DIRS ${SQLITE_ROOT})
endif()

if(EMSCRIPTEN)
    set(Z_ROOT external/zlib)
    add_subdirectory(${Z_ROOT} zlib.output)
    list(APPEND INC_DIRS ${Z_ROOT})
endif()

if(NOT OHOS)
    set(UV_ROOT external/libuv)
    add_subdirectory(${UV_ROOT} libuv.output)
    list(APPEND INC_DIRS ${UV_ROOT}/include)
endif()

include_directories(${INC_DIRS})
message("\n")
message(STATUS "INC_DIRS:")
foreach(x IN LISTS INC_DIRS)
    message(STATUS "  ${x}")
endforeach()
message("\n")

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
    list(APPEND LINK_LIBS quickjs)
endif()
if(APPLE)
    list(APPEND LINK_LIBS sqlite3)
else()
    list(APPEND LINK_LIBS sqlite3-static)
endif()
if(EMSCRIPTEN)
    list(APPEND LINK_LIBS zlib)
endif()
if(NOT OHOS)
    list(APPEND LINK_LIBS libuv)
endif()
message("\n")
message(STATUS "LINK_LIBS:")
foreach(x IN LISTS LINK_LIBS)
    message(STATUS "  ${x}")
endforeach()
message("\n")

target_link_libraries(${PROJECT_NAME} ${LINK_LIBS})
