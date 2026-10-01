function(add_wasm_exe TARGET_NAME)
    set(options "")
    set(oneValueArgs RUNTIME_METHODS FUNCS)
    set(multiValueArgs SRC_FILES LINK_OPTIONS)
    cmake_parse_arguments(WASM "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    add_executable(${TARGET_NAME} ${SRC_FILES})
    dynxx_apply_optimization_options(${TARGET_NAME})

    set_target_properties(${TARGET_NAME} PROPERTIES PREFIX "" SUFFIX ".html")

    # Emscripten reads each "-s NAME=VALUE" as two argv entries, hence the SHELL:
    # prefixes below: without them target_link_options() de-duplicates the repeated
    # "-s" and silently turns "-s FETCH" into a bare "FETCH". (The legacy LINK_FLAGS
    # property this replaced happened to work because CMake pasted the whole string
    # into the shell command line, which split it for us.)

    set(FINAL_RUNTIME_METHODS "'getValue','setValue','UTF8ToString','lengthBytesUTF8','stringToUTF8'")
    if(WASM_RUNTIME_METHODS)
        set(FINAL_RUNTIME_METHODS "${FINAL_RUNTIME_METHODS},${WASM_RUNTIME_METHODS}")
    endif()

    set(FINAL_FUNCS "'_malloc','_free'")
    if(WASM_FUNCS)
        set(FINAL_FUNCS "${FINAL_FUNCS},${WASM_FUNCS}")
    endif()

    target_link_options(${TARGET_NAME} PRIVATE
        "SHELL:-s ALLOW_TABLE_GROWTH"
        ${WASM_LINK_OPTIONS}
        "SHELL:-s EXPORTED_RUNTIME_METHODS=[${FINAL_RUNTIME_METHODS}]"
        "SHELL:-s EXPORTED_FUNCTIONS=[${FINAL_FUNCS}]"
    )
    dynxx_apply_final_target_optimization(${TARGET_NAME})
endfunction()

function(dynxx_add_wasm_target TARGET_NAME)
    set(options "")
    set(oneValueArgs "")
    set(multiValueArgs SRC_FILES)
    cmake_parse_arguments(DYNXX_WASM "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    set(export_funcs "'_dynxx_init','_dynxx_release'")
    if(DYNXX_USE_LUA)
        set(export_funcs "${export_funcs},'_dynxx_lua_loadS','_dynxx_lua_call'")
    endif()

    add_wasm_exe(${TARGET_NAME}
        SRC_FILES ${DYNXX_WASM_SRC_FILES}
        LINK_OPTIONS "SHELL:-s FETCH" "SHELL:-s ASYNCIFY=1"
        RUNTIME_METHODS "'addFunction'"
        FUNCS ${export_funcs}
    )
endfunction()
