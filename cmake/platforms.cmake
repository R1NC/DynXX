function(dynxx_apply_platform_definitions TARGET_NAME)
    if(WIN32)
        add_definitions_if(${TARGET_NAME} OFF
            NOMINMAX
            WIN32_LEAN_AND_MEAN
            GTEST_HAS_PTHREAD=0
            GTEST_HAS_TR1_TUPLE=0
        )
    endif()
endfunction()

function(dynxx_apply_platform_compile_options TARGET_NAME)
    if(WIN32)
        if(MSVC OR (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_CXX_SIMULATE_ID STREQUAL "MSVC"))
            target_compile_options(${TARGET_NAME} PRIVATE /utf-8)
        elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
            target_compile_options(${TARGET_NAME} PRIVATE -finput-charset=UTF-8)
        endif()
    endif()
endfunction()

function(dynxx_apply_objcxx_arc)
    if(APPLE AND ARGN)
        set_source_files_properties(${ARGN} PROPERTIES
            COMPILE_FLAGS "-fobjc-arc"
        )
    endif()
endfunction()

function(dynxx_append_platform_link_libraries LINK_LIBS_VAR)
    set(link_libs ${${LINK_LIBS_VAR}})

    if(ANDROID)
        list(APPEND link_libs
            android
            log
            z
        )
    elseif(APPLE)
        list(APPEND link_libs
            z
            apple_nghttp2
            "-framework Foundation"
        )
        if(USE_DB)
            list(APPEND link_libs sqlite3)
        endif()
    elseif(WIN32)
        list(APPEND link_libs
            ws2_32
            Wldap32
            ntdll
        )
    elseif(OHOS)
        list(APPEND link_libs
            libace_napi.z.so
            libhilog_ndk.z.so
            libdeviceinfo_ndk.z.so
            z
            uv
        )
    endif()

    set(${LINK_LIBS_VAR} ${link_libs} PARENT_SCOPE)
endfunction()
