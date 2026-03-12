import { join } from 'node:path';

import {
  checkArtifacts, copyStaticLibs, exportCompileCommands, getOutputExePath,
  getOutputLibPath, getMsvcToolsHome, getVcpkgLibPath, gotoParentPath,
  mergeLibs, runCMake, setBuildOutputEnv, setupVcpkgEnv,
} from './utils.js';

function main() {
  const root = gotoParentPath();

  const buildType = "Release";
  const abi = "x64";
  
  const platformName = "Windows";

  const preset = `${platformName}-${buildType}`;
  const buildFolder = join(`build.${platformName}`, buildType);
  const outputFolder = join(buildFolder, "output");
  const outputPath = join(root, outputFolder, abi);

  setBuildOutputEnv(buildFolder, outputPath);
  setupVcpkgEnv(`${abi}-windows-static`);

  const vcpkgLibPath = getVcpkgLibPath(root, buildFolder);
  const outputLibPath = getOutputLibPath();

  runCMake(preset, buildFolder, outputFolder, true);
  
  exportCompileCommands(buildFolder, root);

  checkArtifacts([join(outputLibPath, "DynXX.lib")]);
  
  copyStaticLibs(vcpkgLibPath, outputLibPath);

  const libExePath = join(getMsvcToolsHome(), "lib.exe");
  mergeLibs(outputLibPath, "DynXX-All.lib", libExePath);

  checkArtifacts([
    join(outputLibPath, "DynXX-All.lib"),
    join(getOutputExePath(), "qjsc.exe")
  ]);
}

main();
