import { spawnSync, execSync } from 'node:child_process';
import { 
  existsSync, copyFileSync, rmSync, readdirSync, statSync, mkdtempSync, chmodSync
} from 'node:fs';
import { dirname, join, resolve } from 'node:path';
import { homedir, tmpdir, platform } from 'node:os';
import { fileURLToPath } from 'node:url';

const IS_WINDOWS = platform() === 'win32';
const IS_APPLE = platform() === 'darwin';
const IS_LINUX = platform() === 'linux';

// --- Platform Detection ---

export function isWindows(): boolean {
  return IS_WINDOWS;
}

export function isMacOS(): boolean {
  return IS_APPLE;
}

export function isLinux(): boolean {
  return IS_LINUX;
}

export function isUnixLike(): boolean {
  return IS_APPLE || IS_LINUX;
}

// --- File Utils ---

export function copyFile(src: string, dst: string): void {
  copyFileSync(src, dst);
}

export function makeExecutable(filePath: string): void {
  chmodSync(filePath, '755');
}


// --- Path & Env Helpers ---

export function isExistingDirectory(path: string): boolean {
  return existsSync(path) && statSync(path).isDirectory();
}

export function goInTmpDir<T>(operation: (tempRoot: string) => T): T {
  const tempRoot = mkdtempSync(join(tmpdir(), "temp_tool_"));
  try {
    return operation(tempRoot);
  } finally {
    if (existsSync(tempRoot)) {
      rmSync(tempRoot, { recursive: true, force: true });
    }
  }
}

export function gotoParentPath(): string {
  const __filename = fileURLToPath(import.meta.url);
  const __dirname = dirname(__filename);
  const rootPath = resolve(__dirname, '..');
  
  if (process.cwd() !== rootPath) {
    process.chdir(rootPath);
  }
  return rootPath;
}

export function getHomeDir(): string {
  return homedir() ?? process.env.HOME ?? process.env.USERPROFILE ?? "";
}

export function getEnv(name: string): string {
  return process.env[name] ?? "";
}

export function setEnv(name: string, value: string): void {
  process.env[name] = value;
}

export function envEnabled(name: string, defaultValue: boolean): boolean {
  const value = process.env[name];
  if (value === undefined || value.length === 0) {
    return defaultValue;
  }
  return ['1', 'true', 'TRUE', 'on', 'ON'].includes(value);
}

export function readCIEnv(ciEnvName: string, localEnvName: string): string {
  const ciValue = getEnv(ciEnvName);
  if (ciValue && !getEnv(localEnvName)) {
    setEnv(localEnvName, ciValue);
  }
  return getEnv(localEnvName);
}

// --- Platform-specified tools ---

export function getMsvcToolsHome(): string {
  const result = spawn("where.exe", ["lib.exe"], { allowFailure: true });
  if (result) {
    const libPath = result.toString().trim().split(/\r?\n/, 1)[0];
    if (libPath && existsSync(libPath)) {
      const toolsHome = dirname(libPath);
      setEnv("MSVC_TOOLS_HOME", toolsHome);
      return toolsHome;
    }
  }
  throw new Error("Cannot determine MSVC_TOOLS_HOME from lib.exe in PATH");
}

export function getAndroidLlvmHome(ndkHome: string): string {
  const platformPaths: Record<string, string[]> = {
    'darwin': ['darwin-arm64', 'darwin-x86_64'],
    'linux': ['linux-x86_64'],
    'win32': ['windows-x86_64']
  };

  const candidates = platformPaths[process.platform];
  if (!candidates) {
    throw new Error(`Unsupported platform for NDK lookup: ${process.platform}`);
  }

  const prebuiltDir = resolve(ndkHome, 'toolchains', 'llvm', 'prebuilt');

  for (const tag of candidates) {
    const candidateDir = resolve(prebuiltDir, tag, 'bin');
    if (isExistingDirectory(candidateDir)) {
      return candidateDir;
    }
  }

  if (isExistingDirectory(prebuiltDir)) {
    for (const entry of readdirSync(prebuiltDir)) {
      const candidateDir = resolve(prebuiltDir, entry, 'bin');
      if (isExistingDirectory(candidateDir)) {
        return candidateDir;
      }
    }
  }

  throw new Error(`Cannot determine NDK llvm root (bin) under ${prebuiltDir}`);
}

export function getOhosLlvmHome(ndkHome: string): string {
  const llvmDir = resolve(ndkHome, 'llvm', 'bin');
  if (isExistingDirectory(llvmDir)) {
    return llvmDir;
  }
  throw new Error(`Cannot determine HarmonyOS llvm root (bin) under ${resolve(ndkHome, 'llvm')}`);
}

function canExecuteCommand(command: string): boolean {
  if (!command) {
    return false;
  }
  const resolver = IS_WINDOWS ? 'where' : 'which';
  const result = spawnSync(resolver, [command], {
    stdio: ['ignore', 'pipe', 'ignore'],
    shell: false,
    windowsHide: true
  });
  return !result.error && result.status === 0;
}

function isExecutableAvailable(commandOrPath: string): boolean {
  if (!commandOrPath) {
    return false;
  }
  const hasPathSeparator = commandOrPath.includes('/') || commandOrPath.includes('\\');
  if (hasPathSeparator) {
    return existsSync(commandOrPath);
  }
  return canExecuteCommand(commandOrPath);
}

export function checkLLVMReady(tools: string[] = ['llvm-profdata', 'llvm-cov']): boolean {
  const missing = tools.filter((tool) => !isExecutableAvailable(tool));
  if (missing.length > 0) {
    console.warn(`[WARN] LLVM tools unavailable: ${missing.join(', ')}`);
    return false;
  }
  return true;
}

// --- Run Tools ---

/**
 * Executes a command string via shell.
 */
export function exec(command: string, cwd?: string): void {
  try {
    execSync(command, {
      cwd,
      stdio: 'inherit',
      env: process.env
    });
  } catch (error) {
    console.error(`[ERROR] Shell command failed: ${command}`);
    throw error;
  }
}

interface ExecOptions {
  cwd?: string;
  tempDir?: string;
  allowFailure?: boolean;
}

/**
 * Executes a command with args array and captures stdout.
 * Support arguments contain spaces/special chars.
 */
export function spawn(
  cmd: string, 
  args: string[], 
  options: ExecOptions = {}
): Buffer | null {
  const { cwd = process.cwd(), tempDir, allowFailure = false } = options;

  const result = spawnSync(cmd, args, {
    cwd,
    stdio: ['ignore', 'pipe', 'pipe'],
    shell: false,
    windowsHide: true
  });

  let errorMsg: string | null = null;
  if (result.error) {
    errorMsg = `Failed to spawn ${cmd}: ${result.error.message}`;
  } else if (result.status !== 0) {
    const stderrMsg = result.stderr ? result.stderr.toString().trim() : 'Unknown error';
    errorMsg = `${cmd} failed (code ${result.status}): ${stderrMsg}`;
  }

  if (tempDir) rmSync(tempDir, { recursive: true, force: true });

  if (errorMsg) {
    console.error(`[ERROR] ${errorMsg}`);
    if (!allowFailure) process.exit(1);
    return null;
  }

  return result.stdout || null;
}
