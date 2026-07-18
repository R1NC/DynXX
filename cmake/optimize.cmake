function(dynxx_declare_optimization_options)
    option(DYNXX_ENABLE_SIZE_OPTIMIZATION "Enable size-oriented compiler optimizations" OFF)
    option(DYNXX_ENABLE_SECTION_GC "Enable linker dead-stripping / garbage collection" OFF)
    option(DYNXX_ENABLE_HIDDEN_VISIBILITY "Hide symbols by default on supported toolchains" OFF)
    option(DYNXX_ENABLE_FUNCTION_DATA_SECTIONS "Emit function/data into dedicated sections on supported toolchains" OFF)
endfunction()

function(dynxx_initialize_optimization_defaults)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(_dynxx_default_optimization OFF)
    else()
        set(_dynxx_default_optimization ON)
    endif()

    foreach(option_name
        DYNXX_ENABLE_SIZE_OPTIMIZATION
        DYNXX_ENABLE_SECTION_GC
        DYNXX_ENABLE_HIDDEN_VISIBILITY
        DYNXX_ENABLE_FUNCTION_DATA_SECTIONS
    )
        if(NOT DEFINED ${option_name})
            set(${option_name} ${_dynxx_default_optimization} CACHE BOOL
                "Auto-initialized optimization setting for single-config wrapper targets" FORCE)
        endif()
    endforeach()
endfunction()

function(dynxx_append_link_options TARGET_NAME)
    if(COMMAND target_link_options)
        target_link_options(${TARGET_NAME} PRIVATE ${ARGN})
        return()
    endif()

    foreach(link_opt ${ARGN})
        set_property(TARGET ${TARGET_NAME} APPEND_STRING PROPERTY LINK_FLAGS " ${link_opt}")
    endforeach()
endfunction()

function(dynxx_apply_optimization_options TARGET_NAME)
    if(NOT TARGET ${TARGET_NAME})
        message(FATAL_ERROR "`dynxx_apply_optimization_options()`: target `${TARGET_NAME}` not found")
    endif()

    if(MSVC OR (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_CXX_SIMULATE_ID STREQUAL "MSVC"))
        if(DYNXX_ENABLE_FUNCTION_DATA_SECTIONS)
            target_compile_options(${TARGET_NAME} PRIVATE /Gy /Gw)
        endif()
        if(DYNXX_ENABLE_SIZE_OPTIMIZATION)
            target_compile_options(${TARGET_NAME} PRIVATE /O1)
        endif()
        return()
    endif()

    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR
       CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR
       CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
        if(DYNXX_ENABLE_FUNCTION_DATA_SECTIONS)
            target_compile_options(${TARGET_NAME} PRIVATE
                -ffunction-sections
                -fdata-sections
            )
        endif()
        if(DYNXX_ENABLE_HIDDEN_VISIBILITY)
            target_compile_options(${TARGET_NAME} PRIVATE -fvisibility=hidden)
        endif()
        if(DYNXX_ENABLE_SIZE_OPTIMIZATION)
            if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
                target_compile_options(${TARGET_NAME} PRIVATE -Os)
            else()
                target_compile_options(${TARGET_NAME} PRIVATE -Oz)
            endif()
        endif()
    endif()
endfunction()

function(dynxx_apply_final_target_optimization TARGET_NAME)
    if(NOT TARGET ${TARGET_NAME})
        message(FATAL_ERROR "`dynxx_apply_final_target_optimization()`: target `${TARGET_NAME}` not found")
    endif()

    if(NOT DYNXX_ENABLE_SECTION_GC)
        return()
    endif()

    if(MSVC OR (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_CXX_SIMULATE_ID STREQUAL "MSVC"))
        dynxx_append_link_options(${TARGET_NAME} /OPT:REF)
    elseif(APPLE)
        dynxx_append_link_options(${TARGET_NAME} -Wl,-dead_strip)
    else()
        dynxx_append_link_options(${TARGET_NAME} -Wl,--gc-sections)
    endif()
endfunction()
