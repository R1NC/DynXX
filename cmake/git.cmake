# Optional named args after required ones (recommended):
#   INC_DIR <path>
#   SRC_DIR <path>
#   MANUAL_ADD_SUBDIRECTORY <bool> (default: OFF)
function(add_git_lib TARGET_NAME lib url tag)
    if(NOT TARGET_NAME OR NOT lib OR NOT url OR NOT tag)
        message(FATAL_ERROR "`add_git_lib()`: `TARGET_NAME`, `lib`, `url`, `tag` are required!")
    endif()
    set(inc_dir "")
    set(src_dir "")
    set(manual_add_subdirectory OFF)

    cmake_parse_arguments(AGL
        ""
        "INC_DIR;SRC_DIR;MANUAL_ADD_SUBDIRECTORY"
        ""
        ${ARGN}
    )
    if(AGL_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "`add_git_lib()`: unknown args: ${AGL_UNPARSED_ARGUMENTS}")
    endif()
    if(DEFINED AGL_INC_DIR)
        set(inc_dir "${AGL_INC_DIR}")
    endif()
    if(DEFINED AGL_SRC_DIR)
        set(src_dir "${AGL_SRC_DIR}")
    endif()
    if(DEFINED AGL_MANUAL_ADD_SUBDIRECTORY)
        set(manual_add_subdirectory "${AGL_MANUAL_ADD_SUBDIRECTORY}")
    endif()

    set(fetchcontent_args
        GIT_REPOSITORY ${url}
        GIT_TAG ${tag}
        GIT_SHALLOW ON
        GIT_PROGRESS ON
        GIT_REMOTE_UPDATE_STRATEGY REBASE_CHECKOUT
    )

    FetchContent_Declare(${lib} ${fetchcontent_args})
    FetchContent_MakeAvailable(${lib})

    set(inc_path ${${lib}_SOURCE_DIR}/${inc_dir})
    set(src_path ${${lib}_SOURCE_DIR}/${src_dir})

    if(manual_add_subdirectory)
        if(NOT src_dir)
            message(FATAL_ERROR "`add_git_lib()`: `src_dir` is required when `manual_add_subdirectory` is ON")
        endif()
        if(TARGET ${lib})
            message(WARNING "${lib} already exists, skip `add_subdirectory()`")
        else()
            add_subdirectory(${src_path})
        endif()
    endif()

    if(EXISTS ${inc_path})
        target_include_directories(${TARGET_NAME} PRIVATE ${inc_path})
    else()
        message(WARNING "${inc_path} not exists, skip `target_include_directories()`")
    endif()

    set(${lib}_INC_PATH ${inc_path} PARENT_SCOPE)
    set(${lib}_SRC_PATH ${src_path} PARENT_SCOPE)
endfunction()

function(update_git_submodules)
    execute_process(
        COMMAND git submodule update --init --recursive
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        RESULT_VARIABLE git_submod_result
    )
endfunction()
