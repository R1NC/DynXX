import os
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

from build_utils import check_artifacts, copy_static_libs, export_compile_commands, merge_libs, run


def main():
    root = Path(__file__).resolve().parent.parent
    os.chdir(root)

    debug = os.environ.get("DEBUG", "0")
    build_type = os.environ.get("BUILD_TYPE", "Release")
    if debug == "1":
        build_type = "Debug"

    platform_name = "macOS"
    preset = f"{platform_name}-{build_type}"

    os.environ["APPLE_TOOLCHAIN_FILE"] = str(root / "cmake/toolchains/Apple/ios.toolchain.cmake")
    os.environ["APPLE_PLATFORM"] = os.environ.get("APPLE_PLATFORM", "MAC_UNIVERSAL")
    os.environ["APPLE_ABI"] = os.environ.get("APPLE_ABI", "arm64")
    os.environ["APPLE_VER"] = os.environ.get("APPLE_VER", "14.0")

    build_folder = f"build.{platform_name}/{build_type}"
    output_folder = f"{build_folder}/output"
    output_path = (root / output_folder / os.environ["APPLE_ABI"]).as_posix()

    os.environ["BUILD_FOLDER"] = build_folder
    os.environ["OUTPUT_LIB_PATH"] = f"{output_path}/lib"
    os.environ["OUTPUT_DLL_PATH"] = f"{output_path}/share"
    os.environ["OUTPUT_EXE_PATH"] = f"{output_path}/bin"

    home = Path.home().as_posix()
    os.environ["VCPKG_ROOT"] = os.environ.get("CI_VCPKG_ROOT", f"{home}/dev/vcpkg")
    os.environ["VCPKG_BINARY_SOURCES"] = os.environ.get(
        "CI_VCPKG_BINARY_SOURCES",
        f"files,{home}/vcpkg-binary-cache,readwrite",
    )
    os.environ["VCPKG_TARGET_TRIPLET"] = os.environ.get("VCPKG_TARGET_TRIPLET", f"{os.environ['APPLE_ABI']}-osx")

    vcpkg_lib_path = root / build_folder / "vcpkg_installed" / os.environ["VCPKG_TARGET_TRIPLET"] / "lib"
    output_lib_path = Path(os.environ["OUTPUT_LIB_PATH"])
    output_exe_path = Path(os.environ["OUTPUT_EXE_PATH"])

    run(["cmake", "--preset", preset])
    run(["cmake", "--build", "--preset", preset])
    run(["cmake", "--install", build_folder, "--prefix", output_folder, "--component", "headers"])

    export_compile_commands(build_folder, root)

    check_artifacts([f"{output_lib_path}/libDynXX.a", f"{output_exe_path}/qjsc.app/Contents/MacOS/qjsc"])

    copy_static_libs(vcpkg_lib_path, output_lib_path)

    merge_libs(output_lib_path, "libDynXX-All.a", "libtool")
    check_artifacts([f"{output_lib_path}/libDynXX-All.a"])


if __name__ == "__main__":
    main()
