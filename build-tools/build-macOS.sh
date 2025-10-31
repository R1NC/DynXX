#!/bin/bash
source "$(dirname "$0")/build-utils.sh"

cd ..

DEBUG=0
BUILD_TYPE="Release"
if [ $DEBUG == 1 ]; then
    BUILD_TYPE="Debug"
fi

PLATFORM=macOS
PRESET=${PLATFORM}-${BUILD_TYPE}

export APPLE_TOOLCHAIN_FILE=$PWD/cmake/toolchains/Apple/ios.toolchain.cmake
export APPLE_PLATFORM="MAC_UNIVERSAL"
export APPLE_ABI="arm64"
export APPLE_VER="14.0"

export BUILD_FOLDER=build.${PLATFORM}/${BUILD_TYPE}
OUTPUT_FOLDER=${BUILD_FOLDER}/output
OUTPUT_PATH=$PWD/${OUTPUT_FOLDER}/${APPLE_ABI}
export OUTPUT_LIB_PATH=${OUTPUT_PATH}/lib
export OUTPUT_DLL_PATH=${OUTPUT_PATH}/share
export OUTPUT_EXE_PATH=${OUTPUT_PATH}/bin

export VCPKG_ROOT=${CI_VCPKG_ROOT:-"$HOME/dev/vcpkg"}
export VCPKG_BINARY_SOURCES=${CI_VCPKG_BINARY_SOURCES:-"files,$HOME/vcpkg-binary-cache,readwrite"}
export VCPKG_TARGET_TRIPLET=${APPLE_ABI}-osx
VCPKG_LIB_PATH=$PWD/${BUILD_FOLDER}/vcpkg_installed/${VCPKG_TARGET}/lib

cmake --preset ${PRESET}
cmake --build --preset ${PRESET}
cmake --install ${BUILD_FOLDER} --prefix ${OUTPUT_FOLDER} --component headers

export_compile_commands ${BUILD_FOLDER}

ARTIFACTS=(
    "${OUTPUT_LIB_PATH}/libDynXX.a"
    "${OUTPUT_EXE_PATH}/qjsc.app/Contents/MacOS/qjsc"
)
check_artifacts "${ARTIFACTS[@]}"

cp "${VCPKG_LIB_PATH}"/*.a "${OUTPUT_LIB_PATH}/"

AR_TOOL=libtool
FINAL_LIB=libDynXX-All.a
merge_libs "${OUTPUT_LIB_PATH}" "${FINAL_LIB}" "${AR_TOOL}"
check_artifacts "${OUTPUT_LIB_PATH}/${FINAL_LIB}"
