import { dirname, resolve, join } from 'node:path';
import { fileURLToPath } from 'node:url';
import os from 'node:os';
import fs from 'node:fs';

import { 
  checkArtifacts, 
  copyStaticLibs, 
  exportCompileCommands, 
  mergeLibs, 
  run 
} from './utils.js';

function getNdkLlvmRoot(ndkHome: string): string {
  const sysName = os.platform();
  let candidates: string[] = [];

  if (sysName === 'darwin') {
    candidates = ['darwin-arm64', 'darwin-x86_64'];
  } else if (sysName === 'linux') {
    candidates = ['linux-x86_64'];
  } else if (sysName === 'win32') {
    candidates = ['windows-x86_64'];
  } else {
    throw new Error(`Unsupported platform for NDK lookup: ${sysName}`);
  }

  const prebuiltDir = resolve(ndkHome, 'toolchains/llvm/prebuilt');

  for (const tag of candidates) {
    const candidateDir = resolve(prebuiltDir, tag, 'bin');
    if (fs.existsSync(candidateDir) && fs.statSync(candidateDir).isDirectory()) {
      return candidateDir;
    }
  }

  if (fs.existsSync(prebuiltDir) && fs.statSync(prebuiltDir).isDirectory()) {
    const entries = fs.readdirSync(prebuiltDir);
    for (const entry of entries) {
      const fullPath = resolve(prebuiltDir, entry);
      if (fs.statSync(fullPath).isDirectory()) {
        const candidateDir = resolve(fullPath, 'bin');
        if (fs.existsSync(candidateDir) && fs.statSync(candidateDir).isDirectory()) {
          return candidateDir;
        }
      }
    }
  }

  throw new Error(`Cannot determine NDK llvm root (bin) under ${prebuiltDir}`);
}

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

  const platformName = "Android";
  const preset = `${platformName}-${buildType}`;

  const ciNdkHome = process.env.CI_ANDROID_NDK_HOME;
  if (ciNdkHome && !process.env.ANDROID_NDK_HOME) {
    process.env.ANDROID_NDK_HOME = ciNdkHome;
  }
  
  const ndkHome = process.env.ANDROID_NDK_HOME;
  if (!ndkHome) {
    throw new Error("ANDROID_NDK_HOME is not set. Please set it or provide CI_ANDROID_NDK_HOME");
  }

  process.env.ANDROID_ABI = process.env.ANDROID_ABI || "arm64-v8a";
  process.env.ANDROID_VER = process.env.ANDROID_VER || "android-24";

  const buildFolder = `build.${platformName}/${buildType}`;
  const outputFolder = `${buildFolder}/output`;
  const outputPath = join(root, outputFolder, process.env.ANDROID_ABI!);

  process.env.BUILD_FOLDER = buildFolder;
  process.env.OUTPUT_LIB_PATH = join(outputPath, "lib");
  process.env.OUTPUT_DLL_PATH = join(outputPath, "share");
  process.env.OUTPUT_EXE_PATH = join(outputPath, "bin");

  const home = os.homedir();
  const ciVcpkgHome = process.env.CI_VCPKG_HOME;
  
  if (ciVcpkgHome && !process.env.VCPKG_HOME) {
    process.env.VCPKG_HOME = ciVcpkgHome;
  }
  
  process.env.VCPKG_BINARY_SOURCES = process.env.CI_VCPKG_BINARY_SOURCES || 
    `files,${home}/vcpkg-binary-cache,readwrite`;
    
  process.env.VCPKG_TARGET_TRIPLET = process.env.VCPKG_TARGET_TRIPLET || "arm64-android";

  const vcpkgLibPath = join(root, buildFolder, "vcpkg_installed", process.env.VCPKG_TARGET_TRIPLET!, "lib");
  const outputLibPath = process.env.OUTPUT_LIB_PATH!;

  run("cmake", ["--preset", preset]);
  run("cmake", ["--build", "--preset", preset]);
  run("cmake", ["--install", buildFolder, "--prefix", outputFolder, "--component", "headers"]);

  exportCompileCommands(buildFolder, root);

  checkArtifacts([join(outputLibPath, "libDynXX.a")]);

  copyStaticLibs(vcpkgLibPath, outputLibPath);

  const llvmRoot = getNdkLlvmRoot(ndkHome);
  const arTool = join(llvmRoot, "llvm-ar");

  mergeLibs(outputLibPath, "libDynXX-All.a", arTool);
  
  checkArtifacts([join(outputLibPath, "libDynXX-All.a")]);
}

main();
