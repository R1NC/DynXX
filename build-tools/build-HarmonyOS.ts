import { resolve, join } from 'node:path';
import fs from 'node:fs';

import { 
  checkArtifacts, 
  copyStaticLibs, 
  exportCompileCommands, 
  setBuildOutputEnv,
  readCIEnv,
  setupVcpkgEnv,
  getVcpkgLibPath,
  getOutputLibPath,
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

  const buildType = "Release";
  const abi = "arm64-v8a";
  
  process.env.OHOS_ABI = abi;

  const platformName = "HarmonyOS";
  const preset = `${platformName}-${buildType}`;

  const ndkHome = readCIEnv("CI_OHOS_NDK_HOME", "OHOS_NDK_HOME");

  const buildFolder = `build.${platformName}/${buildType}`;
  const outputFolder = `${buildFolder}/output`;
  const outputPath = join(root, outputFolder, abi);

  setBuildOutputEnv(buildFolder, outputPath);

  setupVcpkgEnv("arm64-ohos");

  const vcpkgLibPath = getVcpkgLibPath(root, buildFolder);
  const outputLibPath = getOutputLibPath();

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
