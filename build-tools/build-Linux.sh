#!/bin/bash

#TODO
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
    -DCMAKE_BUILD_TYPE=${LIB_TYPE}

cmake --build . --config ${LIB_TYPE}

#Copy headers
HEADER_OUTPUT_DIR=output/include
mkdir -p ${HEADER_OUTPUT_DIR}
cp -R ../include/ ${HEADER_OUTPUT_DIR}/

#Copy libs
LIB_OUTPUT_DIR=output/libs
mkdir -p ${LIB_OUTPUT_DIR}
ARTIFACTS=(
    "${LIB_OUTPUT_DIR}/NGenXX.a"
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
mv libNGenXX.a ${LIB_OUTPUT_DIR}/NGenXX.a
mv _deps/curl-build/lib/libcurl.a ${LIB_OUTPUT_DIR}/curl.a
mv _deps/openssl-build/ssl/libssl.a ${LIB_OUTPUT_DIR}/ssl.a
mv _deps/openssl-build/crypto/libcrypto.a ${LIB_OUTPUT_DIR}/crypto.a
mv _deps/lua-build/liblua.a ${LIB_OUTPUT_DIR}/lua.a
mv _deps/quickjs-build/libqjs.a ${LIB_OUTPUT_DIR}/qjs.a
mv _deps/spdlog-build/libspdlog.a ${LIB_OUTPUT_DIR}/spdlog.a
mv _deps/libuv-build/libuv.a ${LIB_OUTPUT_DIR}/uv.a
mv _deps/cjson-build/libcjson.a ${LIB_OUTPUT_DIR}/cjson.a
mv _deps/sqlite-build/libsqlite3.a ${LIB_OUTPUT_DIR}/sqlite3.a
mv _deps/mmkv-build/Core/libmmkvcore.a ${LIB_OUTPUT_DIR}/mmkvcore.a
mv _deps/mmkv-build/libmmkv.a ${LIB_OUTPUT_DIR}/mmkv.a

ADA_OUT_FILE=_deps/AdaURL-build/src/libada.a
if [ -f "$ADA_OUT_FILE" ]; then
    libAda=${LIB_OUTPUT_DIR}/ada.a
    mv ${ADA_OUT_FILE} ${libAda}
    ARTIFACTS+=("${libAda}")
fi

#Checking Artifacts
for a in "${ARTIFACTS[@]}"; do
    if [ ! -f "$a" ]; then
        echo "ARTIFACT NOT FOUND: $a"
        exit 1
    fi
done