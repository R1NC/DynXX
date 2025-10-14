#!/bin/bash
source "$(dirname "$0")/build-utils.sh"

cd ..

DEBUG=0
BUILD_TYPE="Release"
if [ $DEBUG == 1 ]; then
    BUILD_TYPE="Debug"
fi

PLATFORM=Android
PRESET=${PLATFORM}-${BUILD_TYPE}

NDK_ROOT=${NDK_ROOT:-"$HOME/Library/Android/sdk/ndk/29.0.14206865/"}
export ANDROID_NDK=$NDK_ROOT
export ANDROID_NDK_HOME=$NDK_ROOT
export ANDROID_ABI=arm64-v8a
export ANDROID_VER=android-24

export BUILD_FOLDER=build.${PLATFORM}
OUTPUT_FOLDER=${BUILD_FOLDER}/output
export OUTPUT_LIB_PATH=$PWD/${OUTPUT_FOLDER}/${ANDROID_ABI}/libs
export OUTPUT_EXE_PATH=$PWD/${OUTPUT_FOLDER}/${ANDROID_ABI}/exe
rm -rf ${BUILD_FOLDER}

export VCPKG_ROOT=${VCPKG_ROOT:-"$HOME/dev/vcpkg/"}
export VCPKG_BINARY_SOURCES="default,read"
export VCPKG_TARGET=arm64-android
$VCPKG_ROOT/vcpkg install --triplet=${VCPKG_TARGET}

cmake --preset ${PRESET}
cmake --build --preset ${PRESET}
cmake --install ${BUILD_FOLDER} --prefix ${OUTPUT_FOLDER} --component headers

# AR_PATH=$NDK_ROOT/toolchains/llvm/prebuilt/darwin-x86_64/bin

ARTIFACTS=(
    "${OUTPUT_LIB_PATH}/libDynXX.a"
    "${OUTPUT_LIB_PATH}/libqjs.a"
    "${OUTPUT_LIB_PATH}/libmmkvcore.a"
    "${OUTPUT_LIB_PATH}/libmmkv.a"
)
check_artifacts "${ARTIFACTS[@]}"
