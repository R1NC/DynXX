import { join } from 'node:path';

import {
  checkArtifacts, copyStaticLibs, exportCompileCommands, getOhosLlvmHome,
  getOutputLibPath, getVcpkgLibPath, gotoParentPath, mergeLibs, readCIEnv,
  runCMake, setBuildOutputEnv, setEnv, setupVcpkgEnv,
} from './utils.js';

function main() {
  const root = gotoParentPath();

  const buildType = "Release";
  const abi = "arm64-v8a";
  
  setEnv("OHOS_ABI", abi);

  const platformName = "OHOS";
  const preset = `${platformName}-${buildType}`;

  const ndkHome = readCIEnv("CI_OHOS_NDK_HOME", "OHOS_NDK_HOME");

  const buildFolder = join(`build.${platformName}`, buildType);
  const outputFolder = join(buildFolder, "output");
  const outputPath = join(root, outputFolder, abi);

  setBuildOutputEnv(buildFolder, outputPath);

  setupVcpkgEnv("arm64-ohos");

  const vcpkgLibPath = getVcpkgLibPath(root, buildFolder);
  const outputLibPath = getOutputLibPath();

  runCMake(preset, buildFolder, outputFolder, true);

  exportCompileCommands(buildFolder, root);

  checkArtifacts([join(outputLibPath, "libDynXX.a")]);

  copyStaticLibs(vcpkgLibPath, outputLibPath);

  const arTool = join(getOhosLlvmHome(ndkHome), "llvm-ar");
  mergeLibs(outputLibPath, "libDynXX-All.a", arTool);
  
  checkArtifacts([join(outputLibPath, "libDynXX-All.a")]);
}

main();
