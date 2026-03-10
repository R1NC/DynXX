import { join } from 'node:path';

import { 
  checkArtifacts, 
  exportCompileCommands, 
  setBuildOutputEnv,
  readCIEnv,
  setupVcpkgEnv,
  runCMake,
  gotoParentPath,
  getOutputExePath
} from './utils.js';

function main() {
  const root = gotoParentPath();

  const debug = process.env.DEBUG || "0";
  let buildType = process.env.BUILD_TYPE || "Release";
  if (debug === "1") {
    buildType = "Debug";
  }

  const platformName = "Wasm";
  const preset = `${platformName}-${buildType}`;

  readCIEnv("CI_WASM_SDK_HOME", "WASM_SDK_HOME");

  process.env.WASM_ABI = process.env.WASM_ABI || "arm";

  const buildFolder = `build.${platformName}/${buildType}`;
  const outputFolder = `${buildFolder}/output`;
  const outputPath = join(root, outputFolder, process.env.WASM_ABI!);

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
