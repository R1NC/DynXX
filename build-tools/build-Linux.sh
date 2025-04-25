#!/bin/sh

#TODO
declare -i DEBUG=0

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
    -DCMAKE_BUILD_TYPE=${LIB_TYPE}

cmake --build . --config ${LIB_TYPE}

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
mv spdlog.output/libspdlog.a ${LIB_OUTPUT_DIR}/spdlog.a
mv libuv.output/libuv.a ${LIB_OUTPUT_DIR}/uv.a
mv cjson.output/libcjson.a ${LIB_OUTPUT_DIR}/cjson.a
mv sqlite.output/libsqlite3.a ${LIB_OUTPUT_DIR}/sqlite3.a
mv mmkv.output/Core/libmmkvcore.a ${LIB_OUTPUT_DIR}/mmkvcore.a
mv mmkv.output/libmmkv.a ${LIB_OUTPUT_DIR}/mmkv.a
mv AdaURL.output/src/libada.a ${LIB_OUTPUT_DIR}/ada.a