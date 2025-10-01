#!/bin/bash

source "$(dirname "$0")/build-utils.sh"

#TODO
VCPKG_ROOT=${VCPKG_ROOT:-"$HOME/dev/vcpkg/"}
VCPKG_TARGET=arm64-osx

DEBUG=0
TARGET_VERSION=14.0

cd ..
export VCPKG_BINARY_SOURCES="clear;default,readwrite"
$VCPKG_ROOT/vcpkg install --triplet=${VCPKG_TARGET}

BUILD_DIR=build.macOS
rm -rf ${BUILD_DIR}
mkdir -p ${BUILD_DIR}
cd ${BUILD_DIR}

build4mac() {
    ARCHS=$1
    SYSTEM_VERSION=$2
    BUILD_TYPE=$3

    GEN_XCODE_PROJ=""
    if [ $BUILD_TYPE = "Debug" ]; then
        GEN_XCODE_PROJ="-GXcode"
    fi
    
    cmake .. \
    -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake \
    -DVCPKG_CHAINLOAD_TOOLCHAIN_FILE=cmake/toolchains/Apple/ios.toolchain.cmake \
    -DVCPKG_TARGET_TRIPLET=$VCPKG_TARGET \
    -DCMAKE_INSTALL_PREFIX=. \
    -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
    -DARCHS=${ARCHS} \
    -DPLATFORM=MAC_UNIVERSAL \
    -DDEPLOYMENT_TARGET=${SYSTEM_VERSION} \
    ${GEN_XCODE_PROJ}

    build_with_cmake . . ${BUILD_TYPE}
}

LIB_TYPE="Release"
if [ $DEBUG == 1 ]; then
    LIB_TYPE="Debug"
fi

build4mac arm64 ${TARGET_VERSION} ${LIB_TYPE}
#build4mac x86_64 ${TARGET_VERSION} ${LIB_TYPE}

#Copy libs
LIB_OUTPUT_DIR=output/libs
mkdir -p ${LIB_OUTPUT_DIR}
ARTIFACTS=(
    "${LIB_OUTPUT_DIR}/DynXX-core.a"
    "${LIB_OUTPUT_DIR}/qjs.a"
    "${LIB_OUTPUT_DIR}/mmkvcore.a"
    "${LIB_OUTPUT_DIR}/mmkv.a"
)
mv libDynXX.a ${LIB_OUTPUT_DIR}/DynXX-core.a
mv quickjs-build/libqjs.a ${LIB_OUTPUT_DIR}/qjs.a
mv mmkv-build/Core/libmmkvcore.a ${LIB_OUTPUT_DIR}/mmkvcore.a
mv mmkv-build/libmmkv.a ${LIB_OUTPUT_DIR}/mmkv.a

#Copy executables
TOOLS_OUTPUT_DIR=output/tools
mkdir -p ${TOOLS_OUTPUT_DIR}
qjsc=${TOOLS_OUTPUT_DIR}/qjsc
mv quickjs-build/qjsc.app/Contents/MacOS/qjsc ${qjsc}
ARTIFACTS+=(${qjsc})

#Checking Artifacts
check_artifacts "${ARTIFACTS[@]}"

#Merge libs
final_lib=DynXX.a
merge_libs_apple "${LIB_OUTPUT_DIR}" "${final_lib}"
check_artifacts "${LIB_OUTPUT_DIR}/${final_lib}"
