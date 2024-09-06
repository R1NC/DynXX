#!/bin/sh

#TODO
declare -i DEBUG=0

BUILD_DIR=../build.macOS
rm -rf ${BUILD_DIR}
mkdir -p ${BUILD_DIR}
cd ${BUILD_DIR}

function build4macos {
    PLATFORM=$1
    BUILD_TYPE=$2

    #Generate xcode project for debug
    GEN_XCODE_PROJ=""
    if [[ $BUILD_TYPE = "Debug" ]]
    then
        GEN_XCODE_PROJ="-GXcode"
    fi
    
    cmake ../build-tools \
    -DCMAKE_TOOLCHAIN_FILE=toolchains/Apple/apple.toolchain.cmake \
    -DCMAKE_INSTALL_PREFIX=. \
    -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
    -DARCHS=${ARCHS} \
    -DPLATFORM=${PLATFORM} \
    ${GEN_XCODE_PROJ} \

    cmake --build . --config ${BUILD_TYPE}
}

LIB_TYPE="Release"
if [[ $DEBUG == 1 ]]
then
    LIB_TYPE="Debug"
fi

build4macos MAC_UNIVERSAL $LIB_TYPE

#Copy libs
LIB_OUTPUT_DIR=output/${LIB_TYPE}
mkdir -p ${LIB_OUTPUT_DIR}
mv libNGenXX.a ${LIB_OUTPUT_DIR}/NGenXX.a
mv curl.output/lib/libcurl.a ${LIB_OUTPUT_DIR}/curl.a
#mv openssl.output/ssl/libssl.a ${LIB_OUTPUT_DIR}/ssl.a
#mv openssl.output/crypto/libcrypto.a ${LIB_OUTPUT_DIR}/crypto.a
mv wolfssl.output/libwolfssl.a ${LIB_OUTPUT_DIR}/wolfssl.a
mv lua.output/liblua.a ${LIB_OUTPUT_DIR}/lua.a
mv cjson.output/libcjson.a ${LIB_OUTPUT_DIR}/cjson.a
mv mmkv.output/libmmkv.a ${LIB_OUTPUT_DIR}/mmkv.a

#Copy headers
HEADER_OUTPUT_DIR=output/include
mkdir -p ${HEADER_OUTPUT_DIR}
cp -R ../include/ ${HEADER_OUTPUT_DIR}/
