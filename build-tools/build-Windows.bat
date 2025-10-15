cd ..

if not defined VCPKG_ROOT (
    set "VCPKG_ROOT=C:/Users/rinc/vcpkg"
)

set "PLATFORM=Windows"
set "BUILD_TYPE=Release"
set "WINDOWS_ABI=x64"
set "PRESET=%PLATFORM%-%BUILD_TYPE%"

set "VCPKG_BINARY_SOURCES=default,read"
set "VCPKG_TARGET=%WINDOWS_ABI%-windows-static"
"%VCPKG_ROOT%/vcpkg" --vcpkg-root "%VCPKG_ROOT%" install --triplet="%VCPKG_TARGET%"

set "BUILD_FOLDER=build.%PLATFORM%\%BUILD_TYPE%"
set "OUTPUT_FOLDER=%BUILD_FOLDER%\output"
set "OUTPUT_LIB_PATH=%CD%\%OUTPUT_FOLDER%\%WINDOWS_ABI%\libs"
set "OUTPUT_EXE_PATH=%CD%\%OUTPUT_FOLDER%\%WINDOWS_ABI%\exe"
rd /s /q %BUILD_FOLDER% 2>nul

cmake --preset "%PRESET%"
cmake --build --preset "%PRESET%"
cmake --install . --prefix "%OUTPUT_FOLDER%" --component headers

for %%f in (
    "%OUTPUT_LIB_PATH%\DynXX.lib"
    "%OUTPUT_LIB_PATH%\qjs.lib"
    "%OUTPUT_LIB_PATH%\mmkvcore.lib"
    "%OUTPUT_LIB_PATH%\mmkv.lib"
    "%OUTPUT_EXE_PATH%\qjsc.exe"
) do (
    if not exist "%%~f" (
        echo ARTIFACT NOT FOUND: %%~f
        exit /b 1
    )
)
