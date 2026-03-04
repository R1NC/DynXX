import os
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

from build_utils import (
    check_artifacts,
    copy_static_libs,
    export_compile_commands,
    merge_libs,
    run,
)


def ohos_llvm_root(ndk_home: str) -> Path:
    llvm_dir = Path(ndk_home) / "llvm" / "bin"
    if llvm_dir.is_dir():
        return llvm_dir
    raise RuntimeError(f"Cannot determine HarmonyOS llvm root (bin) under {Path(ndk_home) / 'llvm'}")


def main():
    root = Path(__file__).resolve().parent.parent
    os.chdir(root)

    debug = os.environ.get("DEBUG", "0")
    build_type = os.environ.get("BUILD_TYPE", "Release")
    if debug == "1":
        build_type = "Debug"

    platform_name = "HarmonyOS"
    preset = f"{platform_name}-{build_type}"

    ci_ndk_home = os.environ.get("CI_OHOS_NDK_HOME")
    if ci_ndk_home and not os.environ.get("OHOS_NDK_HOME"):
        os.environ["OHOS_NDK_HOME"] = ci_ndk_home
    ndk_home = os.environ["OHOS_NDK_HOME"]

    os.environ["OHOS_ABI"] = os.environ.get("OHOS_ABI", "arm64-v8a")

    build_folder = f"build.{platform_name}/{build_type}"
    output_folder = f"{build_folder}/output"
    output_path = (root / output_folder / os.environ["OHOS_ABI"]).as_posix()

    os.environ["BUILD_FOLDER"] = build_folder
    os.environ["OUTPUT_LIB_PATH"] = f"{output_path}/lib"
    os.environ["OUTPUT_DLL_PATH"] = f"{output_path}/share"
    os.environ["OUTPUT_EXE_PATH"] = f"{output_path}/bin"

    home = Path.home().as_posix()
    ci_vcpkg_home = os.environ.get("CI_VCPKG_HOME")
    if ci_vcpkg_home and not os.environ.get("VCPKG_HOME"):
        os.environ["VCPKG_HOME"] = ci_vcpkg_home
    os.environ["VCPKG_BINARY_SOURCES"] = os.environ.get(
        "CI_VCPKG_BINARY_SOURCES",
        f"files,{home}/vcpkg-binary-cache,readwrite",
    )
    os.environ["VCPKG_TARGET_TRIPLET"] = os.environ.get("VCPKG_TARGET_TRIPLET", "arm64-ohos")

    output_lib_path = Path(os.environ["OUTPUT_LIB_PATH"])

    run(["cmake", "--preset", preset])
    run(["cmake", "--build", "--preset", preset])
    run(["cmake", "--install", build_folder, "--prefix", output_folder, "--component", "headers"])

    export_compile_commands(build_folder, root)

    check_artifacts([f"{output_lib_path}/libDynXX.a"])

    llvm_root = ohos_llvm_root(ndk_home)
    ar_tool = llvm_root / "llvm-ar"
    merge_libs(output_lib_path, "libDynXX-All.a", str(ar_tool))
    check_artifacts([f"{output_lib_path}/libDynXX-All.a"])


if __name__ == "__main__":
    main()
