#!/bin/bash

source "$(dirname "$0")/build-utils.sh"

# TODO
OHOS_ROOT=${OHOS_ROOT:-"$HOME/Library/OpenHarmony/Sdk/20/native"}
AR_PATH=$OHOS_ROOT/toolchains/llvm/bin
ARCH=arm64-v8a

BUILD_DIR=../build.HarmonyOS
rm -rf ${BUILD_DIR}
mkdir -p ${BUILD_DIR}
cd ${BUILD_DIR}

build4harmony() {
  ABI=$1
  BUILD_TYPE=$2

  ROOT_DIR=$(pwd)
  ABI_BUILD_DIR=${ROOT_DIR}/${ABI}
  OUTPUT_DIR=${ROOT_DIR}/output/libs/${ABI}
  mkdir -p ${ABI_BUILD_DIR}
  mkdir -p ${OUTPUT_DIR}

  cmake .. \
    -DOHOS_PLATFORM=OHOS \
    -DOHOS_ARCH=${ABI} \
    -DCMAKE_RUNTIME_OUTPUT_DIRECTORY=${OUTPUT_DIR} \
    -DCMAKE_LIBRARY_OUTPUT_DIRECTORY=${OUTPUT_DIR} \
    -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY=${OUTPUT_DIR} \
    -B${ABI_BUILD_DIR} \
    -DCMAKE_TOOLCHAIN_FILE=$OHOS_ROOT/build/cmake/ohos.toolchain.cmake

  build_with_cmake ${ABI_BUILD_DIR} ${ROOT_DIR} ${BUILD_TYPE}

  rm -rf ${ABI_BUILD_DIR}
}

build4harmony ${ARCH} Release

#Checking Artifacts
LIB_OUTPUT_DIR=output/libs/${ARCH}
ARTIFACTS=(
    "${LIB_OUTPUT_DIR}/libDynXX.a"
    "${LIB_OUTPUT_DIR}/libcurl.a"
    "${LIB_OUTPUT_DIR}/libssl.a"
    "${LIB_OUTPUT_DIR}/libcrypto.a"
    "${LIB_OUTPUT_DIR}/liblua.a"
    "${LIB_OUTPUT_DIR}/libqjs.a"
    "${LIB_OUTPUT_DIR}/libspdlog.a"
    "${LIB_OUTPUT_DIR}/libcjson.a"
    "${LIB_OUTPUT_DIR}/libmmkvcore.a"
    "${LIB_OUTPUT_DIR}/libmmkv.a"
)
check_artifacts "${ARTIFACTS[@]}"
