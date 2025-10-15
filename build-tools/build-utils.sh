#!/bin/bash

merge_libs() {
    local current_dir=$(pwd)
    local lib_dir="$1"
    local output_lib="$2"
    local ar_tool="$3"
    
    cd "${lib_dir}"
    local a_files=(*.a)

    if [[ "${ar_tool}" == *"libtool"* ]]; then
        echo "Using libtool for merging ${#a_files[@]} libraries..."
        "${ar_tool}" -static -o "${output_lib}" "${a_files[@]}"
    else
        echo "Using ar tool for merging ${#a_files[@]} libraries..."
        for lib in "${a_files[@]}"; do
            echo "Extracting $lib..."
            "${ar_tool}" x "$lib"
        done
        
        echo "Creating merged library ${output_lib}..."
        "${ar_tool}" rcs "${output_lib}" *.o
        rm -f *.o
    fi

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
