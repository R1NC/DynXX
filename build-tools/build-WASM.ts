import { join } from 'node:path';

import {
  checkArtifacts, exportCompileCommands, getOutputExePath, gotoParentPath,
  readCIEnv, runCMake, setBuildOutputEnv, setEnv, setupVcpkgEnv,
} from './utils.js';

function main() {
  const root = gotoParentPath();

  const buildType = "Release";
  const abi = "arm";

  const platformName = "Wasm";
  const preset = `${platformName}-${buildType}`;

  readCIEnv("CI_WASM_SDK_HOME", "WASM_SDK_HOME");

  setEnv("WASM_ABI", abi);

  const buildFolder = join(`build.${platformName}`, buildType);
  const outputFolder = join(buildFolder, "output");
  const outputPath = join(root, outputFolder, abi);

  setBuildOutputEnv(buildFolder, outputPath);

  setupVcpkgEnv("wasm32-emscripten");

  runCMake(preset, buildFolder, outputFolder, false);

  exportCompileCommands(buildFolder, root);

  const outputExePath = getOutputExePath();
  
  checkArtifacts([
    join(outputExePath, "DynXX.wasm"),
    join(outputExePath, "DynXX.js"),
    join(outputExePath, "DynXX.html"),
  ]);
}

main();
