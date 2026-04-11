cmake_minimum_required(VERSION 3.20)

include(GoogleTest)

find_package(GTest CONFIG REQUIRED)

set(DYNXX_TEST_SOURCES
    Coding.test.cxx
    Crypto.test.cxx
    DynXX.test.cxx
    Json.test.cxx
    Log.test.cxx
    Memory.test.cxx
    Net.test.cxx
    Zip.test.cxx
)

if(USE_DEVICE)
    list(APPEND DYNXX_TEST_SOURCES Device.test.cxx)
endif()

if(USE_QJS)
    list(APPEND DYNXX_TEST_SOURCES JS.test.cxx)
endif()

if(USE_KV)
    list(APPEND DYNXX_TEST_SOURCES KV.test.cxx)
endif()

if(USE_LUA)
    list(APPEND DYNXX_TEST_SOURCES Lua.test.cxx)
endif()

if(USE_DB)
    list(APPEND DYNXX_TEST_SOURCES SQLite.test.cxx)
endif()

add_library(DynXXTest STATIC ${DYNXX_TEST_SOURCES})

target_link_libraries(DynXXTest
    PUBLIC
        DynXX
        GTest::gtest
)

target_include_directories(DynXXTest
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
)

install_headers(${CMAKE_CURRENT_SOURCE_DIR}/include/ include)

if(ANDROID OR IOS OR EMSCRIPTEN OR CMAKE_SYSTEM_NAME STREQUAL "OHOS")
    message(STATUS "Skip DynXX test executable on this target platform")
    return()
endif()

add_executable(DynXXCxxTests
    DynXXTestMain.cxx
    ${DYNXX_TEST_SOURCES}
)

target_link_libraries(DynXXCxxTests
    PRIVATE
        DynXXTest
)

target_include_directories(DynXXCxxTests
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
)

add_test(NAME DynXXCxxTests COMMAND DynXXCxxTests)

set_tests_properties(DynXXCxxTests
    PROPERTIES
        TIMEOUT 300
)
