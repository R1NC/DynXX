#!/bin/bash

source "$(dirname "$0")/build-utils.sh"

NDK_ROOT=${NDK_ROOT:-"~/Library/Android/sdk/ndk/29.0.13846066/"}
ARCH=arm64-v8a
VER=24

BUILD_DIR=../build.Android
rm -rf ${BUILD_DIR}
mkdir -p ${BUILD_DIR}
cd ${BUILD_DIR}

build4android() {
  ABI=$1
  SYSTEM_VERSION=$2
  BUILD_TYPE=$3

  ROOT_DIR=$(pwd)
  ABI_BUILD_DIR=${ROOT_DIR}/${ABI}
  OUTPUT_DIR=${ROOT_DIR}/output/libs/${ABI}
  mkdir -p ${ABI_BUILD_DIR}
  mkdir -p ${OUTPUT_DIR}

  cmake .. \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
    -DCMAKE_SYSTEM_NAME=Android \
    -DCMAKE_ANDROID_NDK=$NDK_ROOT \
    -DCMAKE_TOOLCHAIN_FILE=$NDK_ROOT/build/cmake/android.toolchain.cmake \
    -DANDROID_TOOLCHAIN=clang \
    -DCMAKE_INSTALL_PREFIX=. \
    -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
    -DANDROID_ABI=${ABI} \
    -B${ABI_BUILD_DIR} \
    -DCMAKE_RUNTIME_OUTPUT_DIRECTORY=${OUTPUT_DIR} \
    -DCMAKE_LIBRARY_OUTPUT_DIRECTORY=${OUTPUT_DIR} \
    -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY=${OUTPUT_DIR} \
    -DANDROID_PLATFORM=${SYSTEM_VERSION}

  build_with_cmake ${ABI_BUILD_DIR} ${ROOT_DIR} ${BUILD_TYPE}
  
  rm -rf ${ABI_BUILD_DIR}
}

build4android ${ARCH} android-${VER} Release

ASSETS_APP_DIR=../platforms/Android/app/src/main/assets/
mkdir -p ${ASSETS_APP_DIR}
cp ../scripts/JS/DynXX.js ${ASSETS_APP_DIR}
cp -a ../scripts/Lua/. ${ASSETS_APP_DIR}
BIZ_JS=../scripts/JS/dist/biz.js
if [ -f "$BIZ_JS" ]; then
  cp ${BIZ_JS} ${ASSETS_APP_DIR}
else
  echo "biz.js does not exist!"
fi

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
    "${LIB_OUTPUT_DIR}/libuv.a"
    "${LIB_OUTPUT_DIR}/libcjson.a"
    "${LIB_OUTPUT_DIR}/libmmkvcore.a"
    "${LIB_OUTPUT_DIR}/libmmkv.a"
)
check_artifacts "${ARTIFACTS[@]}"