#!/bin/bash

# TODO
OHOS_ROOT=${OHOS_ROOT:-"~/Library/OpenHarmony/Sdk/15/native"}
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

  cmake --build . --config ${BUILD_TYPE}

  pushd ${ABI_BUILD_DIR}
    make -j5
  popd

  rm -rf ${ABI_BUILD_DIR}
}

build4harmony ${ARCH} Release

HEADER_OUTPUT_DIR=output/include
mkdir -p ${HEADER_OUTPUT_DIR}
cp -R ../include/ ${HEADER_OUTPUT_DIR}/

#Checking Artifacts
LIB_OUTPUT_DIR=output/libs/${ARCH}
ARTIFACTS=(
    "${LIB_OUTPUT_DIR}/libNGenXX.a"
    "${LIB_OUTPUT_DIR}/libcurl.a"
    "${LIB_OUTPUT_DIR}/libssl.a"
    "${LIB_OUTPUT_DIR}/libcrypto.a"
    "${LIB_OUTPUT_DIR}/liblua.a"
    "${LIB_OUTPUT_DIR}/libqjs.a"
    "${LIB_OUTPUT_DIR}/libspdlog.a"
    "${LIB_OUTPUT_DIR}/libuv.a"
    "${LIB_OUTPUT_DIR}/libcjson.a"
    "${LIB_OUTPUT_DIR}/libmmkvcore.a"
    "${LIB_OUTPUT_DIR}/libmmkv.a"
)
for a in "${ARTIFACTS[@]}"; do
    if [ ! -f "$a" ]; then
        echo "ARTIFACT NOT FOUND: $a"
        exit 1
    fi
done