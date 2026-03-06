import { dirname, resolve, join } from 'node:path';
import { fileURLToPath } from 'node:url';

import { 
  checkArtifacts, 
  exportCompileCommands, 
  run 
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

  process.env.BUILD_FOLDER = buildFolder;
  process.env.OUTPUT_LIB_PATH = join(outputPath, "lib");
  process.env.OUTPUT_DLL_PATH = join(outputPath, "share");
  process.env.OUTPUT_EXE_PATH = join(outputPath, "bin");

  const home = process.env.HOME || process.env.USERPROFILE || "";
  const ciVcpkgHome = process.env.CI_VCPKG_HOME;
  
  if (ciVcpkgHome && !process.env.VCPKG_HOME) {
    process.env.VCPKG_HOME = ciVcpkgHome;
  }
  
  process.env.VCPKG_BINARY_SOURCES = process.env.CI_VCPKG_BINARY_SOURCES || 
    `files,${home}/vcpkg-binary-cache,readwrite`;
    
  process.env.VCPKG_TARGET_TRIPLET = process.env.VCPKG_TARGET_TRIPLET || "wasm32-emscripten";

  run("cmake", ["--preset", preset]);
  run("cmake", ["--build", "--preset", preset]);

  exportCompileCommands(buildFolder, root);

  const outputExePath = process.env.OUTPUT_EXE_PATH!;
  
  checkArtifacts([
    join(outputExePath, "DynXX.wasm"),
    join(outputExePath, "DynXX.js"),
    join(outputExePath, "DynXX.html"),
  ]);
}

main();
