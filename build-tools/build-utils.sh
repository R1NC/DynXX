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