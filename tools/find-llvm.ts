import { spawnSync } from 'node:child_process';
import { appendFileSync, copyFileSync, existsSync, mkdirSync, rmSync, symlinkSync } from 'node:fs';
import { basename, dirname, join } from 'node:path';
import { tmpdir } from 'node:os';
import { getEnv, setEnv } from './utils.js';

const IS_WINDOWS = process.platform === 'win32';
const TOOL_SUFFIX = IS_WINDOWS ? '.exe' : '';
const LLVM_COV_BIN = `llvm-cov${TOOL_SUFFIX}`;
const LLVM_PROFDATA_BIN = `llvm-profdata${TOOL_SUFFIX}`;

function firstOutputLine(output: Buffer): string {
  return output.toString().split(/\r?\n/).map((line) => line.trim()).find((line) => line.length > 0) ?? '';
}

function resolveCommandPath(command: string): string {
  const resolver = IS_WINDOWS ? 'where' : 'which';
  const result = spawnSync(resolver, [command], {
    stdio: ['ignore', 'pipe', 'ignore'],
    shell: false
  });
  if (result.error || result.status !== 0) {
    return '';
  }
  const path = firstOutputLine(result.stdout);
  return path && existsSync(path) ? path : '';
}

function isValidLlvmHome(home: string): boolean {
  if (!home) {
    return false;
  }
  return existsSync(join(home, LLVM_COV_BIN)) && existsSync(join(home, LLVM_PROFDATA_BIN));
}

function ensureToolLink(sourcePath: string, targetPath: string): void {
  rmSync(targetPath, { force: true });
  if (IS_WINDOWS) {
    copyFileSync(sourcePath, targetPath);
    return;
  }
  try {
    symlinkSync(sourcePath, targetPath);
  } catch {
    copyFileSync(sourcePath, targetPath);
  }
}

function createShimHome(llvmCovPath: string, llvmProfdataPath: string): string {
  const tempRoot = getEnv('RUNNER_TEMP') || tmpdir();
  const shimHome = join(tempRoot, 'dynxx-llvm-tools');
  mkdirSync(shimHome, { recursive: true });
  ensureToolLink(llvmCovPath, join(shimHome, LLVM_COV_BIN));
  ensureToolLink(llvmProfdataPath, join(shimHome, LLVM_PROFDATA_BIN));
  return shimHome;
}

function resolveUnixLlvmHome(): string {
  const versions = ['', '18', '17', '16', '15', '14', '13', '12'];
  for (const version of versions) {
    const suffix = version ? `-${version}` : '';
    const llvmCovPath = resolveCommandPath(`llvm-cov${suffix}`);
    const llvmProfdataPath = resolveCommandPath(`llvm-profdata${suffix}`);
    if (!llvmCovPath || !llvmProfdataPath) {
      continue;
    }
    const llvmHome = dirname(llvmCovPath);
    if (
      basename(llvmCovPath) === LLVM_COV_BIN &&
      basename(llvmProfdataPath) === LLVM_PROFDATA_BIN &&
      dirname(llvmProfdataPath) === llvmHome &&
      isValidLlvmHome(llvmHome)
    ) {
      return llvmHome;
    }
    return createShimHome(llvmCovPath, llvmProfdataPath);
  }
  return '';
}

function resolveWindowsLlvmHome(): string {
  const llvmCovPath = resolveCommandPath('llvm-cov.exe') || resolveCommandPath('llvm-cov');
  const llvmProfdataPath = resolveCommandPath('llvm-profdata.exe') || resolveCommandPath('llvm-profdata');
  if (llvmCovPath && llvmProfdataPath) {
    const llvmHome = dirname(llvmCovPath);
    if (
      basename(llvmCovPath).toLowerCase() === LLVM_COV_BIN.toLowerCase() &&
      basename(llvmProfdataPath).toLowerCase() === LLVM_PROFDATA_BIN.toLowerCase() &&
      dirname(llvmProfdataPath).toLowerCase() === llvmHome.toLowerCase() &&
      isValidLlvmHome(llvmHome)
    ) {
      return llvmHome;
    }
    return createShimHome(llvmCovPath, llvmProfdataPath);
  }
  const vcInstallDir = getEnv('VCINSTALLDIR');
  if (vcInstallDir) {
    const candidate = join(vcInstallDir, 'Tools', 'Llvm', 'x64', 'bin');
    if (isValidLlvmHome(candidate)) {
      return candidate;
    }
  }
  return '';
}

function resolveLlvmHome(): string {
  const envLlvmHome = getEnv('LLVM_HOME');
  if (isValidLlvmHome(envLlvmHome)) {
    return envLlvmHome;
  }
  return IS_WINDOWS ? resolveWindowsLlvmHome() : resolveUnixLlvmHome();
}

function exportLlvmHome(llvmHome: string): void {
  setEnv('LLVM_HOME', llvmHome);
  const githubEnvPath = getEnv('GITHUB_ENV');
  if (githubEnvPath) {
    appendFileSync(githubEnvPath, `LLVM_HOME=${llvmHome}\n`);
  }
  console.log(`LLVM_HOME=${llvmHome}`);
}

function main() {
  const llvmHome = resolveLlvmHome();
  if (!llvmHome) {
    throw new Error('Cannot locate LLVM tools home (llvm-cov / llvm-profdata)');
  }
  exportLlvmHome(llvmHome);
}

main();
