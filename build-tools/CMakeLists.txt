cmake_minimum_required(VERSION 3.6)

set(CMAKE_C_COMPILER "clang")
set(CMAKE_CXX_COMPILER "clang++")
set (CMAKE_CXX_STANDARD 11)
#add_compile_options("-stdlib=libc++" "-lc++abi" "-v")

project(EngineXX)

set(ENGINE_FILES
    ../src/net/HttpClient.cxx
    ../src/lua/LuaBridge.cxx
    ../src/EngineXX.cxx
)

set(LIB_TYPE STATIC)
add_library(${PROJECT_NAME} ${LIB_TYPE} ${ENGINE_FILES})

add_subdirectory(../external/lua lua.output)
add_subdirectory(../external/cjson cjson.output)
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
        CURL::libcurl 
        OpenSSL::SSL 
        OpenSSL::Crypto
    )
else()
    set(LINK_LIBS
        lua
        cjson
        CURL::libcurl
        wolfssl::wolfssl
    )
endif()

target_link_libraries(${PROJECT_NAME} ${LINK_LIBS})
