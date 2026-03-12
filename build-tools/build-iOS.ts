import { join } from 'node:path';

import {
  checkArtifacts, copyStaticLibs, exportCompileCommands, getOutputLibPath,
  getVcpkgLibPath, gotoParentPath, mergeLibs, runCMake,
  setBuildOutputEnv, setEnv, setupVcpkgEnv,
} from './utils.js';

function main() {
  const root = gotoParentPath();

  const buildType = "Release";
  const platform = "OS64";
  const abi = "arm64";
  const ver = "15.0";

  const platformName = "iOS";
  const preset = `${platformName}-${buildType}`;

  setEnv("APPLE_TOOLCHAIN_FILE", join(root, "cmake", "toolchains", "Apple", "ios.toolchain.cmake"));
  setEnv("APPLE_PLATFORM", platform);
  setEnv("APPLE_ABI", abi);
  setEnv("APPLE_VER", ver);

  const buildFolder = join(`build.${platformName}`, buildType);
  const outputFolder = join(buildFolder, "output");
  const outputPath = join(root, outputFolder, abi);

  setBuildOutputEnv(buildFolder, outputPath);

  setupVcpkgEnv(`${abi}-ios`);

  const vcpkgLibPath = getVcpkgLibPath(root, buildFolder);
  const outputLibPath = getOutputLibPath();

  runCMake(preset, buildFolder, outputFolder, true);

  exportCompileCommands(buildFolder, root);

  checkArtifacts([join(outputLibPath, "libDynXX.a")]);

  copyStaticLibs(vcpkgLibPath, outputLibPath);

  mergeLibs(outputLibPath, "libDynXX-All.a");
  
  checkArtifacts([join(outputLibPath, "libDynXX-All.a")]);
}

main();
