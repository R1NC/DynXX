function(apply_coverage_flags TARGET_NAME)
    if(NOT TARGET ${TARGET_NAME})
        message(FATAL_ERROR "`apply_coverage_flags()`: target `${TARGET_NAME}` not found")
    endif()

    if(NOT DYNXX_ENABLE_COVERAGE)
        return()
    endif()

    if(MSVC)
        set(DYNXX_COVERAGE_COMPILE_OPTIONS
            "/clang:-fprofile-instr-generate"
            "/clang:-fcoverage-mapping"
        )
        set(DYNXX_COVERAGE_CXX_COMPILE_OPTIONS "/EHsc")
        set(DYNXX_COVERAGE_LINK_OPTIONS "")
    else()
        set(DYNXX_COVERAGE_COMPILE_OPTIONS
            "-fprofile-instr-generate"
            "-fcoverage-mapping"
        )
        set(DYNXX_COVERAGE_CXX_COMPILE_OPTIONS "")
        set(DYNXX_COVERAGE_LINK_OPTIONS
            "-fprofile-instr-generate"
            "-fcoverage-mapping"
        )
    endif()

    target_compile_options(${TARGET_NAME}
        PRIVATE
            ${DYNXX_COVERAGE_COMPILE_OPTIONS}
            $<$<COMPILE_LANGUAGE:CXX>:${DYNXX_COVERAGE_CXX_COMPILE_OPTIONS}>
    )

    get_target_property(_target_type ${TARGET_NAME} TYPE)
    
    if(MSVC)
        # MSVC doesn't need special link options for coverage
        return()
    endif()
    
    if(_target_type STREQUAL "STATIC_LIBRARY")
        # Propagate linker requirements to dependents
        target_link_options(${TARGET_NAME}
            INTERFACE
                ${DYNXX_COVERAGE_LINK_OPTIONS}
        )
    else()
        # Executable or shared library needs link options directly
        target_link_options(${TARGET_NAME}
            PRIVATE
                ${DYNXX_COVERAGE_LINK_OPTIONS}
        )
        # Also propagate to dependents if this is a shared library
        if(_target_type STREQUAL "SHARED_LIBRARY")
            target_link_options(${TARGET_NAME}
                INTERFACE
                    ${DYNXX_COVERAGE_LINK_OPTIONS}
            )
        endif()
    endif()
endfunction()
