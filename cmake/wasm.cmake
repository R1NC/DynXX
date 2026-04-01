function(add_wasm_exe TARGET_NAME)
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

function(dynxx_add_wasm_target TARGET_NAME)
    set(options "")
    set(oneValueArgs "")
    set(multiValueArgs SRC_FILES)
    cmake_parse_arguments(DYNXX_WASM "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    set(export_funcs "'_dynxx_init','_dynxx_release'")
    if(USE_LUA)
        set(export_funcs "${export_funcs},'_dynxx_lua_loadS','_dynxx_lua_call'")
    endif()

    add_wasm_exe(${TARGET_NAME}
        SRC_FILES ${DYNXX_WASM_SRC_FILES}
        LINK_FLAGS "-s FETCH -s ASYNCIFY=1"
        RUNTIME_METHODS "'addFunction'"
        FUNCS ${export_funcs}
    )
endfunction()
