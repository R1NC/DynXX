import { join } from 'node:path';

import { 
  checkArtifacts, 
  copyStaticLibs, 
  exportCompileCommands, 
  setBuildOutputEnv,
  setupVcpkgEnv,
  getVcpkgLibPath,
  getOutputLibPath,
  getOutputExePath,
  runCMake,
  mergeLibs, 
  gotoParentPath,
} from './utils.js';

function main() {
  const root = gotoParentPath();

  const buildType = "Release";
  const platform = "MAC_UNIVERSAL";
  const abi = "arm64";
  const ver = "14.0";

  const platformName = "macOS";
  const preset = `${platformName}-${buildType}`;

  process.env.APPLE_TOOLCHAIN_FILE = join(root, "cmake/toolchains/Apple/ios.toolchain.cmake");
  process.env.APPLE_PLATFORM = platform;
  process.env.APPLE_ABI = abi;
  process.env.APPLE_VER = ver;

  const buildFolder = `build.${platformName}/${buildType}`;
  const outputFolder = `${buildFolder}/output`;
  const outputPath = join(root, outputFolder, process.env.APPLE_ABI!);

  setBuildOutputEnv(buildFolder, outputPath);

  setupVcpkgEnv(`${process.env.APPLE_ABI!}-osx`);

  const vcpkgLibPath = getVcpkgLibPath(root, buildFolder);
  const outputLibPath = getOutputLibPath();
  const outputExePath = getOutputExePath();

  runCMake(preset, buildFolder, outputFolder, true);

  exportCompileCommands(buildFolder, root);

  checkArtifacts([
    join(outputLibPath, "libDynXX.a"),
    join(outputExePath, "qjsc.app/Contents/MacOS/qjsc")
  ]);

  copyStaticLibs(vcpkgLibPath, outputLibPath);

  mergeLibs(outputLibPath, "libDynXX-All.a");
  
  checkArtifacts([join(outputLibPath, "libDynXX-All.a")]);
}

main();
