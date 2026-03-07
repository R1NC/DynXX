import { dirname, resolve, join } from 'node:path';
import { fileURLToPath } from 'node:url';

import { 
  checkArtifacts, 
  exportCompileCommands, 
  setBuildOutputEnv,
  setupVcpkgEnv,
  runCMake,
} from './utils.js';

function main() {
  const __filename = fileURLToPath(import.meta.url);
  const __dirname = dirname(__filename);
  const root = resolve(__dirname, '..');

  process.chdir(root);

  const debug = process.env.DEBUG || "0";
  let buildType = process.env.BUILD_TYPE || "Release";
  if (debug === "1") {
    buildType = "Debug";
  }

  const platformName = "Wasm";
  const preset = `${platformName}-${buildType}`;

  let wasmSdkHome = process.env.CI_WASM_SDK_HOME;
  if (!wasmSdkHome) {
    wasmSdkHome = process.env.WASM_SDK_HOME;
  }
  if (!wasmSdkHome) {
    throw new Error("WASM_SDK_HOME is not set. Please set CI_WASM_SDK_HOME or WASM_SDK_HOME environment variable.");
  }

  process.env.WASM_SDK_HOME = wasmSdkHome;
  process.env.WASM_ABI = process.env.WASM_ABI || "arm";

  const buildFolder = `build.${platformName}/${buildType}`;
  const outputFolder = `${buildFolder}/output`;
  const outputPath = join(root, outputFolder, process.env.WASM_ABI!);

  setBuildOutputEnv(buildFolder, outputPath);

  const home = process.env.HOME || process.env.USERPROFILE || "";
  setupVcpkgEnv("wasm32-emscripten", home);

  runCMake(preset, buildFolder, outputFolder, false);

  exportCompileCommands(buildFolder, root);

  const outputExePath = process.env.OUTPUT_EXE_PATH!;
  
  checkArtifacts([
    join(outputExePath, "DynXX.wasm"),
    join(outputExePath, "DynXX.js"),
    join(outputExePath, "DynXX.html"),
  ]);
}

main();
