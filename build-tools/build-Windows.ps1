Set-Location ..

$PLATFORM = "Windows"
$BUILD_TYPE = "Release"
$WINDOWS_ABI = "x64"
$PRESET = "$PLATFORM-$BUILD_TYPE"

if ($env:CI_VCPKG_ROOT) {
    $env:VCPKG_ROOT = $env:CI_VCPKG_ROOT
} else {
    $env:VCPKG_ROOT = "$env:USERPROFILE/vcpkg"
}
if ($env:CI_VCPKG_BINARY_SOURCES) {
    $env:VCPKG_BINARY_SOURCES = $env:CI_VCPKG_BINARY_SOURCES
} else {
    $env:VCPKG_BINARY_SOURCES = "files,$env:USERPROFILE/vcpkg-binary-cache,readwrite"
}
$env:VCPKG_TARGET_TRIPLET = "$WINDOWS_ABI-windows-static"

$env:BUILD_FOLDER = "build.$PLATFORM/$BUILD_TYPE"
$OUTPUT_FOLDER = "$env:BUILD_FOLDER/output"
$OUTPUT_PATH = "$PWD/$OUTPUT_FOLDER/$WINDOWS_ABI"
$env:OUTPUT_LIB_PATH = "$OUTPUT_PATH/lib"
$env:OUTPUT_DLL_PATH = "$OUTPUT_PATH/share"
$env:OUTPUT_EXE_PATH = "$OUTPUT_PATH/bin"

& cmake --preset "$PRESET"
& cmake --build --preset "$PRESET"
& cmake --install "$env:BUILD_FOLDER" --prefix "$OUTPUT_FOLDER" --component headers

foreach ($f in @(
    "$env:OUTPUT_LIB_PATH/DynXX.lib",
    "$env:OUTPUT_EXE_PATH/qjsc.exe"
)) {
    if (-not (Test-Path $f)) {
        Write-Host "ARTIFACT NOT FOUND: $f"
        exit 1
    } else {
        Write-Host "FOUND: $f"
    }
}
