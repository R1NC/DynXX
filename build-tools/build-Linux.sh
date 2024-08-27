#!/bin/sh

#TODO
declare -i DEBUG=0

BUILD_DIR=../build.Linux
rm -rf ${BUILD_DIR}
mkdir -p ${BUILD_DIR}
cd ${BUILD_DIR}

LIB_TYPE="Release"
if [[ $DEBUG == 1 ]]
then
    LIB_TYPE="Debug"
fi

cmake ../build-tools \
    -DCMAKE_INSTALL_PREFIX=. \
    -DCMAKE_BUILD_TYPE=${LIB_TYPE}

cmake --build . --config ${LIB_TYPE}

#Copy libs
LIB_OUTPUT_DIR=output/${LIB_TYPE}
mkdir -p ${LIB_OUTPUT_DIR}
mv libEngineXX.a ${LIB_OUTPUT_DIR}/EngineXX.a
mv curl.output/lib/libcurl.a ${LIB_OUTPUT_DIR}/curl.a
#mv openssl.output/ssl/libssl.a ${LIB_OUTPUT_DIR}/ssl.a
#mv openssl.output/crypto/libcrypto.a ${LIB_OUTPUT_DIR}/crypto.a
mv wolfssl.output/libwolfssl.a ${LIB_OUTPUT_DIR}/wolfssl.a
mv lua.output/liblua.a ${LIB_OUTPUT_DIR}/lua.a
mv cjson.output/libcjson.a ${LIB_OUTPUT_DIR}/cjson.a

#Copy headers
HEADER_OUTPUT_DIR=output/include
mkdir -p ${HEADER_OUTPUT_DIR}
cp ../include/EngineXX.h ${HEADER_OUTPUT_DIR}
cp ../include/EngineXXLua.h ${HEADER_OUTPUT_DIR}
cp ../include/EngineXXNet.h ${HEADER_OUTPUT_DIR}
