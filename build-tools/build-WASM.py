import os
import platform
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

from build_utils import check_artifacts, export_compile_commands, run, win_local_app_data_dir


def default_emsdk_root():
    sysname = platform.system().lower()
    home = Path.home()
    if sysname == "darwin":
        return home / "dev/emsdk"
    if sysname == "linux":
        return home / "dev/emsdk"
    if sysname == "windows":
        local_app_data = win_local_app_data_dir()
        return local_app_data / "dev/emsdk"
    return home / "dev/emsdk"


def main():
    root = Path(__file__).resolve().parent.parent
    os.chdir(root)

    debug = os.environ.get("DEBUG", "0")
    build_type = os.environ.get("BUILD_TYPE", "Release")
    if debug == "1":
        build_type = "Debug"

    platform_name = "Wasm"
    preset = f"{platform_name}-{build_type}"

    emsdk_root = os.environ.get("EMSDK_ROOT") or str(default_emsdk_root())
    emscripten_root = os.environ.get("CI_EMSCRIPTEN_ROOT") or f"{emsdk_root}/upstream/emscripten"

    os.environ["EMSDK"] = emsdk_root
    os.environ["EMSCRIPTEN"] = emscripten_root
    os.environ["WASM_SDK_ROOT"] = emscripten_root
    os.environ["WASM_ABI"] = os.environ.get("WASM_ABI", "arm")

    build_folder = f"build.{platform_name}/{build_type}"
    output_folder = f"{build_folder}/output"
    output_path = (root / output_folder / os.environ["WASM_ABI"]).as_posix()

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
    os.environ["VCPKG_TARGET_TRIPLET"] = os.environ.get("VCPKG_TARGET_TRIPLET", "wasm32-emscripten")

    run(["cmake", "--preset", preset])
    run(["cmake", "--build", "--preset", preset])

    export_compile_commands(build_folder, root)

    output_exe_path = Path(os.environ["OUTPUT_EXE_PATH"])
    check_artifacts(
        [
            f"{output_exe_path}/DynXX.wasm",
            f"{output_exe_path}/DynXX.js",
            f"{output_exe_path}/DynXX.html",
        ]
    )


if __name__ == "__main__":
    main()
