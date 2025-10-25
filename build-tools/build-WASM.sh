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

EMSDK_ROOT=${EMSDK_ROOT:-"$HOME/dev/emsdk"}
EMSCRIPTEN_ROOT=${EMSCRIPTEN_ROOT:-"$EMSDK_ROOT/upstream/emscripten"}
export EMSDK="${EMSDK_ROOT}"
export EMSCRIPTEN="${EMSCRIPTEN_ROOT}"
export WASM_SDK_ROOT="${EMSCRIPTEN_ROOT}"
export WASM_ABI=arm

export BUILD_FOLDER=build.${PLATFORM}/${BUILD_TYPE}
OUTPUT_FOLDER=${BUILD_FOLDER}/output
OUTPUT_PATH=$PWD/${OUTPUT_FOLDER}/${WASM_ABI}
export OUTPUT_LIB_PATH=${OUTPUT_PATH}/lib
export OUTPUT_DLL_PATH=${OUTPUT_PATH}/share
export OUTPUT_EXE_PATH=${OUTPUT_PATH}/bin

export VCPKG_ROOT=${VCPKG_ROOT:-"$HOME/dev/vcpkg"}
export VCPKG_BINARY_SOURCES=${VCPKG_BINARY_SOURCES:-"clear;files,$HOME/vcpkg-binary-cache,readwrite"}
export VCPKG_TARGET=wasm32-emscripten
"${VCPKG_ROOT}/vcpkg" --vcpkg-root "${VCPKG_ROOT}" install --triplet="${VCPKG_TARGET}"

cmake --preset ${PRESET}
cmake --build --preset ${PRESET}

export_compile_commands ${BUILD_FOLDER}

ARTIFACTS=(
    "${OUTPUT_EXE_PATH}/DynXX.wasm"
    "${OUTPUT_EXE_PATH}/DynXX.js"
    "${OUTPUT_EXE_PATH}/DynXX.html"
)
check_artifacts "${ARTIFACTS[@]}"
