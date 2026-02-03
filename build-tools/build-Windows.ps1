Set-Location ..

$env:PLATFORM = "Windows"
$env:BUILD_TYPE = "Release"
$env:WINDOWS_ABI = "x64"
$PRESET = "$env:PLATFORM-$env:BUILD_TYPE"

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
$env:VCPKG_TARGET_TRIPLET = "$env:WINDOWS_ABI-windows-static"

$BUILD_FOLDER = "build.$env:PLATFORM/$env:BUILD_TYPE"
$OUTPUT_FOLDER = "$BUILD_FOLDER/output"
$OUTPUT_PATH = "$PWD/$OUTPUT_FOLDER/$env:WINDOWS_ABI"
$OUTPUT_LIB_PATH = "$OUTPUT_PATH/lib"
$OUTPUT_DLL_PATH = "$OUTPUT_PATH/share"
$OUTPUT_EXE_PATH = "$OUTPUT_PATH/bin"

& cmake --preset "$PRESET"
& cmake --build --preset "$PRESET"
& cmake --install "$BUILD_FOLDER" --prefix "$OUTPUT_FOLDER" --component headers

foreach ($f in @(
    "$OUTPUT_LIB_PATH/DynXX.lib",
    "$OUTPUT_EXE_PATH/qjsc.exe"
)) {
    if (-not (Test-Path $f)) {
        Write-Host "ARTIFACT NOT FOUND: $f"
        exit 1
    } else {
        Write-Host "FOUND: $f"
    }
}
