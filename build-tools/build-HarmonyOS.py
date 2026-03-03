import os
import platform
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

from build_utils import check_artifacts, export_compile_commands, merge_libs, run, win_local_app_data_dir


def default_ohos_root():
    sysname = platform.system().lower()
    home = Path.home()
    ver = "20"
    if sysname == "darwin":
        return home / "Library/OpenHarmony/Sdk/" + ver + "/native"
    if sysname == "linux":
        return home / "OpenHarmony/Sdk/" + ver + "/native"
    if sysname == "windows":
        local_app_data = win_local_app_data_dir()
        return local_app_data / "OpenHarmony/Sdk/" + ver + "/native"
    return home / "OpenHarmony/Sdk/" + ver + "/native"


def ohos_llvm_root(ohos_root: str) -> Path:
    llvm_dir = Path(ohos_root) / "llvm" / "bin"
    if llvm_dir.is_dir():
        return llvm_dir
    raise RuntimeError(f"Cannot determine HarmonyOS llvm root (bin) under {Path(ohos_root) / 'llvm'}")


def main():
    root = Path(__file__).resolve().parent.parent
    os.chdir(root)

    debug = os.environ.get("DEBUG", "0")
    build_type = os.environ.get("BUILD_TYPE", "Release")
    if debug == "1":
        build_type = "Debug"

    platform_name = "HarmonyOS"
    preset = f"{platform_name}-{build_type}"

    ohos_root = os.environ.get("CI_OHOS_ROOT") or os.environ.get("OHOS_ROOT") or str(default_ohos_root())
    os.environ["OHOS_ROOT"] = ohos_root
    os.environ["OHOS_ABI"] = os.environ.get("OHOS_ABI", "arm64-v8a")

    build_folder = f"build.{platform_name}/{build_type}"
    output_folder = f"{build_folder}/output"
    output_path = (root / output_folder / os.environ["OHOS_ABI"]).as_posix()

    os.environ["BUILD_FOLDER"] = build_folder
    os.environ["OUTPUT_LIB_PATH"] = f"{output_path}/lib"
    os.environ["OUTPUT_DLL_PATH"] = f"{output_path}/share"
    os.environ["OUTPUT_EXE_PATH"] = f"{output_path}/bin"

    run(["cmake", "--preset", preset])
    run(["cmake", "--build", "--preset", preset])
    run(["cmake", "--install", build_folder, "--prefix", output_folder, "--component", "headers"])

    export_compile_commands(build_folder, root)

    output_lib_path = Path(os.environ["OUTPUT_LIB_PATH"])
    check_artifacts([f"{output_lib_path}/libDynXX.a"])

    llvm_root = ohos_llvm_root(ohos_root)
    ar_tool = llvm_root / "llvm-ar"
    merge_libs(output_lib_path, "libDynXX-All.a", str(ar_tool))
    check_artifacts([f"{output_lib_path}/libDynXX-All.a"])


if __name__ == "__main__":
    main()
