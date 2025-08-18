#!/bin/bash

#TODO
DEBUG=0
TARGET_VERSION=14.0

BUILD_DIR=../build.macOS
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
    -DCMAKE_TOOLCHAIN_FILE=build-tools/toolchains/Apple/apple.toolchain.cmake \
    -DCMAKE_INSTALL_PREFIX=. \
    -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
    -DARCHS=${ARCHS} \
    -DPLATFORM=MAC_UNIVERSAL \
    -DDEPLOYMENT_TARGET=${SYSTEM_VERSION} \
    ${GEN_XCODE_PROJ}

    cmake --build . --config ${BUILD_TYPE}
    cmake --install . --prefix ${BUILD_DIR}/output --component headers
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
    "${LIB_OUTPUT_DIR}/DynXX.a"
    "${LIB_OUTPUT_DIR}/curl.a"
    "${LIB_OUTPUT_DIR}/ssl.a"
    "${LIB_OUTPUT_DIR}/crypto.a"
    "${LIB_OUTPUT_DIR}/lua.a"
    "${LIB_OUTPUT_DIR}/qjs.a"
    "${LIB_OUTPUT_DIR}/spdlog.a"
    "${LIB_OUTPUT_DIR}/uv.a"
    "${LIB_OUTPUT_DIR}/cjson.a"
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
mv mmkv-build/Core/libmmkvcore.a ${LIB_OUTPUT_DIR}/mmkvcore.a
mv mmkv-build/libmmkv.a ${LIB_OUTPUT_DIR}/mmkv.a
ADA_OUT_FILE=AdaURL-build/src/libada.a
if [ -f "$ADA_OUT_FILE" ]; then
    libAda=${LIB_OUTPUT_DIR}/ada.a
    mv ${ADA_OUT_FILE} ${libAda}
    ARTIFACTS+=(${libAda})
fi

#Copy executables
TOOLS_OUTPUT_DIR=output/tools
mkdir -p ${TOOLS_OUTPUT_DIR}
qjsc=${TOOLS_OUTPUT_DIR}/qjsc
mv quickjs-build/qjsc.app/Contents/MacOS/qjsc ${qjsc}
ARTIFACTS+=(${qjsc})

#Checking Artifacts
for a in "${ARTIFACTS[@]}"; do
    if [ ! -f "$a" ]; then
        echo "ARTIFACT NOT FOUND: $a"
        exit 1
    fi
done
