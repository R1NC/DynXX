function(dynxx_declare_dependency_options)
    option(USE_CURL "Use CURL" ON)
    if(EMSCRIPTEN)
        set(USE_CURL OFF)
    endif()

    option(USE_LUA "Use Lua" ON)
    if(EMSCRIPTEN)
        set(USE_LUA OFF)
    endif()

    option(USE_QJS "Use QuickJS" ON)
    if(EMSCRIPTEN)
        set(USE_QJS OFF)
    endif()

    option(USE_KV "Use MMKV" ON)
    if(EMSCRIPTEN)
        set(USE_KV OFF)
    endif()

    option(USE_DB "Use SQLite" ON)
    if(EMSCRIPTEN)
        set(USE_DB OFF)
    endif()

    option(USE_LIBUV "Use LibUV" ON)
    if(EMSCRIPTEN)
        set(USE_LIBUV OFF)
    endif()

    option(USE_SPDLOG "Use SpdLog" ON)
    if(EMSCRIPTEN)
        set(USE_SPDLOG OFF)
    endif()

    set(USE_ADA OFF)
    check_apple_version_limit("16.3" "13.3" USE_ADA)
    if(OHOS OR EMSCRIPTEN)
        set(USE_ADA OFF)
    endif()

    set(USE_CURL "${USE_CURL}" PARENT_SCOPE)
    set(USE_LUA "${USE_LUA}" PARENT_SCOPE)
    set(USE_QJS "${USE_QJS}" PARENT_SCOPE)
    set(USE_KV "${USE_KV}" PARENT_SCOPE)
    set(USE_DB "${USE_DB}" PARENT_SCOPE)
    set(USE_LIBUV "${USE_LIBUV}" PARENT_SCOPE)
    set(USE_SPDLOG "${USE_SPDLOG}" PARENT_SCOPE)
    set(USE_ADA "${USE_ADA}" PARENT_SCOPE)
endfunction()

function(dynxx_find_dependencies TARGET_NAME LINK_LIBS_VAR)
    set(link_libs)

    find_package(cJSON CONFIG REQUIRED)
    list(APPEND link_libs cjson)

    find_package(OpenSSL REQUIRED)
    list(APPEND link_libs OpenSSL::SSL OpenSSL::Crypto)

    if(USE_CURL)
        find_package(CURL REQUIRED)
        list(APPEND link_libs CURL::libcurl)
    endif()

    if(WIN32 OR EMSCRIPTEN)
        find_package(ZLIB REQUIRED)
        add_library(zlib ALIAS ZLIB::ZLIB)
        list(APPEND link_libs zlib)
    endif()

    if(USE_LUA)
        find_package(Lua REQUIRED)
        list(APPEND link_libs ${LUA_LIBRARIES})
    endif()

    if(USE_QJS)
        add_git_lib(${TARGET_NAME}
            quickjs
            https://github.com/R1NC/quickjs-ng
            dev
        )
        list(APPEND link_libs qjs)
    endif()

    if(USE_KV)
        add_git_lib(${TARGET_NAME}
            mmkv
            https://github.com/R1NC/MMKV
            dev
            "Core"
            "POSIX/src"
            ON
        )
        list(APPEND link_libs mmkv)
    endif()

    if(USE_DB)
        if(NOT APPLE)
            find_package(unofficial-sqlite3 CONFIG REQUIRED)
            add_library(sqlite3 ALIAS unofficial::sqlite3::sqlite3)
        endif()
        list(APPEND link_libs sqlite3)
    endif()

    if(USE_LIBUV AND NOT OHOS)
        find_package(libuv REQUIRED)
        list(APPEND link_libs $<IF:$<TARGET_EXISTS:libuv::uv_a>,libuv::uv_a,libuv::uv>)
    endif()

    if(USE_ADA)
        find_package(ada REQUIRED)
        list(APPEND link_libs ada::ada)
    endif()

    if(USE_SPDLOG)
        find_package(spdlog REQUIRED)
        list(APPEND link_libs spdlog::spdlog_header_only)
    endif()

    set(${LINK_LIBS_VAR} ${link_libs} PARENT_SCOPE)
endfunction()

function(dynxx_apply_dependency_definitions TARGET_NAME)
    add_definitions_if(${TARGET_NAME} ON
        USE_CURL
        USE_LUA
        USE_QJS
        USE_KV
        USE_DB
        USE_ADA
        USE_LIBUV
        USE_SPDLOG
    )
endfunction()
