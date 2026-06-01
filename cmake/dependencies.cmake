include(${CMAKE_CURRENT_LIST_DIR}/git.cmake)

function(dynxx_declare_dependency_options)
    option(DYNXX_USE_CURL "Use CURL" ON)
    if(EMSCRIPTEN)
        set(DYNXX_USE_CURL OFF)
    endif()

    option(DYNXX_USE_LUA "Use Lua" ON)
    if(EMSCRIPTEN)
        set(DYNXX_USE_LUA OFF)
    endif()

    option(DYNXX_USE_QJS "Use QuickJS" ON)
    if(EMSCRIPTEN)
        set(DYNXX_USE_QJS OFF)
    endif()

    option(DYNXX_USE_KV "Use MMKV" ON)
    if(EMSCRIPTEN)
        set(DYNXX_USE_KV OFF)
    endif()

    option(DYNXX_USE_DB "Use SQLite" ON)
    if(EMSCRIPTEN)
        set(DYNXX_USE_DB OFF)
    endif()

    option(DYNXX_USE_LIBUV "Use LibUV" ON)
    if(EMSCRIPTEN)
        set(DYNXX_USE_LIBUV OFF)
    endif()

    option(DYNXX_USE_SPDLOG "Use SpdLog" ON)
    if(EMSCRIPTEN)
        set(DYNXX_USE_SPDLOG OFF)
    endif()

    set(DYNXX_USE_ADA OFF)
    check_apple_version_limit("16.3" "13.3" DYNXX_USE_ADA)
    if(OHOS OR EMSCRIPTEN)
        set(DYNXX_USE_ADA OFF)
    endif()

    set(DYNXX_USE_CURL "${DYNXX_USE_CURL}" PARENT_SCOPE)
    set(DYNXX_USE_LUA "${DYNXX_USE_LUA}" PARENT_SCOPE)
    set(DYNXX_USE_QJS "${DYNXX_USE_QJS}" PARENT_SCOPE)
    set(DYNXX_USE_KV "${DYNXX_USE_KV}" PARENT_SCOPE)
    set(DYNXX_USE_DB "${DYNXX_USE_DB}" PARENT_SCOPE)
    set(DYNXX_USE_LIBUV "${DYNXX_USE_LIBUV}" PARENT_SCOPE)
    set(DYNXX_USE_SPDLOG "${DYNXX_USE_SPDLOG}" PARENT_SCOPE)
    set(DYNXX_USE_ADA "${DYNXX_USE_ADA}" PARENT_SCOPE)
endfunction()

function(dynxx_find_dependencies TARGET_NAME LINK_LIBS_VAR)
    set(link_libs)

    find_package(cJSON CONFIG REQUIRED)
    list(APPEND link_libs cjson)

    find_package(OpenSSL REQUIRED)
    list(APPEND link_libs OpenSSL::SSL OpenSSL::Crypto)

    if(DYNXX_USE_CURL)
        find_package(CURL REQUIRED)
        list(APPEND link_libs CURL::libcurl)
    endif()

    if(WIN32 OR EMSCRIPTEN)
        find_package(ZLIB REQUIRED)
        add_library(zlib ALIAS ZLIB::ZLIB)
        list(APPEND link_libs zlib)
    endif()

    if(DYNXX_USE_LUA)
        find_package(Lua REQUIRED)
        list(APPEND link_libs ${LUA_LIBRARIES})
    endif()

    if(DYNXX_USE_QJS)
        # quickjs-ng defaults BUILD_QJSC=OFF on Linux. Enable it for desktop, but
        # keep mobile/wasm platforms excluded.
        if(ANDROID OR OHOS OR EMSCRIPTEN OR CMAKE_SYSTEM_NAME STREQUAL "iOS" OR (DEFINED IOS AND IOS))
            set(BUILD_QJSC OFF CACHE BOOL "Build qjsc executable" FORCE)
        else()
            set(BUILD_QJSC ON CACHE BOOL "Build qjsc executable" FORCE)
        endif()
        add_git_lib(${TARGET_NAME}
            quickjs
            https://github.com/rinc-xyz/quickjs-ng
            dev
        )
        if(CMAKE_C_COMPILER_ID MATCHES "Clang" OR CMAKE_C_COMPILER_ID MATCHES "AppleClang")
            foreach(qjs_target IN ITEMS qjs qjsc quickjs)
                if(TARGET ${qjs_target})
                    target_compile_options(${qjs_target} PRIVATE
                        -Wno-shorten-64-to-32
                        -Wno-error=shorten-64-to-32
                    )
                endif()
            endforeach()
        endif()
        list(APPEND link_libs qjs)
    endif()

    if(DYNXX_USE_KV)
        add_git_lib(${TARGET_NAME}
            mmkv
            https://github.com/rinc-xyz/MMKV
            dev
            INC_DIR "Core"
            SRC_DIR "POSIX/src"
            MANUAL_ADD_SUBDIRECTORY ON
        )
        list(APPEND link_libs mmkv)
    endif()

    if(DYNXX_USE_DB)
        if(NOT APPLE)
            find_package(unofficial-sqlite3 CONFIG REQUIRED)
            add_library(sqlite3 ALIAS unofficial::sqlite3::sqlite3)
        endif()
        list(APPEND link_libs sqlite3)
    endif()

    if(DYNXX_USE_LIBUV AND NOT OHOS)
        find_package(libuv REQUIRED)
        list(APPEND link_libs $<IF:$<TARGET_EXISTS:libuv::uv_a>,libuv::uv_a,libuv::uv>)
    endif()

    if(DYNXX_USE_ADA)
        find_package(ada REQUIRED)
        list(APPEND link_libs ada::ada)
    endif()

    if(DYNXX_USE_SPDLOG)
        find_package(spdlog REQUIRED)
        list(APPEND link_libs spdlog::spdlog_header_only)
    endif()

    set(${LINK_LIBS_VAR} ${link_libs} PARENT_SCOPE)
endfunction()

function(dynxx_apply_dependency_definitions TARGET_NAME)
    add_definitions_if(${TARGET_NAME} ON
        DYNXX_USE_CURL
        DYNXX_USE_LUA
        DYNXX_USE_QJS
        DYNXX_USE_KV
        DYNXX_USE_DB
        DYNXX_USE_ADA
        DYNXX_USE_LIBUV
        DYNXX_USE_SPDLOG
    )
endfunction()
