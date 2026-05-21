function(dynxx_configure_cxx_feature_options)
    set(DYNXX_USE_STD_TO_CHARS ON)
    check_apple_version_limit("16.5" "13.4" DYNXX_USE_STD_TO_CHARS)

    set(DYNXX_USE_STD_TO_CHARS "${DYNXX_USE_STD_TO_CHARS}" PARENT_SCOPE)
endfunction()

function(dynxx_apply_cxx_feature_definitions TARGET_NAME)
    add_definitions_if(${TARGET_NAME} ON
        DYNXX_USE_STD_TO_CHARS
    )
endfunction()
