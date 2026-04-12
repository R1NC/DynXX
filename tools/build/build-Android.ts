import { join, dirname } from 'node:path';

import {
  getAndroidLlvmHome, gotoParentPath, readCIEnv, setEnv, copyFile, makeExecutable, exec,
  isWindows,
} from '../utils.js';
import {
  checkArtifacts, copyStaticLibs, exportCompileCommands, getOutputLibPath, getVcpkgLibPath,
  mergeLibs, resolveBuildType, runCMake, setBuildOutputEnv, setupVcpkgEnv
} from './build-utils.js';
import { shouldBuildTests } from '../test/gtest-utils.js';

function main() {
  const root = gotoParentPath();

  const buildType = resolveBuildType();
  const abi = "arm64-v8a";
  const api = 24;

  setEnv("ANDROID_ABI", abi);
  setEnv("ANDROID_VER", `android-${api}`);

  const platformName = "Android";
  const preset = `${platformName}-${buildType}`;

  const ndkHome = readCIEnv("CI_ANDROID_NDK_HOME", "ANDROID_NDK_HOME");

  const buildFolder = join(`build.${platformName}`, buildType);
  const outputFolder = join(buildFolder, "output");
  const outputPath = join(root, outputFolder, abi);

  setBuildOutputEnv(buildFolder, outputPath);

  setupVcpkgEnv("arm64-android");

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

  const arTool = join(getAndroidLlvmHome(ndkHome), "llvm-ar");
  mergeLibs(outputLibPath, "libDynXX-All.a", arTool);
  
  checkArtifacts([join(outputLibPath, "libDynXX-All.a")]);
  
  const androidPath = join(root, 'platforms', 'Android');
  const gradleBuildType = buildType.toLowerCase();
  
  const gradlewCmd = isWindows() ? String.raw`.\gradlew.bat` : './gradlew';
  if (!isWindows()) {
    makeExecutable(join(androidPath, 'gradlew'));
  }
  exec(`${gradlewCmd} --no-daemon :DynXX-lib:assemble${buildType}`, androidPath);
  exec(`${gradlewCmd} --stop`, androidPath);

  const aarModule = "DynXX-lib";
  const aarName = `${aarModule}-${gradleBuildType}.aar`;
  const aarSrcPath = join(androidPath, aarModule, 'build', 'outputs', 'aar', aarName);
  const aarDstPath = join(dirname(outputPath), aarName);
  copyFile(aarSrcPath, aarDstPath);
  
  checkArtifacts([aarDstPath]);
}

main();
