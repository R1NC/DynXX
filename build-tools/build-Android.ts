import { join, dirname } from 'node:path';

import {
  checkArtifacts, copyStaticLibs, exportCompileCommands, getAndroidLlvmHome,
  getOutputLibPath, getVcpkgLibPath, gotoParentPath, mergeLibs, readCIEnv,
  runCMake, setBuildOutputEnv, setEnv, setupVcpkgEnv, copyFile, makeExecutable, exec,
  isWindows,
} from './utils.js';

function main() {
  const root = gotoParentPath();

  const buildType = "Release";
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

  runCMake(preset, buildFolder, outputFolder, true);

  exportCompileCommands(buildFolder, root);

  checkArtifacts([join(outputLibPath, "libDynXX.a")]);

  copyStaticLibs(vcpkgLibPath, outputLibPath);

  const arTool = join(getAndroidLlvmHome(ndkHome), "llvm-ar");
  mergeLibs(outputLibPath, "libDynXX-All.a", arTool);
  
  checkArtifacts([join(outputLibPath, "libDynXX-All.a")]);
  
  const androidPath = join(root, 'platforms', 'Android');
  const gradleBuildType = buildType.toLowerCase();
  
  if (!isWindows()) {
    makeExecutable(join(androidPath, 'gradlew'));
  }
  
  const gradlewCmd = isWindows() ? '.\\gradlew.bat' : './gradlew';
  exec(`${gradlewCmd} :DynXX-lib:assemble${buildType}`, androidPath);

  const aarSrcPath = join(androidPath, 'DynXX-lib', 'build', 'outputs', 'aar', `DynXX-lib-${gradleBuildType}.aar`);
  const aarDstPath = join(dirname(outputPath), `DynXX-lib-${gradleBuildType}.aar`);
  copyFile(aarSrcPath, aarDstPath);
  
  checkArtifacts([aarDstPath]);
}

main();
