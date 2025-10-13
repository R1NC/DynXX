#!/bin/bash

source "$(dirname "$0")/build-utils.sh"

#TODO
export VCPKG_ROOT=${VCPKG_ROOT:-"$HOME/dev/vcpkg/"}
export VCPKG_TARGET=x64-linux
export BUILD_FOLDER=build.Linux
export OUTPUT_PATH=${BUILD_FOLDER}/output
export OUTPUT_LIB_PATH=$PWD/${OUTPUT_PATH}/libs
export OUTPUT_EXE_PATH=$PWD/${OUTPUT_PATH}/exe

DEBUG=0
BUILD_TYPE="release"
if [ $DEBUG == 1 ]; then
    BUILD_TYPE="debug"
fi

cd ..
rm -rf ${BUILD_FOLDER}
mkdir -p ${OUTPUT_LIB_PATH}

cmake --preset linux-${BUILD_TYPE}

$VCPKG_ROOT/vcpkg install --triplet=${VCPKG_TARGET}

cmake --build --preset linux-${BUILD_TYPE}
cmake --install ${BUILD_FOLDER} --prefix ${OUTPUT_PATH} --component headers

#Checking Artifacts
ARTIFACTS=(
    "${OUTPUT_LIB_PATH}/libDynXX.a"
    "${OUTPUT_LIB_PATH}/libqjs.a"
    "${OUTPUT_LIB_PATH}/libmmkvcore.a"
    "${OUTPUT_LIB_PATH}/libmmkv.a"
)
check_artifacts "${ARTIFACTS[@]}"
