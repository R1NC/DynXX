#!/bin/bash
source "$(dirname "$0")/build-utils.sh"

cd ..

DEBUG=0
BUILD_TYPE="Release"
if [ $DEBUG == 1 ]; then
    BUILD_TYPE="Debug"
fi

PLATFORM=HarmonyOS
PRESET=${PLATFORM}-${BUILD_TYPE}

export OHOS_ROOT=${OHOS_ROOT:-"$HOME/Library/OpenHarmony/Sdk/20/native"}
export OHOS_ABI=arm64-v8a

export BUILD_FOLDER=build.${PLATFORM}
OUTPUT_FOLDER=${BUILD_FOLDER}/output
export OUTPUT_LIB_PATH=$PWD/${OUTPUT_FOLDER}/${OHOS_ABI}/libs
export OUTPUT_EXE_PATH=$PWD/${OUTPUT_FOLDER}/${OHOS_ABI}/exe
rm -rf ${BUILD_FOLDER}

cmake --preset ${PRESET}
cmake --build --preset ${PRESET}
cmake --install ${BUILD_FOLDER} --prefix ${OUTPUT_FOLDER} --component headers

# AR_PATH=$OHOS_ROOT/toolchains/llvm/bin

ARTIFACTS=(
    "${OUTPUT_LIB_PATH}/libDynXX.a"
    "${OUTPUT_LIB_PATH}/libqjs.a"
    "${OUTPUT_LIB_PATH}/libmmkvcore.a"
    "${OUTPUT_LIB_PATH}/libmmkv.a"
)
check_artifacts "${ARTIFACTS[@]}"
