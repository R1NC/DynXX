cmake_minimum_required(VERSION 3.5.0)

project(dynxx)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(${PROJECT_SOURCE_DIR}/../../../../../../cmake/optimize.cmake)
dynxx_initialize_optimization_defaults()
dynxx_declare_optimization_options()

set(NATIVERENDER_ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR})

if(DEFINED PACKAGE_FIND_FILE)
    include(${PACKAGE_FIND_FILE})
endif()

include_directories(${NATIVERENDER_ROOT_PATH}
                    ${NATIVERENDER_ROOT_PATH}/include)

file(GLOB CXX_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cxx")
add_library(${CMAKE_PROJECT_NAME} SHARED ${CXX_SOURCES})
dynxx_apply_optimization_options(${CMAKE_PROJECT_NAME})

set(outputDir ${PROJECT_SOURCE_DIR}/../../../../../../build.HarmonyOS/Release/output)
set(libDir ${outputDir}/arm64-v8a/lib)

target_include_directories(${PROJECT_NAME} PUBLIC ${outputDir}/include)

#Find DynXX
set(DynXXPath ${libDir}/libDynXX-All.a)
add_library(libDynXX STATIC IMPORTED)
set_property(TARGET libDynXX PROPERTY IMPORTED_LOCATION ${DynXXPath})

target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC
    -Wl,--whole-archive
    libDynXX
    -Wl,--no-whole-archive
    z
    uv
    libace_napi.z.so
    libhilog_ndk.z.so
    libdeviceinfo_ndk.z.so
)
dynxx_apply_final_target_optimization(${CMAKE_PROJECT_NAME})
