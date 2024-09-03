cmake_minimum_required(VERSION 3.6)

set(CMAKE_C_COMPILER "clang")
set(CMAKE_CXX_COMPILER "clang++")
set (CMAKE_C_STANDARD 99)
set (CMAKE_CXX_STANDARD 11)
#add_compile_options("-stdlib=libc++" "-lc++abi" "-v")

project(NGenXX)

set(ENGINE_FILES
    ../src/log/Log.cxx
    ../src/net/HttpClient.cxx
    ../src/lua/LuaBridge.cxx
    ../src/NGenXX.cxx
)

set(LIB_TYPE STATIC)
if(EMSCRIPTEN)
    add_executable(${PROJECT_NAME} ${ENGINE_FILES})
    set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".html")
    # WARNING: Export with `EMSCRIPTEN_KEEPALIVE` will cause Lua running automatically.
    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "-s ALLOW_TABLE_GROWTH -s EXPORTED_RUNTIME_METHODS=['getValue','setValue','UTF8ToString','lengthBytesUTF8','stringToUTF8','addFunction'] -s EXPORTED_FUNCTIONS=['_malloc','_free','_ngenxx_init','_ngenxx_release','_ngenxx_L_loadS','_ngenxx_L_call']")
else()
    add_library(${PROJECT_NAME} ${LIB_TYPE} ${ENGINE_FILES})
endif()

# WARNING:
# 1). WolfSSL is not available in HarmonyOS，use OpenSSL instead;
# 2). The buillt-in sqlite in iOS/macOS is available，no need to compile；
# 3). The buillt-in sqlite in Android is only accessible in Java/kotlin: https://stackoverflow.com/questions/5523067/sqlite-with-android-ndk；
add_subdirectory(../external/lua lua.output)
add_subdirectory(../external/cjson cjson.output)
if(NOT APPLE)
    add_subdirectory(../external/sqlite sqlite.output)
endif()
if(OHOS)
    add_subdirectory(../external/openssl openssl.output)
else()
    add_subdirectory(../external/wolfssl wolfssl.output)
endif()
add_subdirectory(../external/curl curl.output)

if(OHOS)
    set(LINK_LIBS
        lua
        cjson
        sqlite3-static
        CURL::libcurl 
        OpenSSL::SSL 
        OpenSSL::Crypto
    )
elseif(APPLE)
    set(LINK_LIBS
        lua
        cjson
        sqlite3
        CURL::libcurl
        wolfssl::wolfssl
    )
else()
    set(LINK_LIBS
        lua
        cjson
        sqlite3-static
        CURL::libcurl
        wolfssl::wolfssl
    )
endif()

target_link_libraries(${PROJECT_NAME} ${LINK_LIBS})
