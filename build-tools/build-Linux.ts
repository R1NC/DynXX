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

  const platformName = "Linux";
  const preset = `${platformName}-${buildType}`;

  const linuxAbi = process.env.LINUX_ABI || "x64";

  const buildFolder = `build.${platformName}/${buildType}`;
  const outputFolder = `${buildFolder}/output`;
  const outputPath = join(root, outputFolder, linuxAbi);

  setBuildOutputEnv(buildFolder, outputPath);

  const home = process.env.HOME || process.env.USERPROFILE || "";
  setupVcpkgEnv(`${linuxAbi}-linux`, home);

  const vcpkgLibPath = getVcpkgLibPath(root, buildFolder);
  const outputLibPath = process.env.OUTPUT_LIB_PATH!;

  runCMake(preset, buildFolder, outputFolder, true);

  exportCompileCommands(buildFolder, root);

  checkArtifacts([join(outputLibPath, "libDynXX.a")]);

  copyStaticLibs(vcpkgLibPath, outputLibPath);

  mergeLibs(outputLibPath, "libDynXX-All.a", "ar");
  
  checkArtifacts([join(outputLibPath, "libDynXX-All.a")]);
}

main();
