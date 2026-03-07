import { join } from 'node:path';

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

function main() {
  const root = gotoParentPath();

  const debug = process.env.DEBUG || "0";
  let buildType = process.env.BUILD_TYPE || "Release";
  if (debug === "1") {
    buildType = "Debug";
  }

  const platformName = "iOS";
  const preset = `${platformName}-${buildType}`;

  process.env.APPLE_TOOLCHAIN_FILE = join(root, "cmake/toolchains/Apple/ios.toolchain.cmake");
  process.env.APPLE_PLATFORM = process.env.APPLE_PLATFORM || "OS64";
  process.env.APPLE_ABI = process.env.APPLE_ABI || "arm64";
  process.env.APPLE_VER = process.env.APPLE_VER || "15.0";

  const buildFolder = `build.${platformName}/${buildType}`;
  const outputFolder = `${buildFolder}/output`;
  const outputPath = join(root, outputFolder, process.env.APPLE_ABI!);

  setBuildOutputEnv(buildFolder, outputPath);

  const home = process.env.HOME || process.env.USERPROFILE || "";
  setupVcpkgEnv(`${process.env.APPLE_ABI!}-ios`, home);

  const vcpkgLibPath = getVcpkgLibPath(root, buildFolder);
  const outputLibPath = process.env.OUTPUT_LIB_PATH!;

  runCMake(preset, buildFolder, outputFolder, true);

  exportCompileCommands(buildFolder, root);

  checkArtifacts([join(outputLibPath, "libDynXX.a")]);

  copyStaticLibs(vcpkgLibPath, outputLibPath);

  mergeLibs(outputLibPath, "libDynXX-All.a", "libtool");
  
  checkArtifacts([join(outputLibPath, "libDynXX-All.a")]);
}

main();
