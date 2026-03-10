import { join } from 'node:path';

import { 
  checkArtifacts, 
  exportCompileCommands, 
  setupVcpkgEnv,
  runCMake,
  gotoParentPath,
  setBuildOutputEnv,
  getOutputLibPath,
  getOutputExePath,
  copyStaticLibs,
  mergeLibs,
  getVcpkgLibPath,
  readCIEnv
} from './utils.js';

function main() {
  const root = gotoParentPath();

  const debug = process.env.DEBUG || "0";
  let buildType = process.env.BUILD_TYPE || "Release";
  if (debug === "1") {
    buildType = "Debug";
  }

  const platformName = "Windows";
  const preset = `${platformName}-${buildType}`;
  const windowsAbi = process.env.WINDOWS_ABI || "x64";

  const buildFolder = `build.${platformName}/${buildType}`;
  const outputFolder = `${buildFolder}/output`;
  const outputPath = join(root, outputFolder, windowsAbi);

  setBuildOutputEnv(buildFolder, outputPath);

  setupVcpkgEnv(`${windowsAbi}-windows-static`);

  const vcpkgLibPath = getVcpkgLibPath(root, buildFolder);
  const outputLibPath = getOutputLibPath();

  runCMake(preset, buildFolder, outputFolder, true);

  exportCompileCommands(buildFolder, root);

  checkArtifacts([join(outputLibPath, "DynXX.lib")]);

  copyStaticLibs(vcpkgLibPath, outputLibPath);

  const msvcToolsHome = readCIEnv("CI_MSVC_TOOLS_HOME", "MSVC_TOOLS_HOME");
  libExePath = join(msvcToolsHome, "lib.exe");

  mergeLibs(outputLibPath, "DynXX-All.lib", libExePath);

  checkArtifacts([
    join(outputLibPath, "DynXX-All.lib"),
    join(getOutputExePath(), "qjsc.exe")
  ]);
}

main();
