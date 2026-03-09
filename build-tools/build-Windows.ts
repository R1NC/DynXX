import { join } from 'node:path';

import { 
  checkArtifacts, 
  exportCompileCommands, 
  setupVcpkgEnv,
  getOutputLibPath,
  runCMake,
  gotoParentPath,
  setBuildOutputEnv
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

  setupVcpkgEnv(`${windowsAbi}-windows-static`);

  const buildFolder = `build.${platformName}/${buildType}`;
  const outputFolder = `${buildFolder}/output`;
  const outputPath = join(root, outputFolder, windowsAbi);

  setBuildOutputEnv(buildFolder, outputPath);

  runCMake(preset, buildFolder, outputFolder, true);

  exportCompileCommands(buildFolder, root);

  checkArtifacts([
    join(getOutputLibPath(), "DynXX.lib"),
    join(process.env.OUTPUT_EXE_PATH!, "qjsc.exe")
  ]);
}

main();
