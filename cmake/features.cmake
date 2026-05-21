function(dynxx_declare_feature_options)
    option(DYNXX_USE_DEVICE "Use Device" ON)
    if(EMSCRIPTEN)
        set(DYNXX_USE_DEVICE OFF)
    endif()

    set(DYNXX_USE_DEVICE "${DYNXX_USE_DEVICE}" PARENT_SCOPE)
endfunction()

function(dynxx_apply_feature_definitions TARGET_NAME)
    add_definitions_if(${TARGET_NAME} ON
        DYNXX_USE_DEVICE
    )
endfunction()
