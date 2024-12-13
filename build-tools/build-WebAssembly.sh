#!/bin/sh

#TODO
EMSCRIPTEN_ROOT_PATH="~/dev/emsdk/upstream/emscripten"
declare -i DEBUG=0

BUILD_DIR=../build.WebAssembly
rm -rf ${BUILD_DIR}
mkdir -p ${BUILD_DIR}
cd ${BUILD_DIR}

function build4wasm {
    BUILD_TYPE=$1
    ABI=$2

    cmake .. \
    -DEMSCRIPTEN_ROOT_PATH=${EMSCRIPTEN_ROOT_PATH} \
    -DCMAKE_TOOLCHAIN_FILE=${EMSCRIPTEN_ROOT_PATH}/cmake/Modules/Platform/Emscripten.cmake \
    -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
    -DEMSCRIPTEN_SYSTEM_PROCESSOR=${ABI} \

    cmake --build . --config ${BUILD_TYPE}

    OUTPUT_DIR=output/${ABI}
    mkdir -p ${OUTPUT_DIR}
    cp NGenXX.wasm ${OUTPUT_DIR}
    cp NGenXX.js ${OUTPUT_DIR}
}

LIB_TYPE="Release"
if [[ $DEBUG == 1 ]]
then
    LIB_TYPE="Debug"
fi

build4wasm $LIB_TYPE arm
#build4wasm $LIB_TYPE x86