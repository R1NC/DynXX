## Initialize before project() call
function(initBeforeProject)
    # Use ccache to turbo compile speed
    find_program(CCACHE_FOUND ccache)
    if(CCACHE_FOUND)
        set(CMAKE_C_COMPILER_LAUNCHER ccache PARENT_SCOPE)
        set(CMAKE_CXX_COMPILER_LAUNCHER ccache PARENT_SCOPE)
    endif()
endfunction()

## Initialize after project() call
function(initAfterProject)
    # Set C/C++ Standard for the target
    set(CMAKE_C_STANDARD 99 PARENT_SCOPE)
    set(CMAKE_C_STANDARD_REQUIRED ON PARENT_SCOPE)
    set(CMAKE_CXX_STANDARD 23 PARENT_SCOPE)
    set(CMAKE_CXX_STANDARD_REQUIRED ON PARENT_SCOPE)
    
    # Disable C++ extensions for better portability
    set(CXX_EXTENSIONS_SETTING OFF PARENT_SCOPE)
    # Enable extensions for CYGWIN/MSYS/MinGW (Windows POSIX compatibility layers)
    if(CMAKE_SYSTEM_NAME MATCHES "CYGWIN" OR CMAKE_SYSTEM_NAME MATCHES "MSYS" OR CMAKE_SYSTEM_NAME MATCHES "MINGW")
        set(CXX_EXTENSIONS_SETTING ON PARENT_SCOPE)
    endif()
    set(CMAKE_CXX_EXTENSIONS ${CXX_EXTENSIONS_SETTING} PARENT_SCOPE)

    # Enable Parallel Build
    include(ProcessorCount)
    ProcessorCount(N PARENT_SCOPE)
    if(NOT N EQUAL 0)
        set(CMAKE_BUILD_PARALLEL_LEVEL ${N} PARENT_SCOPE)
    endif()

    # Enable FetchContent
    include(FetchContent)
    set(FETCHCONTENT_UPDATES_DISCONNECTED ON PARENT_SCOPE)

    set(CMAKE_EXPORT_COMPILE_COMMANDS ON PARENT_SCOPE)
endfunction()

## Check Apple Version for C++ new features
function(checkAppleVersionLimit iosV macV RESULT_VAR)
    if(APPLE AND ((CMAKE_SYSTEM_NAME STREQUAL "iOS" AND CMAKE_OSX_DEPLOYMENT_TARGET VERSION_LESS "${iosV}") OR 
                  (CMAKE_SYSTEM_NAME STREQUAL "Darwin" AND CMAKE_OSX_DEPLOYMENT_TARGET VERSION_LESS "${macV}")))
        set(${RESULT_VAR} OFF PARENT_SCOPE)
    else()
        set(${RESULT_VAR} ON PARENT_SCOPE)
    endif()
endfunction()

## Add lib from Git
## optional params: `inc_dir`, `src_dir`, `manual_add`.
function(addGitLib TARGET_NAME lib url tag)
    if(NOT TARGET_NAME OR NOT lib OR NOT url OR NOT tag)
        message(FATAL_ERROR "`addGitLib()`: `TARGET_NAME`, `lib`, `url`, `tag` are required!")
    endif()
    set(inc_dir ${ARGV4})
    set(src_dir ${ARGV5})
    set(manual_add ${ARGV6})
    if(NOT src_dir)
        set(src_dir "")
    endif()
    if(NOT inc_dir)
        set(inc_dir "")
    endif()
    if(NOT DEFINED manual_add)
        set(manual_add OFF)
    endif()
    
    FetchContent_Declare(
        ${lib}
        GIT_REPOSITORY ${url}
        GIT_TAG        ${tag}
        GIT_SHALLOW    ON
        GIT_PROGRESS   ON
    )
    FetchContent_MakeAvailable(${lib})

    set(inc_path ${${lib}_SOURCE_DIR}/${inc_dir})
    set(src_path ${${lib}_SOURCE_DIR}/${src_dir})
    
    if(manual_add)
        if(TARGET ${lib})
            message(WARNING "${lib} already exists, skip `add_subdirectory()`")
        else()
            add_subdirectory(${src_path})
        endif()
    endif()

    if(EXISTS ${inc_path})
        target_include_directories(${TARGET_NAME} PRIVATE ${inc_path})
    else()
        message(WARNING "${inc_path} not exists, skip `target_include_directories()`")
    endif()

    set(${lib}_INC_PATH ${inc_path} PARENT_SCOPE)
    set(${lib}_SRC_PATH ${src_path} PARENT_SCOPE)
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

function(addFeatureDefinitions TARGET_NAME)
    foreach(FEATURE ${ARGN})
        if(${FEATURE})
            target_compile_definitions(${TARGET_NAME} PRIVATE ${FEATURE})
        endif()
    endforeach()
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
    if(CMAKE_TOOLCHAIN_FILE)
        message("     └─ TOOLCHAIN_FILE: ${CMAKE_TOOLCHAIN_FILE}")
    endif()
    message("     └─ BUILD:")
    message("         └─ TYPE: ${CMAKE_BUILD_TYPE}")
    message("         └─ CROSS_COMPILING: ${CMAKE_CROSSCOMPILING}")
    message("         └─ PARALLEL_LEVEL: ${CMAKE_BUILD_PARALLEL_LEVEL}")
    message("     └─ OUTPUT_DIR:")
    message("         └─ ARCHIVE: ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}")
    message("         └─ LIBRARY: ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
    message("         └─ RUNTIME: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
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
