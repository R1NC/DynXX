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

export BUILD_FOLDER=build.${PLATFORM}/${BUILD_TYPE}
OUTPUT_FOLDER=${BUILD_FOLDER}/output
OUTPUT_PATH=$PWD/${OUTPUT_FOLDER}/${OHOS_ABI}
export OUTPUT_LIB_PATH=${OUTPUT_PATH}/lib
export OUTPUT_DLL_PATH=${OUTPUT_PATH}/share
export OUTPUT_EXE_PATH=${OUTPUT_PATH}/bin
rm -rf ${BUILD_FOLDER}

cmake --preset ${PRESET}
cmake --build --preset ${PRESET}
cmake --install ${BUILD_FOLDER} --prefix ${OUTPUT_FOLDER} --component headers

export_compile_commands ${BUILD_FOLDER}

ARTIFACTS=(
    "${OUTPUT_LIB_PATH}/libDynXX.a"
)
check_artifacts "${ARTIFACTS[@]}"

AR_TOOL=$OHOS_ROOT/llvm/bin/llvm-ar
FINAL_LIB=libDynXX-All.a
merge_libs "${OUTPUT_LIB_PATH}" "${FINAL_LIB}" "${AR_TOOL}"
check_artifacts "${OUTPUT_LIB_PATH}/${FINAL_LIB}"
