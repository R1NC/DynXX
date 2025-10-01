#!/bin/bash

source "$(dirname "$0")/build-utils.sh"

#TODO
VCPKG_ROOT=${VCPKG_ROOT:-"$HOME/dev/vcpkg/"}
VCPKG_TARGET=x64-linux

DEBUG=0

cd ..
export VCPKG_BINARY_SOURCES="clear;default,readwrite"
$VCPKG_ROOT/vcpkg install --triplet=${VCPKG_TARGET}

BUILD_DIR=build.Linux
rm -rf ${BUILD_DIR}
mkdir -p ${BUILD_DIR}
cd ${BUILD_DIR}

LIB_TYPE="Release"
if [ $DEBUG == 1 ]; then
    LIB_TYPE="Debug"
fi

LIB_OUTPUT_DIR=$(pwd)/output/libs
mkdir -p ${LIB_OUTPUT_DIR}

cmake .. \
    -DCMAKE_INSTALL_PREFIX=. \
    -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake \
    -DVCPKG_TARGET_TRIPLET=$VCPKG_TARGET \
    -DCMAKE_BUILD_TYPE=${LIB_TYPE} \
    -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY=${LIB_OUTPUT_DIR} \
    -DCMAKE_LIBRARY_OUTPUT_DIRECTORY=${LIB_OUTPUT_DIR} \
    -DCMAKE_RUNTIME_OUTPUT_DIRECTORY=${LIB_OUTPUT_DIR}

build_with_cmake . ${BUILD_DIR} ${LIB_TYPE}

#Checking Artifacts
ARTIFACTS=(
    "${LIB_OUTPUT_DIR}/libDynXX.a"
    "${LIB_OUTPUT_DIR}/libqjs.a"
    "${LIB_OUTPUT_DIR}/libmmkvcore.a"
    "${LIB_OUTPUT_DIR}/libmmkv.a"
)
check_artifacts "${ARTIFACTS[@]}"