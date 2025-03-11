cmake_minimum_required(VERSION 3.10)

project(NGenXX VERSION 1.0)

set(CMAKE_C_COMPILER "clang")
set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${PROJECT_NAME} 
    main.cpp
    ../POSIX/NGenXXPOSIX.cxx
)

get_filename_component(CURRENT_DIR "${CMAKE_CURRENT_SOURCE_DIR}" ABSOLUTE)
set(LIB_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../build.Linux/output/libs")

target_link_libraries(${PROJECT_NAME} PRIVATE
    ${LIB_PATH}/NGenXX.a
    ${LIB_PATH}/cjson.a
    ${LIB_PATH}/lua.a
    ${LIB_PATH}/quickjs.a
    ${LIB_PATH}/uv.a
    ${LIB_PATH}/sqlite3.a
    ${LIB_PATH}/curl.a
    ${LIB_PATH}/ssl.a
    ${LIB_PATH}/crypto.a
    ${LIB_PATH}/mmkv.a
    ${LIB_PATH}/mmkvcore.a
    z
)
