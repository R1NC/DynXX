cmake_minimum_required(VERSION 3.10)

project(DynXX VERSION 1.0)

set(CMAKE_C_COMPILER "clang")
set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${PROJECT_NAME} 
    main.cpp
    ../POSIX/DynXXPOSIX.cxx
)

get_filename_component(CURRENT_DIR "${CMAKE_CURRENT_SOURCE_DIR}" ABSOLUTE)
set(LIB_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../build.Linux/output/libs")

target_link_libraries(${PROJECT_NAME} PRIVATE
    ${LIB_PATH}/libDynXX-All.a
    z
)
