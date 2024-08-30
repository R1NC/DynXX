#!/bin/sh

# TODO
NDK_ROOT="~/Library/OpenHarmony/Sdk/12/native"

BUILD_DIR=../build.HarmonyOS
rm -rf ${BUILD_DIR}
mkdir -p ${BUILD_DIR}
cd ${BUILD_DIR}

function build4harmony {
  ABI=$1
  API_LEVEL=$2
  BUILD_TYPE=$3

  ROOT_DIR=$(pwd)
  ABI_BUILD_DIR=${ROOT_DIR}/${ABI}
  OUTPUT_DIR=${ROOT_DIR}/output/${BUILD_TYPE}/${ABI}
  mkdir -p ${ABI_BUILD_DIR}
  mkdir -p ${OUTPUT_DIR}

  cmake ../build-tools \
    -DOHOS_PLATFORM=OHOS \
    -DOHOS_PLATFORM_LEVEL=${API_LEVEL} \
    -DOHOS_ARCH=${ABI} \
    -DCMAKE_RUNTIME_OUTPUT_DIRECTORY=${OUTPUT_DIR} \
    -DCMAKE_LIBRARY_OUTPUT_DIRECTORY=${OUTPUT_DIR} \
    -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY=${OUTPUT_DIR} \
    -B${ABI_BUILD_DIR} \
    -DCMAKE_TOOLCHAIN_FILE=$NDK_ROOT/build/cmake/ohos.toolchain.cmake \

  cmake --build . --config ${BUILD_TYPE}

  pushd ${ABI_BUILD_DIR}
    make -j5
  popd

  rm -rf ${ABI_BUILD_DIR}
}

#build4harmony armeabi-v7a 12 Release
build4harmony arm64-v8a 12 Release

HEADER_OUTPUT_DIR=output/include
mkdir -p ${HEADER_OUTPUT_DIR}
cp ../include/NGenXX.h ${HEADER_OUTPUT_DIR}
cp ../include/NGenXXLog.h ${HEADER_OUTPUT_DIR}
cp ../include/NGenXXLua.h ${HEADER_OUTPUT_DIR}
cp ../include/NGenXXNet.h ${HEADER_OUTPUT_DIR}