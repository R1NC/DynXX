import { dirname, resolve, join } from 'node:path';
import { fileURLToPath } from 'node:url';

import { 
  checkArtifacts, 
  exportCompileCommands, 
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

  const platformName = "Windows";
  const preset = `${platformName}-${buildType}`;

  const windowsAbi = process.env.WINDOWS_ABI || "x64";

  const home = process.env.USERPROFILE || process.env.HOME || "";
  setupVcpkgEnv(`${windowsAbi}-windows-static`, home);

  const buildFolder = `build.${platformName}/${buildType}`;
  const outputFolder = `${buildFolder}/output`;
  const outputPath = join(root, outputFolder, windowsAbi);

  process.env.BUILD_FOLDER = buildFolder;
  process.env.OUTPUT_LIB_PATH = join(outputPath, "lib");
  process.env.OUTPUT_DLL_PATH = join(outputPath, "share");
  process.env.OUTPUT_EXE_PATH = join(outputPath, "bin");

  runCMake(preset, buildFolder, outputFolder, true);

  exportCompileCommands(buildFolder, root);

  checkArtifacts([
    join(process.env.OUTPUT_LIB_PATH!, "DynXX.lib"),
    join(process.env.OUTPUT_EXE_PATH!, "qjsc.exe")
  ]);
}

main();
