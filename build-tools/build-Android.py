import os
import platform
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

from build_utils import check_artifacts, copy_static_libs, export_compile_commands, merge_libs, run, win_local_app_data_dir


def default_ndk_root():
    sysname = platform.system().lower()
    home = Path.home()
    ver = "29.0.14206865"
    if sysname == "darwin":
        return home / "Library/Android/sdk/ndk/" + ver
    if sysname == "linux":
        return home / "Android/Sdk/ndk/" + ver
    if sysname == "windows":
        local_app_data = win_local_app_data_dir()
        return local_app_data / "Android/sdk/ndk/" + ver
    return home / "Android/sdk/ndk/" + ver


def ndk_llvm_root(ndk_root: str) -> Path:
    sysname = platform.system().lower()
    candidates = []
    if sysname == "darwin":
        candidates = ["darwin-arm64", "darwin-x86_64"]
    elif sysname == "linux":
        candidates = ["linux-x86_64"]
    elif sysname == "windows":
        candidates = ["windows-x86_64"]

    prebuilt_dir = Path(ndk_root) / "toolchains/llvm/prebuilt"
    for tag in candidates:
        candidate_dir = prebuilt_dir / tag / "bin"
        if candidate_dir.is_dir():
            return candidate_dir

    if prebuilt_dir.is_dir():
        for entry in prebuilt_dir.iterdir():
            if entry.is_dir():
                candidate_dir = entry / "bin"
                if candidate_dir.is_dir():
                    return candidate_dir

    raise RuntimeError(f"Cannot determine NDK llvm root (bin) under {prebuilt_dir}")


def main():
    root = Path(__file__).resolve().parent.parent
    os.chdir(root)

    debug = os.environ.get("DEBUG", "0")
    build_type = os.environ.get("BUILD_TYPE", "Release")
    if debug == "1":
        build_type = "Debug"

    platform_name = "Android"
    preset = f"{platform_name}-{build_type}"

    ndk_root = (
        os.environ.get("CI_NDK_ROOT")
        or os.environ.get("ANDROID_NDK_HOME")
        or os.environ.get("ANDROID_NDK")
        or str(default_ndk_root())
    )

    os.environ["ANDROID_NDK"] = ndk_root
    os.environ["ANDROID_NDK_HOME"] = ndk_root
    os.environ["ANDROID_ABI"] = os.environ.get("ANDROID_ABI", "arm64-v8a")
    os.environ["ANDROID_VER"] = os.environ.get("ANDROID_VER", "android-24")

    build_folder = f"build.{platform_name}/{build_type}"
    output_folder = f"{build_folder}/output"
    output_path = (root / output_folder / os.environ["ANDROID_ABI"]).as_posix()

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
    os.environ["VCPKG_TARGET_TRIPLET"] = os.environ.get("VCPKG_TARGET_TRIPLET", "arm64-android")

    vcpkg_lib_path = root / build_folder / "vcpkg_installed" / os.environ["VCPKG_TARGET_TRIPLET"] / "lib"
    output_lib_path = Path(os.environ["OUTPUT_LIB_PATH"])

    run(["cmake", "--preset", preset])
    run(["cmake", "--build", "--preset", preset])
    run(["cmake", "--install", build_folder, "--prefix", output_folder, "--component", "headers"])

    export_compile_commands(build_folder, root)

    check_artifacts([f"{output_lib_path}/libDynXX.a"])

    copy_static_libs(vcpkg_lib_path, output_lib_path)

    llvm_root = ndk_llvm_root(ndk_root)
    ar_tool = llvm_root / "llvm-ar"
    merge_libs(output_lib_path, "libDynXX-All.a", str(ar_tool))
    check_artifacts([f"{output_lib_path}/libDynXX-All.a"])


if __name__ == "__main__":
    main()
