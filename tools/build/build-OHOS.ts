import { join } from 'node:path';

import {
  getOhosLlvmHome, gotoParentPath, readCIEnv, setEnv,
} from '../utils.js';
import {
  checkArtifacts, copyStaticLibs, exportCompileCommands, getOutputLibPath, getVcpkgLibPath,
  mergeLibs, resolveBuildType, runCMake, setBuildOutputEnv, setupVcpkgEnv
} from './build-utils.js';
import { shouldBuildTests } from '../test/test-utils.js';

function main() {
  const root = gotoParentPath();

  const buildType = resolveBuildType();
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
  const buildTests = shouldBuildTests();

  runCMake(preset, buildFolder, outputFolder, true, [`-DDYNXX_BUILD_TESTS=${buildTests ? "ON" : "OFF"}`]);

  exportCompileCommands(buildFolder, root);

  const buildArtifacts = [join(outputLibPath, "libDynXX.a")];
  if (buildTests) {
    buildArtifacts.push(
      join(outputLibPath, "libDynXXTest.a"),
      join(root, outputFolder, "include", "DynXXTest.hxx"),
    );
  }
  checkArtifacts(buildArtifacts);

  copyStaticLibs(vcpkgLibPath, outputLibPath);

  const arTool = join(getOhosLlvmHome(ndkHome), "llvm-ar");
  mergeLibs(outputLibPath, "libDynXX-All.a", arTool);
  
  checkArtifacts([join(outputLibPath, "libDynXX-All.a")]);
}

main();
