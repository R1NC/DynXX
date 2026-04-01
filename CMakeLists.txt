cmake_minimum_required(VERSION 3.20)

include(cmake/utils.cmake)
include(cmake/cxx_features.cmake)
include(cmake/features.cmake)
include(cmake/dependencies.cmake)
include(cmake/platforms.cmake)
include(cmake/sanitizers.cmake)
include(cmake/print.cmake)
if(EMSCRIPTEN)
    include(cmake/wasm.cmake)
endif()

init_before_project()

project(DynXX)

init_after_project()

update_git_submodules()

dynxx_declare_sanitizer_options()
dynxx_configure_cxx_feature_options()
dynxx_declare_feature_options()
dynxx_declare_dependency_options()

set(SRC_FILES "")

file(GLOB_RECURSE CPP_FILES "src/*.cxx")
list(APPEND SRC_FILES ${CPP_FILES})

if(APPLE)
    file(GLOB_RECURSE OBJCXX_FILES "src/*.mm")
    list(APPEND SRC_FILES ${OBJCXX_FILES})
    dynxx_apply_objcxx_arc(${OBJCXX_FILES})
endif()

if(EMSCRIPTEN)
    dynxx_add_wasm_target(${PROJECT_NAME} SRC_FILES ${SRC_FILES})
else()
    add_library(${PROJECT_NAME} STATIC ${SRC_FILES})
endif()

dynxx_apply_cxx_feature_definitions(${PROJECT_NAME})
dynxx_apply_feature_definitions(${PROJECT_NAME})
dynxx_apply_dependency_definitions(${PROJECT_NAME})

dynxx_apply_platform_definitions(${PROJECT_NAME})
dynxx_apply_platform_compile_options(${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PRIVATE include)

if(NOT APPLE)
    target_precompile_headers(${PROJECT_NAME} PRIVATE include/pch.hxx)
endif()

set(LINK_LIBS "")
dynxx_find_dependencies(${PROJECT_NAME} LINK_LIBS)
dynxx_append_platform_link_libraries(LINK_LIBS)
target_link_libraries(${PROJECT_NAME} PRIVATE ${LINK_LIBS})

set(SANITIZERS "")
dynxx_configure_sanitizers(${PROJECT_NAME} SANITIZERS)

install_headers(include/DynXX include)

dynxx_print_summary(${PROJECT_NAME} LINK_LIBS SANITIZERS)
