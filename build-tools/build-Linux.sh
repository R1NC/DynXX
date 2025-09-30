#!/bin/bash

source "$(dirname "$0")/build-utils.sh"

#TODO
VCPKG_ROOT=${VCPKG_ROOT:-"$HOME/dev/vcpkg/"}
VCPKG_TARGET=x64-linux
DEBUG=0

BUILD_DIR=../build.Linux
rm -rf ${BUILD_DIR}
mkdir -p ${BUILD_DIR}
cd ${BUILD_DIR}

LIB_TYPE="Release"
if [ $DEBUG == 1 ]; then
    LIB_TYPE="Debug"
fi

cmake .. \
    -DCMAKE_INSTALL_PREFIX=. \
    -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake \
    -DVCPKG_TARGET_TRIPLET=$VCPKG_TARGET \
    -DCMAKE_BUILD_TYPE=${LIB_TYPE}

build_with_cmake . ${BUILD_DIR} ${LIB_TYPE}

#Copy libs
LIB_OUTPUT_DIR=output/libs
mkdir -p ${LIB_OUTPUT_DIR}
ARTIFACTS=(
    "${LIB_OUTPUT_DIR}/DynXX.a"
    "${LIB_OUTPUT_DIR}/qjs.a"
    "${LIB_OUTPUT_DIR}/mmkvcore.a"
    "${LIB_OUTPUT_DIR}/mmkv.a"
)
mv libDynXX.a ${LIB_OUTPUT_DIR}/DynXX.a
mv curl-build/lib/libcurl.a ${LIB_OUTPUT_DIR}/curl.a
mv openssl-build/ssl/libssl.a ${LIB_OUTPUT_DIR}/ssl.a
mv openssl-build/crypto/libcrypto.a ${LIB_OUTPUT_DIR}/crypto.a
mv lua-build/liblua.a ${LIB_OUTPUT_DIR}/lua.a
mv quickjs-build/libqjs.a ${LIB_OUTPUT_DIR}/qjs.a
mv spdlog-build/libspdlog.a ${LIB_OUTPUT_DIR}/spdlog.a
mv libuv-build/libuv.a ${LIB_OUTPUT_DIR}/uv.a
mv cjson-build/libcjson.a ${LIB_OUTPUT_DIR}/cjson.a
mv sqlite-build/libsqlite3.a ${LIB_OUTPUT_DIR}/sqlite3.a
mv mmkv-build/Core/libmmkvcore.a ${LIB_OUTPUT_DIR}/mmkvcore.a
mv mmkv-build/libmmkv.a ${LIB_OUTPUT_DIR}/mmkv.a

ADA_OUT_FILE=AdaURL-build/src/libada.a
if [ -f "$ADA_OUT_FILE" ]; then
    libAda=${LIB_OUTPUT_DIR}/ada.a
    mv ${ADA_OUT_FILE} ${libAda}
    ARTIFACTS+=("${libAda}")
fi

#Checking Artifacts
check_artifacts "${ARTIFACTS[@]}"
