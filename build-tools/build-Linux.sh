2#!/bin/bash
source "$(dirname "$0")/build-utils.sh"

cd ..

DEBUG=0
BUILD_TYPE="Release"
if [ $DEBUG == 1 ]; then
    BUILD_TYPE="Debug"
fi

PLATFORM=Linux
PRESET=${PLATFORM}-${BUILD_TYPE}

LINUX_ABI=x64

export BUILD_FOLDER=build.${PLATFORM}/${BUILD_TYPE}
OUTPUT_FOLDER=${BUILD_FOLDER}/output
export OUTPUT_LIB_PATH=$PWD/${OUTPUT_FOLDER}/${LINUX_ABI}/libs
export OUTPUT_EXE_PATH=$PWD/${OUTPUT_FOLDER}/${LINUX_ABI}/exe
rm -rf ${BUILD_FOLDER}

export VCPKG_ROOT=${VCPKG_ROOT:-"$HOME/dev/vcpkg/"}
export VCPKG_BINARY_SOURCES="default,read"
export VCPKG_TARGET=${LINUX_ABI}-linux
VCPKG_LIB_PATH=$PWD/${BUILD_FOLDER}/vcpkg_installed/${VCPKG_TARGET}/lib
$VCPKG_ROOT/vcpkg install --triplet=${VCPKG_TARGET}

cmake --preset ${PRESET}
cmake --build --preset ${PRESET}
cmake --install ${BUILD_FOLDER} --prefix ${OUTPUT_FOLDER} --component headers

cp "${VCPKG_LIB_PATH}"/*.a "${OUTPUT_LIB_PATH}/"

ARTIFACTS=(
    "${OUTPUT_LIB_PATH}/libDynXX.a"
    "${OUTPUT_LIB_PATH}/libqjs.a"
    "${OUTPUT_LIB_PATH}/libmmkvcore.a"
    "${OUTPUT_LIB_PATH}/libmmkv.a"
)
check_artifacts "${ARTIFACTS[@]}"

final_lib=DynXX.a
merge_libs "${OUTPUT_LIB_PATH}" "${final_lib}" "ar"
check_artifacts "${OUTPUT_LIB_PATH}/${final_lib}"
