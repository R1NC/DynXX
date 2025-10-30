@echo off

cd ..

if not defined VCPKG_ROOT (
    set "VCPKG_ROOT=%USERPROFILE%/vcpkg"
)

set "PLATFORM=Windows"
set "BUILD_TYPE=Release"
set "WINDOWS_ABI=x64"
set "PRESET=%PLATFORM%-%BUILD_TYPE%"

if not defined VCPKG_BINARY_SOURCES (
    set "VCPKG_BINARY_SOURCES=clear;files,%USERPROFILE%/vcpkg-binary-cache,readwrite"
)
set "VCPKG_TARGET_TRIPLET=%WINDOWS_ABI%-windows-static"

set "BUILD_FOLDER=build.%PLATFORM%\%BUILD_TYPE%"
set "OUTPUT_FOLDER=%BUILD_FOLDER%\output"
set "OUTPUT_PATH=%CD%\%OUTPUT_FOLDER%\%WINDOWS_ABI%"
set "OUTPUT_LIB_PATH=%OUTPUT_PATH%\lib"
set "OUTPUT_DLL_PATH=%OUTPUT_PATH%\share"
set "OUTPUT_EXE_PATH=%OUTPUT_PATH%\bin"

cmake --preset "%PRESET%"
cmake --build --preset "%PRESET%"
cmake --install "%BUILD_FOLDER%" --prefix "%OUTPUT_FOLDER%" --component headers

for %%f in (
    "%OUTPUT_LIB_PATH%\DynXX.lib"
    "%OUTPUT_EXE_PATH%\qjsc.exe"
) do (
    if not exist "%%~f" (
        echo ARTIFACT NOT FOUND: %%~f
        exit /b 1
    ) else (
        echo FOUND: %%~f
    )
)
