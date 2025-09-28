#!/bin/bash

source "$(dirname "$0")/build-utils.sh"

EMSCRIPTEN_ROOT=${EMSCRIPTEN_ROOT:-"$HOME/dev/emsdk/upstream/emscripten"}
DEBUG=0

BUILD_DIR=../build.WebAssembly
rm -rf ${BUILD_DIR}
mkdir -p ${BUILD_DIR}
cd ${BUILD_DIR}

build4wasm() {
    BUILD_TYPE=$1
    ABI=$2

    cmake .. \
    -DEMSCRIPTEN_ROOT_PATH=${EMSCRIPTEN_ROOT} \
    -DCMAKE_TOOLCHAIN_FILE=${EMSCRIPTEN_ROOT}/cmake/Modules/Platform/Emscripten.cmake \
    -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
    -DEMSCRIPTEN_SYSTEM_PROCESSOR=${ABI}

    cmake --build . --config ${BUILD_TYPE}

    OUTPUT_DIR=output/${ABI}
    mkdir -p ${OUTPUT_DIR}
    cp DynXX.wasm ${OUTPUT_DIR}
    cp DynXX.js ${OUTPUT_DIR}
    cp DynXX.html ${OUTPUT_DIR}

    #Checking Artifacts
    ARTIFACTS=(
        "${OUTPUT_DIR}/DynXX.wasm"
        "${OUTPUT_DIR}/DynXX.js"
        "${OUTPUT_DIR}/DynXX.html"
    )
    check_artifacts "${ARTIFACTS[@]}"
}

# Debug|RelWithDebInfo|Release|MinSizeRel
LIB_TYPE="MinSizeRel"
if [ $DEBUG == 1 ]; then
    LIB_TYPE="Debug"
fi

build4wasm $LIB_TYPE arm
