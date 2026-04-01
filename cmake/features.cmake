function(dynxx_declare_feature_options)
    option(USE_DEVICE "Use Device" ON)
    if(EMSCRIPTEN)
        set(USE_DEVICE OFF)
    endif()

    set(USE_DEVICE "${USE_DEVICE}" PARENT_SCOPE)
endfunction()

function(dynxx_apply_feature_definitions TARGET_NAME)
    add_definitions_if(${TARGET_NAME} ON
        USE_DEVICE
    )
endfunction()
