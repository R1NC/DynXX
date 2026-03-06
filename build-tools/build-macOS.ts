import { dirname, resolve, join } from 'node:path';
import { fileURLToPath } from 'node:url';

import { 
  checkArtifacts, 
  copyStaticLibs, 
  exportCompileCommands, 
  mergeLibs, 
  run 
} from './utils.js';

function main() {
  const __filename = fileURLToPath(import.meta.url);
  const __dirname = dirname(__filename);
  const root = resolve(__dirname, '..');

  process.chdir(root);

  const debug = process.env.DEBUG || "0";
  let buildType = process.env.BUILD_TYPE || "Release";
  if (debug === "1") {
    buildType = "Debug";
  }

  const platformName = "macOS";
  const preset = `${platformName}-${buildType}`;

  process.env.APPLE_TOOLCHAIN_FILE = join(root, "cmake/toolchains/Apple/ios.toolchain.cmake");
  process.env.APPLE_PLATFORM = process.env.APPLE_PLATFORM || "MAC_UNIVERSAL";
  process.env.APPLE_ABI = process.env.APPLE_ABI || "arm64";
  process.env.APPLE_VER = process.env.APPLE_VER || "14.0";

  const buildFolder = `build.${platformName}/${buildType}`;
  const outputFolder = `${buildFolder}/output`;
  const outputPath = join(root, outputFolder, process.env.APPLE_ABI!);

  process.env.BUILD_FOLDER = buildFolder;
  process.env.OUTPUT_LIB_PATH = join(outputPath, "lib");
  process.env.OUTPUT_DLL_PATH = join(outputPath, "share");
  process.env.OUTPUT_EXE_PATH = join(outputPath, "bin");

  const home = process.env.HOME || process.env.USERPROFILE || "";
  const ciVcpkgHome = process.env.CI_VCPKG_HOME;
  
  if (ciVcpkgHome && !process.env.VCPKG_HOME) {
    process.env.VCPKG_HOME = ciVcpkgHome;
  }
  
  process.env.VCPKG_BINARY_SOURCES = process.env.CI_VCPKG_BINARY_SOURCES || 
    `files,${home}/vcpkg-binary-cache,readwrite`;
    
  process.env.VCPKG_TARGET_TRIPLET = process.env.VCPKG_TARGET_TRIPLET || 
    `${process.env.APPLE_ABI!}-osx`;

  const vcpkgLibPath = join(root, buildFolder, "vcpkg_installed", process.env.VCPKG_TARGET_TRIPLET!, "lib");
  const outputLibPath = process.env.OUTPUT_LIB_PATH!;
  const outputExePath = process.env.OUTPUT_EXE_PATH!;

  run("cmake", ["--preset", preset]);
  run("cmake", ["--build", "--preset", preset]);
  run("cmake", ["--install", buildFolder, "--prefix", outputFolder, "--component", "headers"]);

  exportCompileCommands(buildFolder, root);

  checkArtifacts([
    join(outputLibPath, "libDynXX.a"),
    join(outputExePath, "qjsc.app/Contents/MacOS/qjsc")
  ]);

  copyStaticLibs(vcpkgLibPath, outputLibPath);

  mergeLibs(outputLibPath, "libDynXX-All.a", "libtool");
  
  checkArtifacts([join(outputLibPath, "libDynXX-All.a")]);
}

main();
