#!/bin/bash
source "$(dirname "$0")/build-utils.sh"

cd ..

DEBUG=0
BUILD_TYPE="Release"
if [ $DEBUG == 1 ]; then
    BUILD_TYPE="Debug"
fi

PLATFORM=Wasm
PRESET=${PLATFORM}-${BUILD_TYPE}

export BUILD_FOLDER=build.${PLATFORM}
OUTPUT_FOLDER=${BUILD_FOLDER}/output
export OUTPUT_LIB_PATH=$PWD/${OUTPUT_FOLDER}/libs
export OUTPUT_EXE_PATH=$PWD/${OUTPUT_FOLDER}/exe

EMSDK_ROOT=${EMSDK_ROOT:-"$HOME/dev/emsdk"}
EMSCRIPTEN_ROOT=${EMSCRIPTEN_ROOT:-"$EMSDK_ROOT/upstream/emscripten"}
export EMSDK="${EMSDK_ROOT}"
export EMSCRIPTEN="${EMSCRIPTEN_ROOT}"
export WASM_SDK_ROOT="${EMSCRIPTEN_ROOT}"
export WASM_ABI=arm

export VCPKG_ROOT=${VCPKG_ROOT:-"$HOME/dev/vcpkg/"}
export VCPKG_BINARY_SOURCES="default,read"
export VCPKG_TARGET=wasm32-emscripten

rm -rf ${BUILD_FOLDER}

cmake --preset ${PRESET}

$VCPKG_ROOT/vcpkg install --triplet=${VCPKG_TARGET}

cmake --build --preset ${PRESET}

ARTIFACTS=(
    "${OUTPUT_EXE_PATH}/DynXX.wasm"
    "${OUTPUT_EXE_PATH}/DynXX.js"
    "${OUTPUT_EXE_PATH}/DynXX.html"
)
check_artifacts "${ARTIFACTS[@]}"
