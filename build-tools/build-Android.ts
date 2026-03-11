import { resolve, join } from 'node:path';
import os from 'node:os';
import fs from 'node:fs';

import { 
  checkArtifacts, 
  copyStaticLibs, 
  exportCompileCommands, 
  setBuildOutputEnv,
  readCIEnv,
  setupVcpkgEnv,
  getVcpkgLibPath,
  getOutputLibPath,
  runCMake,
  mergeLibs, 
  gotoParentPath,
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
  const root = gotoParentPath();

  const buildType = "Release";
  const abi = "arm64-v8a";
  const api = 24;

  process.env.ANDROID_ABI = abi;
  process.env.ANDROID_VER = `android-${api}`;

  const platformName = "Android";
  const preset = `${platformName}-${buildType}`;

  const ndkHome = readCIEnv("CI_ANDROID_NDK_HOME", "ANDROID_NDK_HOME");

  const buildFolder = `build.${platformName}/${buildType}`;
  const outputFolder = `${buildFolder}/output`;
  const outputPath = join(root, outputFolder, abi);

  setBuildOutputEnv(buildFolder, outputPath);

  setupVcpkgEnv("arm64-android");

  const vcpkgLibPath = getVcpkgLibPath(root, buildFolder);
  const outputLibPath = getOutputLibPath();

  runCMake(preset, buildFolder, outputFolder, true);

  exportCompileCommands(buildFolder, root);

  checkArtifacts([join(outputLibPath, "libDynXX.a")]);

  copyStaticLibs(vcpkgLibPath, outputLibPath);

  const llvmRoot = getNdkLlvmRoot(ndkHome);
  const arTool = join(llvmRoot, "llvm-ar");

  mergeLibs(outputLibPath, "libDynXX-All.a", arTool);
  
  checkArtifacts([join(outputLibPath, "libDynXX-All.a")]);
}

main();
