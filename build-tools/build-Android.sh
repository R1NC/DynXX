#!/bin/sh

# Use NDK_ROOT from environment variable, with a fallback default
NDK_ROOT=${NDK_ROOT:-"~/Library/Android/sdk/ndk/28.1.13356709/"}

BUILD_DIR=../build.Android
rm -rf ${BUILD_DIR}
mkdir -p ${BUILD_DIR}
cd ${BUILD_DIR}

function build4android {
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

  pushd ${ABI_BUILD_DIR}
    make -j5
  popd

  rm -rf ${ABI_BUILD_DIR}
}

#build4android armeabi-v7a android-21 Release
build4android arm64-v8a android-21 Release

HEADER_OUTPUT_DIR=output/include
mkdir -p ${HEADER_OUTPUT_DIR}
cp -R ../include/ ${HEADER_OUTPUT_DIR}/

ASSETS_APP_DIR=../platforms/Android/app/src/main/assets/
mkdir -p ${ASSETS_APP_DIR}
cp ../scripts/JS/NGenXX.js ${ASSETS_APP_DIR}
cp -a ../scripts/Lua/. ${ASSETS_APP_DIR}
BIZ_JS=../scripts/JS/dist/biz.js
if [ -f "$BIZ_JS" ]; then
  cp ${BIZ_JS} ${ASSETS_APP_DIR}
else
  echo "biz.js does not exist!"
fi
