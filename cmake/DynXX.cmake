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

    # Enable FetchContent
    include(FetchContent)
    set(FETCHCONTENT_UPDATES_DISCONNECTED ON)
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

## Add lib from Git
function(addGitLib name url tag src_dir inc_dir _target manual_add)
    FetchContent_Declare(
        ${name}
        GIT_REPOSITORY ${url}
        GIT_TAG        ${tag}
    )
    FetchContent_MakeAvailable(${name})

    set(inc_path ${${name}_SOURCE_DIR}/${inc_dir})
    set(src_path ${${name}_SOURCE_DIR}/${src_dir})
    
    if(manual_add)
        if(TARGET ${name})
            message(WARNING "${name} already exists, skip add_subdirectory")
        else()
            add_subdirectory(${src_path})
        endif()
    endif()
    
    target_include_directories(${_target} PRIVATE ${inc_path})

    set(${name}_INC_PATH ${inc_path} PARENT_SCOPE)
    set(${name}_SRC_PATH ${src_path} PARENT_SCOPE)
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

## Print Common Build Information
function(printBaseBuildInfo)
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
        message(" └─ ANDROID:")
        message("     └─ PLATFORM: ${ANDROID_PLATFORM}")
        message("     └─ ABI: ${ANDROID_ABI}")
        message("     └─ NATIVE_API: ${ANDROID_NATIVE_API_LEVEL}")
    endif()
    if(EMSCRIPTEN)
        message(" └─ EMSCRIPTEN:")
        message("     └─ ABI: ${EMSCRIPTEN_SYSTEM_PROCESSOR}")
    endif()
endfunction()
