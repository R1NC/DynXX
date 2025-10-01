#!/bin/bash

source "$(dirname "$0")/build-utils.sh"

#TODO
VCPKG_ROOT=${VCPKG_ROOT:-"$HOME/dev/vcpkg/"}
VCPKG_TARGET=arm64-ios

DEBUG=0
TARGET_VERSION=15.0

cd ..
export VCPKG_BINARY_SOURCES="clear;default,readwrite"
$VCPKG_ROOT/vcpkg install --triplet=${VCPKG_TARGET}

BUILD_DIR=build.iOS
rm -rf ${BUILD_DIR}
mkdir -p ${BUILD_DIR}
cd ${BUILD_DIR}

build4ios() {
    PLATFORM=$1
    ARCHS=$2
    SYSTEM_VERSION=$3
    BUILD_TYPE=$4

    if [ $PLATFORM = "sim" ]; then
        PLATFORM="SIMULATOR64"
    else
        PLATFORM="OS64"
    fi

    #Generate xcode project for debug
    GEN_XCODE_PROJ=""
    if [ $BUILD_TYPE = "Debug" ]; then
        GEN_XCODE_PROJ="-G Xcode"
    fi
    
    cmake .. \
    -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake \
    -DVCPKG_CHAINLOAD_TOOLCHAIN_FILE=cmake/toolchains/Apple/ios.toolchain.cmake \
    -DVCPKG_TARGET_TRIPLET=$VCPKG_TARGET \
    -DCMAKE_SYSTEM_NAME=iOS \
    -DCMAKE_INSTALL_PREFIX=. \
    -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
    -DARCHS=${ARCHS} \
    -DDEPLOYMENT_TARGET=${SYSTEM_VERSION} \
    -DPLATFORM=${PLATFORM} \
    ${GEN_XCODE_PROJ}

    build_with_cmake . . ${BUILD_TYPE}
}

LIB_TYPE="Release"
if [ $DEBUG == 1 ]; then
    LIB_TYPE="Debug"
fi

build4ios phone arm64 ${TARGET_VERSION} ${LIB_TYPE}

#Copy libs
LIB_DIR=""
COMMAND="strip -x -S"
COMMAND_ARG="-o"
if [ $DEBUG == 1 ]; then
    LIB_DIR="Debug-iphoneos/"
    COMMAND="mv"
    COMMAND_ARG=""
fi
LIB_OUTPUT_DIR=output/libs
mkdir -p ${LIB_OUTPUT_DIR}
ARTIFACTS=(
    "${LIB_OUTPUT_DIR}/DynXX-core.a"
    "${LIB_OUTPUT_DIR}/qjs.a"
    "${LIB_OUTPUT_DIR}/mmkvcore.a"
    "${LIB_OUTPUT_DIR}/mmkv.a"
)
${COMMAND} ${LIB_DIR}libDynXX.a ${COMMAND_ARG} ${LIB_OUTPUT_DIR}/DynXX-core.a
${COMMAND} quickjs-build/${LIB_DIR}libqjs.a ${COMMAND_ARG} ${LIB_OUTPUT_DIR}/qjs.a
${COMMAND} mmkv-build/Core/${LIB_DIR}/libmmkvcore.a ${COMMAND_ARG} ${LIB_OUTPUT_DIR}/mmkvcore.a
${COMMAND} mmkv-build/${LIB_DIR}libmmkv.a ${COMMAND_ARG} ${LIB_OUTPUT_DIR}/mmkv.a

#Checking Artifacts
check_artifacts "${ARTIFACTS[@]}"

#Merge libs
final_lib=DynXX.a
merge_libs_apple "${LIB_OUTPUT_DIR}" "${final_lib}"
check_artifacts "${LIB_OUTPUT_DIR}/${final_lib}"
