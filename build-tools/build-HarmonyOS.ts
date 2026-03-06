import { dirname, resolve, join } from 'node:path';
import { fileURLToPath } from 'node:url';
import fs from 'node:fs';

import { 
  checkArtifacts, 
  copyStaticLibs, 
  exportCompileCommands, 
  mergeLibs, 
  run 
} from './utils.js';

function getOhosLlvmRoot(ndkHome: string): string {
  const llvmDir = resolve(ndkHome, 'llvm', 'bin');
  
  if (fs.existsSync(llvmDir) && fs.statSync(llvmDir).isDirectory()) {
    return llvmDir;
  }
  
  throw new Error(`Cannot determine HarmonyOS llvm root (bin) under ${resolve(ndkHome, 'llvm')}`);
}

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

  const platformName = "HarmonyOS";
  const preset = `${platformName}-${buildType}`;

  const ciNdkHome = process.env.CI_OHOS_NDK_HOME;
  if (ciNdkHome && !process.env.OHOS_NDK_HOME) {
    process.env.OHOS_NDK_HOME = ciNdkHome;
  }
  
  const ndkHome = process.env.OHOS_NDK_HOME;
  if (!ndkHome) {
    throw new Error("OHOS_NDK_HOME is not set. Please set it or provide CI_OHOS_NDK_HOME");
  }

  process.env.OHOS_ABI = process.env.OHOS_ABI || "arm64-v8a";

  const buildFolder = `build.${platformName}/${buildType}`;
  const outputFolder = `${buildFolder}/output`;
  const outputPath = join(root, outputFolder, process.env.OHOS_ABI!);

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
    
  process.env.VCPKG_TARGET_TRIPLET = process.env.VCPKG_TARGET_TRIPLET || "arm64-ohos";

  const vcpkgLibPath = join(root, buildFolder, "vcpkg_installed", process.env.VCPKG_TARGET_TRIPLET!, "lib");
  const outputLibPath = process.env.OUTPUT_LIB_PATH!;

  run("cmake", ["--preset", preset]);
  run("cmake", ["--build", "--preset", preset]);
  run("cmake", ["--install", buildFolder, "--prefix", outputFolder, "--component", "headers"]);

  exportCompileCommands(buildFolder, root);

  checkArtifacts([join(outputLibPath, "libDynXX.a")]);

  copyStaticLibs(vcpkgLibPath, outputLibPath);

  const llvmRoot = getOhosLlvmRoot(ndkHome);
  const arTool = join(llvmRoot, "llvm-ar");

  mergeLibs(outputLibPath, "libDynXX-All.a", arTool);
  
  checkArtifacts([join(outputLibPath, "libDynXX-All.a")]);
}

main();
