import { join } from 'node:path';

import {
  checkArtifacts, copyStaticLibs, exportCompileCommands, getOutputExePath,
  getOutputLibPath, getVcpkgLibPath, gotoParentPath, mergeLibs,
  runCMake, setBuildOutputEnv, setEnv, setupVcpkgEnv,
} from './utils.js';

function main() {
  const root = gotoParentPath();

  const buildType = "Release";
  const platform = "MAC_UNIVERSAL";
  const abi = "arm64";
  const ver = "14.0";

  const platformName = "macOS";
  const preset = `${platformName}-${buildType}`;

  setEnv("APPLE_TOOLCHAIN_FILE", join(root, "cmake", "toolchains", "Apple", "ios.toolchain.cmake"));
  setEnv("APPLE_PLATFORM", platform);
  setEnv("APPLE_ABI", abi);
  setEnv("APPLE_VER", ver);

  const buildFolder = join(`build.${platformName}`, buildType);
  const outputFolder = join(buildFolder, "output");
  const outputPath = join(root, outputFolder, abi)

  setBuildOutputEnv(buildFolder, outputPath);

  setupVcpkgEnv(`${abi}-osx`);

  const vcpkgLibPath = getVcpkgLibPath(root, buildFolder);
  const outputLibPath = getOutputLibPath();
  const outputExePath = getOutputExePath();

  runCMake(preset, buildFolder, outputFolder, true);

  exportCompileCommands(buildFolder, root);

  checkArtifacts([
    join(outputLibPath, "libDynXX.a"),
    join(outputExePath, "qjsc.app", "Contents", "MacOS", "qjsc")
  ]);

  copyStaticLibs(vcpkgLibPath, outputLibPath);

  mergeLibs(outputLibPath, "libDynXX-All.a");
  
  checkArtifacts([join(outputLibPath, "libDynXX-All.a")]);
}

main();
