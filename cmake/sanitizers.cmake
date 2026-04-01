function(dynxx_declare_sanitizer_options)
    set(USE_SANITIZERS OFF PARENT_SCOPE)
endfunction()

function(dynxx_configure_sanitizers TARGET_NAME SANITIZERS_VAR)
    set(sanitizers)

    if(USE_SANITIZERS AND ${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        list(APPEND sanitizers "address")

        if(NOT WIN32)
            list(APPEND sanitizers "undefined")
        endif()

        if(NOT WIN32 AND NOT EMSCRIPTEN AND NOT ANDROID AND NOT OHOS AND NOT APPLE)
            list(APPEND sanitizers "leak")
        endif()

        if(sanitizers)
            list(JOIN sanitizers "," sanitizers_str)
            if(MSVC)
                set(sanitizer_flags "/fsanitize=${sanitizers_str}")
            else()
                set(sanitizer_flags "-fsanitize=${sanitizers_str}")
            endif()
            set_target_properties(${TARGET_NAME} PROPERTIES
                COMPILE_FLAGS "${sanitizer_flags} -fno-omit-frame-pointer -g"
                LINK_FLAGS "${sanitizer_flags}"
            )
        endif()
    endif()

    set(${SANITIZERS_VAR} ${sanitizers} PARENT_SCOPE)
endfunction()

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
