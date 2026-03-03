import os
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path


def run(cmd, cwd=None):
    subprocess.run(cmd, cwd=cwd, check=True)


def export_compile_commands(build_folder, root):
    src = root / build_folder / "compile_commands.json"
    dst = root / "compile_commands.json"
    if dst.exists() or dst.is_symlink():
        dst.unlink()
    if src.exists():
        try:
            os.symlink(str(src), str(dst))
        except OSError:
            shutil.copy2(src, dst)


def check_artifacts(paths):
    missing = []
    for path in paths:
        if Path(path).is_file():
            print(f"FOUND: {path}")
        else:
            missing.append(path)
    if missing:
        for path in missing:
            print(f"ARTIFACT NOT FOUND: {path}")
        sys.exit(1)


def copy_static_libs(src_dir, dest_dir):
    src_dir = Path(src_dir)
    dest_dir = Path(dest_dir)
    if not src_dir.exists():
        return
    for lib_file in src_dir.glob("*.a"):
        shutil.copy2(lib_file, dest_dir)


def merge_libs(lib_dir, output_lib, ar_tool):
    lib_dir = Path(lib_dir)
    a_files = sorted(lib_dir.glob("*.a"))
    if not a_files:
        print(f"ERROR: No static libraries found in {lib_dir}")
        sys.exit(1)

    output_lib_path = lib_dir / output_lib
    ar_name = Path(ar_tool).name

    if "libtool" in ar_name:
        run([ar_tool, "-static", "-o", str(output_lib_path)] + [str(p) for p in a_files], cwd=lib_dir)
    else:
        temp_dir = Path(tempfile.mkdtemp(prefix="temp_merge_", dir=lib_dir))
        try:
            for index, lib in enumerate(a_files):
                extract_dir = temp_dir / f"lib{index}"
                extract_dir.mkdir(parents=True, exist_ok=True)
                run([ar_tool, "x", str(lib)], cwd=extract_dir)
            obj_files = [str(p) for p in temp_dir.rglob("*.o")]
            if not obj_files:
                print("ERROR: No object files found after extraction")
                sys.exit(1)
            run([ar_tool, "rcs", str(output_lib_path)] + obj_files, cwd=lib_dir)
        finally:
            shutil.rmtree(temp_dir, ignore_errors=True)

    if not output_lib_path.is_file():
        print(f"ERROR: Output library {output_lib_path} was not created")
        sys.exit(1)

    size_bytes = output_lib_path.stat().st_size
    print(f"FOUND: {output_lib_path} ({size_bytes} Bytes)")


def win_local_app_data_dir() -> Path:
    home = Path.home()
    return Path(os.environ.get("LOCALAPPDATA", home / "AppData/Local"))
