#!/bin/bash

merge_libs() {
    local current_dir=$(pwd)
    local lib_dir="$1"
    local output_lib="$2"
    local ar_tool="$3"
    
    cd "${lib_dir}"
    local a_files=(*.a)
    
    if [ ${#a_files[@]} -eq 0 ]; then
        echo "ERROR: No static libraries found in ${lib_dir}"
        cd "${current_dir}"
        exit 1
    fi

    if [[ "${ar_tool}" == *"libtool"* ]]; then
        echo "Using libtool for merging ${#a_files[@]} libraries..."
        echo "Libraries to merge: ${a_files[@]}"

        # libtool handles duplicate symbols automatically
        "${ar_tool}" -static -o "${output_lib}" "${a_files[@]}"
        
        if [ $? -ne 0 ]; then
            echo "ERROR: libtool merge failed"
            cd "${current_dir}"
            exit 1
        fi
        
        echo "✓ Successfully merged with libtool"
    else
        echo "Using ar tool for merging ${#a_files[@]} libraries..."
        echo "Libraries to merge: ${a_files[@]}"

        # Extract each library in a temporary directory to avoid duplicate symbols

        local temp_dir="temp_merge_$$"
        mkdir -p "${temp_dir}"
        local index=0

        for lib in "${a_files[@]}"; do
            echo "Extracting $lib..."
            local lib_dir="${temp_dir}/lib${index}"
            mkdir -p "${lib_dir}"
            cd "${lib_dir}"
            "${ar_tool}" x "../../$lib"
            if [ $? -ne 0 ]; then
                echo "ERROR: Failed to extract $lib"
                cd "${current_dir}"
                rm -rf "${temp_dir}"
                exit 1
            fi
            cd "../.."
            ((index++))
        done
        
        echo "Creating merged library ${output_lib}..."
        local obj_files=$(find "${temp_dir}" -name '*.o')
        if [ -z "${obj_files}" ]; then
            echo "ERROR: No object files found after extraction"
            rm -rf "${temp_dir}"
            cd "${current_dir}"
            exit 1
        fi
        
        # Collect all .o files into an array and add them in one ar command
        local obj_array=()
        while IFS= read -r -d '' obj_file; do
            obj_array+=("$obj_file")
        done < <(find "${temp_dir}" -name '*.o' -print0)
        
        # Create archive with all object files at once to avoid duplicates
        "${ar_tool}" rcs "${output_lib}" "${obj_array[@]}"
        
        if [ $? -ne 0 ]; then
            echo "ERROR: ar merge failed"
            rm -rf "${temp_dir}"
            cd "${current_dir}"
            exit 1
        fi
        
        rm -rf "${temp_dir}"
        echo "✓ Successfully merged with ar"
    fi
    
    if [ ! -f "${output_lib}" ]; then
        echo "ERROR: Output library ${output_lib} was not created"
        cd "${current_dir}"
        exit 1
    fi
    
    local lib_size_bytes=$(stat -f%z "${output_lib}" 2>/dev/null || stat -c%s "${output_lib}" 2>/dev/null)
    local lib_size_formatted=$(printf "%'d" ${lib_size_bytes} 2>/dev/null || echo ${lib_size_bytes})
    echo "✓ Merged library size: ${lib_size_formatted} Bytes"

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
