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

export BUILD_FOLDER=build.${PLATFORM}
OUTPUT_FOLDER=${BUILD_FOLDER}/output
export OUTPUT_LIB_PATH=$PWD/${OUTPUT_FOLDER}/libs
export OUTPUT_EXE_PATH=$PWD/${OUTPUT_FOLDER}/exe

export DARWIN_TOOLCHAIN_PATH=$PWD/cmake/toolchains/Apple/ios.toolchain.cmake
export DARWIN_PLATFORM="MAC_UNIVERSAL"
export DARWIN_ARCHS="arm64"
export DARWIN_SYSTEM_VERSION="14.0"

export VCPKG_ROOT=${VCPKG_ROOT:-"$HOME/dev/vcpkg/"}
export VCPKG_BINARY_SOURCES="default,read"
export VCPKG_TARGET=arm64-osx

rm -rf ${BUILD_FOLDER}

cmake --preset ${PRESET}

$VCPKG_ROOT/vcpkg install --triplet=${VCPKG_TARGET}

cmake --build --preset ${PRESET}
cmake --install ${BUILD_FOLDER} --prefix ${OUTPUT_FOLDER} --component headers

ARTIFACTS=(
    "${OUTPUT_LIB_PATH}/libDynXX.a"
    "${OUTPUT_LIB_PATH}/libqjs.a"
    "${OUTPUT_LIB_PATH}/libmmkvcore.a"
    "${OUTPUT_LIB_PATH}/libmmkv.a"
    "${OUTPUT_EXE_PATH}/qjsc.app/Contents/MacOS/qjsc"
)
check_artifacts "${ARTIFACTS[@]}"

final_lib=DynXX.a
merge_libs_apple "${OUTPUT_LIB_PATH}" "${final_lib}"
check_artifacts "${OUTPUT_LIB_PATH}/${final_lib}"
