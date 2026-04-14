import { join } from 'node:path';

import {
  gotoParentPath, readCIEnv, setEnv,
} from '../utils.js';
import {
  checkArtifacts, exportCompileCommands, getOutputExePath,
  resolveBuildType, runCMake, setBuildOutputEnv, setupVcpkgEnv, shouldConfigureOnly
} from './build-utils.js';

function main() {
  const root = gotoParentPath();

  const buildType = resolveBuildType();
  const abi = "arm";

  const platformName = "WASM";
  const preset = `${platformName}-${buildType}`;

  readCIEnv("CI_WASM_SDK_HOME", "WASM_SDK_HOME");

  setEnv("WASM_ABI", abi);

  const buildFolder = join(`build.${platformName}`, buildType);
  const outputFolder = join(buildFolder, "output");
  const outputPath = join(root, outputFolder, abi);

  setBuildOutputEnv(buildFolder, outputPath);
  const configureOnly = shouldConfigureOnly();

  setupVcpkgEnv("wasm32-emscripten");
  runCMake(preset, buildFolder, outputFolder, true, ["-DDYNXX_BUILD_TESTS=OFF"]);

  exportCompileCommands(buildFolder, root);
  if (configureOnly) {
    return;
  }

  const outputExePath = getOutputExePath();
  
  const buildArtifacts = [
    join(outputExePath, "DynXX.wasm"),
    join(outputExePath, "DynXX.js"),
    join(outputExePath, "DynXX.html"),
  ];
  checkArtifacts(buildArtifacts);
}

main();
