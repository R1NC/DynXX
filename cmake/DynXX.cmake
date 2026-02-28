## Initialize before project() call
function(initBeforeProject)
    # Use ccache to turbo compile speed
    find_program(CCACHE_PROGRAM ccache)
    if(CCACHE_PROGRAM)
        set(CMAKE_C_COMPILER_LAUNCHER "${CCACHE_PROGRAM}" PARENT_SCOPE)
        set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}" PARENT_SCOPE)
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
    set(FETCHCONTENT_QUIET OFF PARENT_SCOPE)
    set(FETCHCONTENT_TIMEOUT 600 PARENT_SCOPE)
    set(FETCHCONTENT_DOWNLOAD_TIMEOUT_SEC 600 PARENT_SCOPE)

    # Enable Compile Commands for better IDE support
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
        GIT_REMOTE_UPDATE_STRATEGY REBASE_CHECKOUT
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

function(addDefinitions TARGET_NAME NEED_CHECK)
    foreach(def ${ARGN})
        if(NEED_CHECK)
            if(DEFINED ${def} AND ${def})
                target_compile_definitions(${TARGET_NAME} PRIVATE ${def})
            endif()
        else()
            target_compile_definitions(${TARGET_NAME} PRIVATE ${def})
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

## Install Sanitizers Runtime
function(install_asan_runtime dest_dir)
	if(APPLE)
		if(CMAKE_OSX_SYSROOT MATCHES "Simulator|iPhoneSimulator")
			set(_asan_lib_name "libclang_rt.asan_iossim_dynamic.dylib")
		elseif(CMAKE_OSX_SYSROOT MATCHES "iPhoneOS|iphoneos")
			set(_asan_lib_name "libclang_rt.asan_ios_dynamic.dylib")
		elseif(CMAKE_OSX_SYSROOT MATCHES "MacOSX|macosx")
			set(_asan_lib_name "libclang_rt.asan_osx_dynamic.dylib")
		else()
			message(SEND_ERROR "install_asan_runtime: UNKNOWN CMAKE_OSX_SYSROOT: ${CMAKE_OSX_SYSROOT}")
			return()
		endif()
	elseif(CMAKE_SYSTEM_NAME STREQUAL "Android" OR CMAKE_SYSTEM_NAME STREQUAL "OHOS")
		set(_abi "${CMAKE_ANDROID_ARCH_ABI}")
		if(CMAKE_SYSTEM_NAME STREQUAL "OHOS" AND NOT _abi)
			set(_abi "${CMAKE_SYSTEM_PROCESSOR}")
		endif()
		if(_abi STREQUAL "arm64-v8a" OR _abi STREQUAL "aarch64")
			set(_asan_lib_name "libclang_rt.asan-aarch64-android.so")
		elseif(_abi STREQUAL "armeabi-v7a" OR _abi STREQUAL "armv7a" OR _abi STREQUAL "arm")
			set(_asan_lib_name "libclang_rt.asan-arm-android.so")
		elseif(_abi STREQUAL "x86_64")
			set(_asan_lib_name "libclang_rt.asan-x86_64-android.so")
		elseif(_abi STREQUAL "x86" OR _abi STREQUAL "i686")
			set(_asan_lib_name "libclang_rt.asan-i686-android.so")
		else()
			message(SEND_ERROR "install_asan_runtime: UNKNOWN ABI: ${_abi}")
			return()
		endif()
	else()
		message(SEND_ERROR "install_asan_runtime: Only support APPLE / Android / OHOS (CMAKE_SYSTEM_NAME=${CMAKE_SYSTEM_NAME})")
		return()
	endif()

	execute_process(
		COMMAND ${CMAKE_CXX_COMPILER} -print-file-name=${_asan_lib_name}
		OUTPUT_VARIABLE _asan_lib_full_path
	)

	string(STRIP "${_asan_lib_full_path}" _asan_lib_full_path)

	if(NOT EXISTS "${_asan_lib_full_path}" AND (CMAKE_SYSTEM_NAME STREQUAL "Android" OR CMAKE_SYSTEM_NAME STREQUAL "OHOS"))
		if(NOT _asan_lib_full_path MATCHES "^[/\\\\]|^[A-Za-z]:")
			execute_process(
				COMMAND ${CMAKE_CXX_COMPILER} -print-resource-dir
				OUTPUT_VARIABLE _clang_resource_dir
			)
			string(STRIP "${_clang_resource_dir}" _clang_resource_dir)
			set(_asan_lib_full_path "${_clang_resource_dir}/lib/linux/${_asan_lib_name}")
		endif()
	endif()

	if(EXISTS "${_asan_lib_full_path}")
		file(COPY "${_asan_lib_full_path}" DESTINATION "${dest_dir}")
		message(STATUS "ASan runtime lib is installed: ${_asan_lib_full_path} -> ${dest_dir}/${_asan_lib_name}")
	else()
		message(SEND_ERROR "ASan lib not found: ${_asan_lib_name} (path: ${_asan_lib_full_path})")
	endif()
endfunction()

## Print Common Build Information
function(printBaseBuildInfo)
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
    message("     └─ SIMULATE:")
    message("         └─ C: ${CMAKE_C_SIMULATE_ID} ${CMAKE_C_SIMULATE_VERSION}")
    message("         └─ C++: ${CMAKE_CXX_SIMULATE_ID} ${CMAKE_CXX_SIMULATE_VERSION}")
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
