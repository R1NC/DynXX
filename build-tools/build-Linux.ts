import { join } from 'node:path';

import {
  checkArtifacts, copyStaticLibs, exportCompileCommands, getOutputLibPath,
  getVcpkgLibPath, gotoParentPath, mergeLibs, runCMake,
  setBuildOutputEnv, setupVcpkgEnv,
} from './utils.js';

function main() {
  const root = gotoParentPath();

  const buildType = "Release";
  const abi = "x64";

  const platformName = "Linux";
  const preset = `${platformName}-${buildType}`;

  const buildFolder = join(`build.${platformName}`, buildType);
  const outputFolder = join(buildFolder, "output");
  const outputPath = join(root, outputFolder, abi);

  setBuildOutputEnv(buildFolder, outputPath);

  setupVcpkgEnv(`${abi}-linux`);

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
