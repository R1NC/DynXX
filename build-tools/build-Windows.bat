set VISUAL_STUDIO="Visual Studio 17 2022"
set BUILD_TYPE=Release

set BUILD_DIR=%CD%\..\build.Windows
set OUTPUT_DIR=%BUILD_DIR%\output\
set HEADER_OUTPUT_DIR=%OUTPUT_DIR%\include\

rmdir /s /q %BUILD_DIR% 2>nul
mkdir %BUILD_DIR%
mkdir %OUTPUT_DIR%
cd %BUILD_DIR%

cmake .. ^
    -G %VISUAL_STUDIO% ^
    -A x64 ^
    -DCMAKE_INSTALL_PREFIX=. ^
    -DCMAKE_RUNTIME_OUTPUT_DIRECTORY=%OUTPUT_DIR% ^
    -DCMAKE_LIBRARY_OUTPUT_DIRECTORY=%OUTPUT_DIR% ^
    -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY=%OUTPUT_DIR%

cmake --build . --config %BUILD_TYPE%

mkdir %HEADER_OUTPUT_DIR% 2>nul
xcopy /E /I /Y ..\include %HEADER_OUTPUT_DIR%

set ARTIFACTS=^
    %OUTPUT_DIR%NGenXX.lib ^
    %OUTPUT_DIR%curl.lib ^
    %OUTPUT_DIR%ssl.lib ^
    %OUTPUT_DIR%crypto.lib ^
    %OUTPUT_DIR%lua.lib ^
    %OUTPUT_DIR%qjs.lib ^
    %OUTPUT_DIR%spdlog.lib ^
    %OUTPUT_DIR%sqlite3.lib ^
    %OUTPUT_DIR%uv.lib ^
    %OUTPUT_DIR%cjson.lib ^
    %OUTPUT_DIR%mmkvcore.lib ^
    %OUTPUT_DIR%mmkv.lib

for %%f in (%ARTIFACTS%) do (
    if not exist %%f (
        echo ARTIFACT NOT FOUND: %%f
        exit /b 1
    )
)