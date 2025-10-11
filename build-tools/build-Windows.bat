if not defined VCPKG_ROOT (
    set "VCPKG_ROOT=C:/Users/rinc/vcpkg"
)

set "VCPKG_TARGET=x64-windows-static"
set "BUILD_TYPE=Release"

cd ..
set "VCPKG_BINARY_SOURCES=clear;default,readwrite"
%VCPKG_ROOT%/vcpkg install --triplet=%VCPKG_TARGET%

set "BUILD_DIR=%CD%\build.Windows"
set "OUTPUT_DIR=%BUILD_DIR%\output"
set "HEADER_OUTPUT_DIR=%OUTPUT_DIR%\include"
set "LIB_OUTPUT_DIR=%OUTPUT_DIR%\libs\%BUILD_TYPE%"

rd /s /q %BUILD_DIR% 2>nul
md "%HEADER_OUTPUT_DIR%" 2>nul
md "%LIB_OUTPUT_DIR%" 2>nul
cd "%BUILD_DIR%"

if /i "%BUILD_TYPE%"=="Debug" (
    set "GENERATOR=Visual Studio 17 2022"
    set "CMAKE_CONFIG_C_COMPILER="
    set "CMAKE_CONFIG_CXX_COMPILER="
    set "CMAKE_CONFIG_EXTRA=-T ClangCL"
) else (
    set "GENERATOR=Ninja"
    set "CMAKE_CONFIG_C_COMPILER=-DCMAKE_C_COMPILER=clang-cl"
    set "CMAKE_CONFIG_CXX_COMPILER=-DCMAKE_CXX_COMPILER=clang-cl"
    set "CMAKE_CONFIG_EXTRA="
)

cmake .. ^
    -G "%GENERATOR%" ^
    -DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake" ^
    -DVCPKG_TARGET_TRIPLET="%VCPKG_TARGET%" ^
    %CMAKE_CONFIG_C_COMPILER% ^
    %CMAKE_CONFIG_CXX_COMPILER% ^
    -DCMAKE_BUILD_TYPE="%BUILD_TYPE%" ^
    -DCMAKE_INSTALL_PREFIX=. ^
    -DCMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE="%LIB_OUTPUT_DIR%" ^
    -DCMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE="%LIB_OUTPUT_DIR%" ^
    -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE="%LIB_OUTPUT_DIR%" ^
    %CMAKE_CONFIG_EXTRA%

cmake --build . --config "%BUILD_TYPE%"
cmake --install . --prefix "%OUTPUT_DIR%" --component headers

for %%f in (
    "%LIB_OUTPUT_DIR%\DynXX.lib"
    "%LIB_OUTPUT_DIR%\qjs.lib"
    "%LIB_OUTPUT_DIR%\mmkvcore.lib"
    "%LIB_OUTPUT_DIR%\mmkv.lib"
    "%LIB_OUTPUT_DIR%\qjsc.exe"
) do (
    if not exist "%%~f" (
        echo ARTIFACT NOT FOUND: %%~f
        exit /b 1
    )
)
