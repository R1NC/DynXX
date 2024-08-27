cmake_minimum_required(VERSION 3.10)
project(demo.linux VERSION 0.1)

set(CMAKE_C_COMPILER "clang")
set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(${PROJECT_NAME} main.cpp)

get_filename_component(CURRENT_DIR "${CMAKE_CURRENT_SOURCE_DIR}" ABSOLUTE)
set(STATIC_LIB_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../build.Linux/output/Release")

target_link_libraries(${PROJECT_NAME} PRIVATE
    ${STATIC_LIB_PATH}/EngineXX.a
    ${STATIC_LIB_PATH}/curl.a
    ${STATIC_LIB_PATH}/cjson.a
    ${STATIC_LIB_PATH}/lua.a
    ${STATIC_LIB_PATH}/wolfssl.a
    z
)
