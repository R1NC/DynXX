import { resolve, join } from 'node:path';
import fs from 'node:fs';

import { 
  checkArtifacts, 
  copyStaticLibs, 
  exportCompileCommands, 
  setBuildOutputEnv,
  setupVcpkgEnv,
  getVcpkgLibPath,
  runCMake,
  mergeLibs, 
  gotoParentPath,
} from './utils.js';

function getOhosLlvmRoot(ndkHome: string): string {
  const llvmDir = resolve(ndkHome, 'llvm', 'bin');
  
  if (fs.existsSync(llvmDir) && fs.statSync(llvmDir).isDirectory()) {
    return llvmDir;
  }
  
  throw new Error(`Cannot determine HarmonyOS llvm root (bin) under ${resolve(ndkHome, 'llvm')}`);
}

function main() {
  const root = gotoParentPath();

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

  setBuildOutputEnv(buildFolder, outputPath);

  const home = process.env.HOME || process.env.USERPROFILE || "";
  setupVcpkgEnv("arm64-ohos", home);

  const vcpkgLibPath = getVcpkgLibPath(root, buildFolder);
  const outputLibPath = process.env.OUTPUT_LIB_PATH!;

  runCMake(preset, buildFolder, outputFolder, true);

  exportCompileCommands(buildFolder, root);

  checkArtifacts([join(outputLibPath, "libDynXX.a")]);

  copyStaticLibs(vcpkgLibPath, outputLibPath);

  const llvmRoot = getOhosLlvmRoot(ndkHome);
  const arTool = join(llvmRoot, "llvm-ar");

  mergeLibs(outputLibPath, "libDynXX-All.a", arTool);
  
  checkArtifacts([join(outputLibPath, "libDynXX-All.a")]);
}

main();
