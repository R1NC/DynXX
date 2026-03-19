cmake_minimum_required(VERSION 3.22.1)

project("DynXXJNI")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB CXX_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cxx")
add_library(${CMAKE_PROJECT_NAME} SHARED ${CXX_SOURCES})

set(outputDir ${PROJECT_SOURCE_DIR}/../../../../../../build.Android/Release/output)
set(libDir ${outputDir}/${ANDROID_ABI}/lib)

target_include_directories(${PROJECT_NAME} PUBLIC ${outputDir}/include)

#Find libDynXX
set(libDynXXPath ${libDir}/libDynXX-All.a)
add_library(libDynXX STATIC IMPORTED)
set_property(TARGET libDynXX PROPERTY IMPORTED_LOCATION ${libDynXXPath})

target_link_libraries(${CMAKE_PROJECT_NAME}
        libDynXX
        z
        android
        log
)
