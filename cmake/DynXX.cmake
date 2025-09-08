## Initialize before project() call
function(initBeforeProject)
    # Use ccache to turbo compile speed
    find_program(CCACHE_FOUND ccache)
    if(CCACHE_FOUND)
        set(CMAKE_C_COMPILER_LAUNCHER ccache PARENT_SCOPE)
        set(CMAKE_CXX_COMPILER_LAUNCHER ccache PARENT_SCOPE)
    endif()
    
    # Check if this is the root project
    string(
        COMPARE EQUAL
        "${CMAKE_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}"
        IS_ROOT
    )
    set(IS_ROOT ${IS_ROOT} PARENT_SCOPE)
endfunction()

## Initialize after project() call
function(initAfterProject)
    # Enable Parallel Build
    include(ProcessorCount)
    ProcessorCount(N)
    if(NOT N EQUAL 0)
        set(CMAKE_BUILD_PARALLEL_LEVEL ${N} PARENT_SCOPE)
    endif()
endfunction()

## Check Apple Version for C++ new features
function(checkAppleVersionLimit iosV macV RESULT_VAR)
    if(APPLE AND ((CMAKE_SYSTEM_NAME STREQUAL "iOS" AND CMAKE_OSX_DEPLOYMENT_TARGET VERSION_LESS "${iosV}") OR 
                  (CMAKE_SYSTEM_NAME STREQUAL "Darwin" AND CMAKE_OSX_DEPLOYMENT_TARGET VERSION_LESS "${macV}")))
        set(${RESULT_VAR} FALSE PARENT_SCOPE)
    else()
        set(${RESULT_VAR} TRUE PARENT_SCOPE)
    endif()
endfunction()

## Add WebAssembly Executable
function(addWasmExe TARGET_NAME)
    set(options "")
    set(oneValueArgs LINK_FLAGS RUNTIME_METHODS FUNCS)
    set(multiValueArgs SRC_FILES)
    cmake_parse_arguments(WASM "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
    
    add_executable(${TARGET_NAME} ${SRC_FILES})
    
    set_target_properties(${TARGET_NAME} PROPERTIES PREFIX "" SUFFIX ".html")
    
    set(FINAL_LINK_FLAGS "-s ALLOW_TABLE_GROWTH")
    if(WASM_LINK_FLAGS)
        set(FINAL_LINK_FLAGS "${FINAL_LINK_FLAGS} ${WASM_LINK_FLAGS}")
    endif()
    
    set(FINAL_RUNTIME_METHODS "'getValue','setValue','UTF8ToString','lengthBytesUTF8','stringToUTF8'")
    if(WASM_RUNTIME_METHODS)
        set(FINAL_RUNTIME_METHODS "${FINAL_RUNTIME_METHODS},${WASM_RUNTIME_METHODS}")
    endif()

    set(FINAL_FUNCS "'_malloc','_free'")
    if(WASM_FUNCS)
        set(FINAL_FUNCS "${FINAL_FUNCS},${WASM_FUNCS}")
    endif()
    
    set_target_properties(${TARGET_NAME} PROPERTIES 
        LINK_FLAGS "${FINAL_LINK_FLAGS} -s EXPORTED_RUNTIME_METHODS=[${FINAL_RUNTIME_METHODS}] -s EXPORTED_FUNCTIONS=[${FINAL_FUNCS}]")
endfunction()

## Install Headers
function(installHeaders SOURCE_DIR DESTINATION_DIR)
    include(GNUInstallDirs)
    install(DIRECTORY "${SOURCE_DIR}"
        DESTINATION "${DESTINATION_DIR}"
        COMPONENT headers
        FILES_MATCHING
        PATTERN "*.h"
        PATTERN "*.hxx"
    )
endfunction()

## Print Build Information
function(printBuildInfo)
    message("")
    message("${PROJECT_NAME}")
    message(" └─ IS_ROOT: ${IS_ROOT}")
    message(" └─ CCACHE: ${CCACHE_FOUND}")
    message(" └─ CMAKE:")
    message("     └─ VERSION: ${CMAKE_VERSION}")
    message("     └─ HOST: ${CMAKE_HOST_SYSTEM_NAME} ${CMAKE_HOST_SYSTEM_VERSION} ${CMAKE_HOST_SYSTEM_PROCESSOR}")
    message("     └─ TARGET: ${CMAKE_SYSTEM_NAME} ${CMAKE_SYSTEM_VERSION} ${CMAKE_SYSTEM_PROCESSOR}")
    message("     └─ STANDARD:")
    message("         └─ C: ${CMAKE_C_STANDARD}")
    message("         └─ C++: ${CMAKE_CXX_STANDARD}")
    message("     └─ COMPILER:")
    message("         └─ C: ${CMAKE_C_COMPILER_ID} ${CMAKE_C_COMPILER_VERSION} (${CMAKE_C_COMPILER})")
    message("         └─ C++: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION} (${CMAKE_CXX_COMPILER})")
    message("     └─ LINKER: ${CMAKE_LINKER}")
    message("     └─ GENERATOR: ${CMAKE_GENERATOR}")
    message("     └─ MAKE_PROGRAM: ${CMAKE_MAKE_PROGRAM}")
    message("     └─ CROSS_COMPILING: ${CMAKE_CROSSCOMPILING}")
    if(CMAKE_TOOLCHAIN_FILE)
        message("     └─ TOOLCHAIN_FILE: ${CMAKE_TOOLCHAIN_FILE}")
    endif()
    message("     └─ BUILD:")
    message("         └─ TYPE: ${CMAKE_BUILD_TYPE}")
    message("         └─ PARALLEL_LEVEL: ${CMAKE_BUILD_PARALLEL_LEVEL}")
    if(APPLE)
        message(" └─ OSX:")
        message("     └─ DEPLOYMENT_TARGET: ${CMAKE_OSX_DEPLOYMENT_TARGET}")
        message("     └─ ARCHITECTURES: ${CMAKE_OSX_ARCHITECTURES}")
    endif()
    if(ANDROID)
        message("  └─ ANDROID:")
        message("     └─ PLATFORM: ${ANDROID_PLATFORM}")
        message("     └─ ABI: ${ANDROID_ABI}")
        message("     └─ NATIVE_API: ${ANDROID_NATIVE_API_LEVEL}")
    endif()
    if(EMSCRIPTEN)
        message(" └─ EMSCRIPTEN:")
        message("     └─ ABI: ${EMSCRIPTEN_SYSTEM_PROCESSOR}")
    endif()
    message(" └─ OPTIONS:")
    message("     └─ USE_STD_TO_CHARS: ${USE_STD_TO_CHARS}")
    message("     └─ USE_CURL: ${USE_CURL}")
    message("     └─ USE_LUA: ${USE_LUA}")
    message("     └─ USE_QJS: ${USE_QJS}")
    message("     └─ USE_KV: ${USE_KV}")
    message("     └─ USE_DB: ${USE_DB}")
    message("     └─ USE_DEVICE: ${USE_DEVICE}")
    message("     └─ USE_LIBUV: ${USE_LIBUV}")
    message("     └─ USE_SPDLOG: ${USE_SPDLOG}")
    message("     └─ USE_ADA: ${USE_ADA}")
    message(" └─ LINK_LIBS:")
    foreach(x IN LISTS LINK_LIBS)
        message("     └─ ${x}")
    endforeach()
    message("")
endfunction()