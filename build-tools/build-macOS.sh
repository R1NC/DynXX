#!/bin/sh

#TODO
declare -i DEBUG=0
TARGET_VERSION=11.0

BUILD_DIR=../build.macOS
rm -rf ${BUILD_DIR}
mkdir -p ${BUILD_DIR}
cd ${BUILD_DIR}

function build4macos {
    PLATFORM=$1
    ARCHS=$2
    SYSTEM_VERSION=$3
    BUILD_TYPE=$4

    #Generate xcode project for debug
    GEN_XCODE_PROJ=""
    if [ $BUILD_TYPE = "Debug" ]; then
        GEN_XCODE_PROJ="-GXcode"
    fi
    
    cmake .. \
    -DCMAKE_TOOLCHAIN_FILE=build-tools/toolchains/Apple/apple.toolchain.cmake \
    -DCMAKE_INSTALL_PREFIX=. \
    -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
    -DARCHS=${ARCHS} \
    -DPLATFORM=${PLATFORM} \
    -DDEPLOYMENT_TARGET=${SYSTEM_VERSION} \
    ${GEN_XCODE_PROJ}

    cmake --build . --config ${BUILD_TYPE}
}

LIB_TYPE="Release"
if [ $DEBUG == 1 ]; then
    LIB_TYPE="Debug"
fi

build4macos MAC_UNIVERSAL arm64 ${TARGET_VERSION} ${LIB_TYPE}
#build4macos MAC_UNIVERSAL x86_64 ${TARGET_VERSION} ${LIB_TYPE}

#Copy headers
HEADER_OUTPUT_DIR=output/include
mkdir -p ${HEADER_OUTPUT_DIR}
cp -R ../include/ ${HEADER_OUTPUT_DIR}/

#Copy libs
LIB_OUTPUT_DIR=output/libs
mkdir -p ${LIB_OUTPUT_DIR}
mv libNGenXX.a ${LIB_OUTPUT_DIR}/NGenXX.a
mv curl.output/lib/libcurl.a ${LIB_OUTPUT_DIR}/curl.a
mv openssl.output/ssl/libssl.a ${LIB_OUTPUT_DIR}/ssl.a
mv openssl.output/crypto/libcrypto.a ${LIB_OUTPUT_DIR}/crypto.a
mv lua.output/liblua.a ${LIB_OUTPUT_DIR}/lua.a
mv quickjs.output/libqjs.a ${LIB_OUTPUT_DIR}/qjs.a
mv libuv.output/libuv.a ${LIB_OUTPUT_DIR}/uv.a
mv cjson.output/libcjson.a ${LIB_OUTPUT_DIR}/cjson.a
mv mmkv.output/Core/libmmkvcore.a ${LIB_OUTPUT_DIR}/mmkvcore.a
mv mmkv.output/libmmkv.a ${LIB_OUTPUT_DIR}/mmkv.a
ADA_OUT_FILE=AdaURL.output/src/libada.a
if [ -f "$ADA_OUT_FILE" ]; then
    mv ${ADA_OUT_FILE} ${LIB_OUTPUT_DIR}/ada.a
fi

#Copy executables
TOOLS_OUTPUT_DIR=output/tools
mkdir -p ${TOOLS_OUTPUT_DIR}
mv quickjs.output/qjsc.app/Contents/MacOS/qjsc ${TOOLS_OUTPUT_DIR}/qjsc
