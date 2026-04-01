function(dynxx_configure_cxx_feature_options)
    set(USE_STD_TO_CHARS ON)
    check_apple_version_limit("16.5" "13.4" USE_STD_TO_CHARS)

    set(USE_STD_TO_CHARS "${USE_STD_TO_CHARS}" PARENT_SCOPE)
endfunction()

function(dynxx_apply_cxx_feature_definitions TARGET_NAME)
    add_definitions_if(${TARGET_NAME} ON
        USE_STD_TO_CHARS
    )
endfunction()
