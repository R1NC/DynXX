#!/bin/bash

build_with_cmake() {
    local build_dir="$1"
    local root_dir="$2"
    local build_type="$3"
    cd "${build_dir}"
    cmake --build . --config ${build_type}
    cmake --install . --prefix "${root_dir}/output" --component headers
    cd "${root_dir}"
}

merge_libs_apple() {
    local current_dir=$(pwd)
    local lib_dir="$1"
    local output_lib="$2"
    cd "${lib_dir}"
    
    local a_files=(*.a)

    libtool -static -o "${output_lib}" "${a_files[@]}"

    for lib in "${a_files[@]}"; do
        echo "Removing $lib..."
        rm "$lib"
    done

    cd "${current_dir}"
}

merge_libs_posix() {
    local current_dir=$(pwd)
    local lib_dir="$1"
    local output_lib="$2"
    local ar_tool="${3:-ar}"
    cd "${lib_dir}"
    
    for lib in *.a; do
        echo "Extracting $lib..."
        "${ar_tool}" x "$lib"
    done

    rm *.a
    "${ar_tool}" rcs "${output_lib}" *.o
    rm *.o
    find . -maxdepth 1 ! -name "*.a" -type f -delete
    cd "${current_dir}"
}

check_artifacts() {
    local artifacts=("$@")
    echo "Checking artifacts..."
    for artifact in "${artifacts[@]}"; do
        if [ ! -f "$artifact" ]; then
            echo "ARTIFACT NOT FOUND: $artifact"
            exit 1
        else
            echo "✓ Found: $artifact"
        fi
    done
    echo "All artifacts verified successfully!"
}
