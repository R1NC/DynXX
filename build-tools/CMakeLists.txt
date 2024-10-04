cmake_minimum_required(VERSION 3.6)

set(CMAKE_C_COMPILER "clang")
set(CMAKE_CXX_COMPILER "clang++")
set (CMAKE_C_STANDARD 99)
set (CMAKE_CXX_STANDARD 17)

option(USE_LUA "Use Lua or not" ON)

if(USE_LUA)
    add_definitions(-DUSE_LUA)
endif()

project(NGenXX)

set(ENGINE_FILES
    ../src/log/Log.cxx
    ../src/device/DeviceInfo-android.cxx
    ../src/device/DeviceInfo-harmony.cxx
    ../src/device/DeviceInfo-ios.mm
    ../src/device/DeviceInfo-mac.mm
    ../src/device/DeviceInfo-wasm.cxx
    ../src/device/DeviceInfo-windows.cxx
    ../src/device/DeviceInfo-linux.cxx
    ../src/net/HttpClient.cxx
    ../src/store/SQLite.cxx
    ../src/store/KV.cxx
    ../src/json/JsonDecoder.cxx
    ../src/zip/Zip.cxx
    ../src/coding/Coding.cxx
    ../src/crypto/Crypto.cxx
    ../src/NGenXX.cxx
)
if(USE_LUA)
    set(ENGINE_FILES ${ENGINE_FILES}
        ../src/lua/LuaBridge.cxx
        ../src/NGenXX-Lua.cxx
    )
endif()

set(LIB_TYPE STATIC)
if(EMSCRIPTEN)
    add_executable(${PROJECT_NAME} ${ENGINE_FILES})
    set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".html")
    # WARNING: Export with `EMSCRIPTEN_KEEPALIVE` will cause Lua running automatically.
    if(USE_LUA)
        set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "-s ALLOW_TABLE_GROWTH -s EXPORTED_RUNTIME_METHODS=['getValue','setValue','UTF8ToString','lengthBytesUTF8','stringToUTF8','addFunction'] -s EXPORTED_FUNCTIONS=['_malloc','_free','_ngenxx_init','_ngenxx_release','_ngenxx_L_loadS','_ngenxx_L_call']")
    else()
        set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "-s ALLOW_TABLE_GROWTH -s EXPORTED_RUNTIME_METHODS=['getValue','setValue','UTF8ToString','lengthBytesUTF8','stringToUTF8','addFunction'] -s EXPORTED_FUNCTIONS=['_malloc','_free','_ngenxx_init','_ngenxx_release']")
    endif()
else()
    add_library(${PROJECT_NAME} ${LIB_TYPE} ${ENGINE_FILES})
endif()

# WARNING:
# 1). The buillt-in sqlite in iOS/macOS is available，no need to compile；
# 2). The buillt-in sqlite in Android is only accessible in Java/kotlin: https://stackoverflow.com/questions/5523067/sqlite-with-android-ndk；
# 3). WASM has no built-in zlib;
add_subdirectory(../external/cjson cjson.output)
add_subdirectory(../external/mmkv/POSIX/src mmkv.output)
add_subdirectory(../external/openssl openssl.output)
add_subdirectory(../external/curl curl.output)
if(USE_LUA)
    add_subdirectory(../external/lua lua.output)
endif()
if(NOT APPLE)
    add_subdirectory(../external/sqlite sqlite.output)
endif()
if(EMSCRIPTEN)
    add_subdirectory(../external/zlib zlib.output)
endif()

set(LINK_LIBS
    mmkv
    cjson
    OpenSSL::SSL 
    OpenSSL::Crypto
    CURL::libcurl
)
if(USE_LUA)
    set(LINK_LIBS ${LINK_LIBS}
        lua
    )
endif()
if(APPLE)
    set(LINK_LIBS ${LINK_LIBS}
        sqlite3
    )
else()
    set(LINK_LIBS ${LINK_LIBS}
        sqlite3-static
    )
endif()
if(EMSCRIPTEN)
    set(LINK_LIBS ${LINK_LIBS}
        zlib
    )
endif()

target_link_libraries(${PROJECT_NAME} ${LINK_LIBS})
