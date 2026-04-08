import { spawnSync, execSync } from 'node:child_process';
import { 
  existsSync, unlinkSync, symlinkSync, copyFileSync, writeFileSync,
  mkdirSync, rmSync, readdirSync, statSync, mkdtempSync, readlinkSync, chmodSync
} from 'node:fs';
import { dirname, join, resolve, basename, extname, isAbsolute, relative } from 'node:path';
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

export function readCIEnv(ciEnvName: string, localEnvName: string): string {
  const ciValue = getEnv(ciEnvName);
  if (ciValue && !getEnv(localEnvName)) {
    setEnv(localEnvName, ciValue);
  }
  return getEnv(localEnvName);
}

export function setBuildOutputEnv(buildFolder: string, outputPath: string) {
  setEnv("BUILD_FOLDER", buildFolder);
  setEnv("OUTPUT_LIB_PATH", join(outputPath, "lib"));
  setEnv("OUTPUT_DLL_PATH", join(outputPath, "share"));
  setEnv("OUTPUT_EXE_PATH", join(outputPath, "bin"));
}

export const getOutputLibPath = (): string => getEnv("OUTPUT_LIB_PATH");
export const getOutputDllPath = (): string => getEnv("OUTPUT_DLL_PATH");
export const getOutputExePath = (): string => getEnv("OUTPUT_EXE_PATH");

// --- vcpkg setup ---

export function setupVcpkgEnv(triplet: string) {
  readCIEnv("CI_VCPKG_HOME", "VCPKG_HOME");
  const home = getHomeDir();
  let cacheDir = getEnv("VCPKG_DEFAULT_BINARY_CACHE");
  if (!cacheDir) {
    cacheDir = join(home, "vcpkg-binary-cache");
  }
  setEnv("VCPKG_BINARY_SOURCES", `files,${cacheDir},readwrite`);
  if (!getEnv("VCPKG_TARGET_TRIPLET")) {
    setEnv("VCPKG_TARGET_TRIPLET", triplet);
  }
}

export function getVcpkgLibPath(root: string, buildFolder: string): string {
  const triplet = getEnv("VCPKG_TARGET_TRIPLET");
  return join(root, buildFolder, "vcpkg_installed", triplet, "lib");
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

// --- CMake Operations ---

export function exportCompileCommands(buildFolder: string, root: string) {
  const src = resolve(root, buildFolder, "compile_commands.json");
  const dst = resolve(root, "compile_commands.json");

  if (existsSync(dst)) {
    try {
      if (statSync(dst).isSymbolicLink()) {
        const target = readlinkSync(dst);
        if (resolve(target) === src) return;
      }
      unlinkSync(dst);
    } catch (error) {
      console.error(`Failed to process existing file: ${dst}`, error);
      try {
        unlinkSync(dst);
      } catch (unlinkError) {
        console.error(`Failed to force unlink: ${dst}`, unlinkError);
      }
    }
  }

  if (existsSync(src)) {
    try {
      const relativeSrc = relative(dirname(dst), src);
      symlinkSync(relativeSrc, dst, 'file');
      console.log(`Created symlink: ${dst} -> ${relativeSrc}`);
    } catch (error: any) {
      console.warn(`Failed to create symlink, falling back to copy: ${error.message}`);
      copyFileSync(src, dst);
    }
  }
}

export function runCMake(preset: string, buildFolder: string, outputFolder: string, needInstall: boolean) {
  exec(`cmake --preset ${preset}`);
  exec(`cmake --build --preset ${preset}`);
  
  if (needInstall) {
    exec(`cmake --install ${buildFolder} --prefix ${outputFolder} --component headers`);
  }
}

// --- Handle Artifacts ---

export function checkArtifacts(paths: string[]) {
  const missing: string[] = [];
  for (const p of paths) {
    const path = resolve(p);
    if (existsSync(path) && statSync(path).isFile()) {
      console.log(`FOUND: ${path}`);
    } else {
      missing.push(path);
    }
  }
  if (missing.length > 0) {
    missing.forEach(path => console.log(`ARTIFACT NOT FOUND: ${path}`));
    process.exit(1);
  }
}

export function copyStaticLibs(srcDir: string, destDir: string) {
  const srcPath = resolve(srcDir);
  const destPath = resolve(destDir);

  if (!existsSync(srcPath)) return;
  if (!existsSync(destPath)) mkdirSync(destPath, { recursive: true });

  readdirSync(srcPath).forEach(file => {
    const lowerFile = file.toLowerCase();
    if (lowerFile.endsWith(".a") || lowerFile.endsWith(".lib")) {
      copyFileSync(join(srcPath, file), join(destPath, file));
    }
  });
}

// --- Merge Libs ---

function collectObjectFiles(extractDirs: { path: string }[]): string[] {
  const allEntries = extractDirs.flatMap((dirInfo) =>
    readdirSync(dirInfo.path, { recursive: true })
      .map((entry) => join(dirInfo.path, entry.toString()))
  );

  const oFiles = allEntries.filter((file) => file.toLowerCase().endsWith('.o'));
  const objFiles = allEntries.filter((file) => file.toLowerCase().endsWith('.obj'));
  const finalFiles = oFiles.length > 0 ? oFiles : objFiles;

  if (finalFiles.length > 0) {
    const ext = oFiles.length > 0 ? '.o' : '.obj';
    console.log(`[MergeLibs] Detected object format: ${ext} (Count: ${finalFiles.length})`);
  }

  return finalFiles;
}

function mergeLibsWithGenericAr(arTool: string, libDirPath: string, aFiles: string[], outputPath: string) {
  goInTmpDir((tempRoot) => {
    const extractDirs: { path: string }[] = [];
    
    for (const lib of aFiles) {
      const extractDir = join(tempRoot, `extract_${basename(lib, extname(lib))}`);
      mkdirSync(extractDir, { recursive: true });
      extractDirs.push({ path: extractDir });

      spawn(arTool, ["x", join(libDirPath, lib)], { cwd: extractDir });
    }

    const objFiles = collectObjectFiles(extractDirs);
    if (objFiles.length === 0) {
      console.error("ERROR: No object files (.o or .obj) found after extraction");
      process.exit(1);
    }

    const rspPath = join(tempRoot, "ar-merge.rsp");
    writeFileSync(rspPath, objFiles.map(f => `"${f}"`).join("\n"), "utf8");
    spawn(arTool, ["rcs", outputPath, `@${rspPath}`]);
  });
}

function mergeLibsWithWindowsLibExe(toolPath: string, libDirPath: string, libFiles: string[], outputPath: string) {
  if (!existsSync(toolPath)) {
    throw new Error(`Specified lib.exe not found at: ${toolPath}`);
  }

  goInTmpDir((tempRoot) => {
    const extractDirs: { path: string }[] = [];

    for (const lib of libFiles) {
      const libPath = join(libDirPath, lib);
      const extractDir = join(tempRoot, `extract_${basename(lib, '.lib')}`);
      mkdirSync(extractDir, { recursive: true });
      extractDirs.push({ path: extractDir });

      const listOutput = spawn(toolPath, ["/LIST", libPath], { cwd: extractDir, allowFailure: false });
      if (!listOutput) process.exit(1);

      const members = listOutput.toString()
        .split(/\r?\n/)
        .map(line => line.trim())
        .filter(line => line.length > 0);

      for (const member of members) {
        spawn(toolPath, [`/EXTRACT:${member}`, libPath], { cwd: extractDir });
      }
    }

    const objFiles = collectObjectFiles(extractDirs);
    if (objFiles.length === 0) {
      console.error("ERROR: No object files found after extraction from .lib files");
      process.exit(1);
    }

    const rspPath = join(tempRoot, "merge-lib.rsp");
    const rspContent = [`/OUT:"${outputPath}"`, ...objFiles.map(f => `"${f}"`)].join("\n");
    writeFileSync(rspPath, rspContent, "utf8");
    
    spawn(toolPath, [`@${rspPath}`], { cwd: tempRoot });
  });
}

function mergeLibsWithAppleLibTool(libDirPath: string, aFiles: string[], outputPath: string) {
  const args = ["-static", "-o", outputPath, ...aFiles.map(lib => join(libDirPath, lib))];
  spawn("libtool", args, { cwd: libDirPath });
}

function resolveMergeOutputPath(libDirPath: string, outputLib: string): string {
  const shouldResolveFromCwd = outputLib.includes('/') || (IS_WINDOWS && outputLib.includes('\\'));
  if (isAbsolute(outputLib)) {
    return resolve(outputLib);
  }
  if (shouldResolveFromCwd) {
    return resolve(process.cwd(), outputLib);
  }
  return join(libDirPath, outputLib);
}

function pickInputLibFiles(files: string[]): string[] {
  const sortAlphabetically = (left: string, right: string) => left.localeCompare(right);
  const libFiles = files.filter(f => f.toLowerCase().endsWith(".lib")).sort(sortAlphabetically);
  if (libFiles.length > 0) {
    return libFiles;
  }
  return files.filter(f => f.endsWith(".a")).sort(sortAlphabetically);
}

function mergeWithExplicitTool(tool: string, libDirPath: string, libFiles: string[], outputPath: string) {
  console.log(`[MergeLibs] Using explicit tool path: ${tool}`);
  if (IS_WINDOWS && tool.toLowerCase().endsWith("lib.exe")) {
    mergeLibsWithWindowsLibExe(tool, libDirPath, libFiles, outputPath);
    return;
  }
  mergeLibsWithGenericAr(tool, libDirPath, libFiles, outputPath);
}

function mergeWithPlatformDefaultTool(libDirPath: string, libFiles: string[], outputPath: string) {
  if (IS_WINDOWS) {
    console.error(`[MergeLibs] YOU MUST SPECIFY THE ABSOLUTE PATH OF lib.exe on Windows`);
    process.exit(1);
  }
  if (IS_APPLE) {
    console.log("[MergeLibs] Using native macOS libtool");
    mergeLibsWithAppleLibTool(libDirPath, libFiles, outputPath);
    return;
  }
  console.log("[MergeLibs] Using native Linux ar");
  mergeLibsWithGenericAr("ar", libDirPath, libFiles, outputPath);
}

export function mergeLibs(libDir: string, outputLib: string, tool?: string) {
  const libDirPath = resolve(libDir);
  const outputPath = resolveMergeOutputPath(libDirPath, outputLib);
  
  if (!existsSync(libDirPath)) {
    console.error(`ERROR: No such directory: ${libDirPath}`);
    process.exit(1);
  }

  const outputDir = dirname(outputPath);
  if (!existsSync(outputDir)) mkdirSync(outputDir, { recursive: true });

  const files = readdirSync(libDirPath);
  const libFiles = pickInputLibFiles(files);

  if (libFiles.length === 0) {
    console.error(`ERROR: No static libraries (.a or .lib) found in ${libDirPath}`);
    process.exit(1);
  }

  try {
    if (tool) {
      mergeWithExplicitTool(tool, libDirPath, libFiles, outputPath);
    } else {
      mergeWithPlatformDefaultTool(libDirPath, libFiles, outputPath);
    }

    if (!existsSync(outputPath)) {
      console.error(`ERROR: Output library ${outputPath} was not created`);
      process.exit(1);
    }

    const sizeBytes = statSync(outputPath).size;
    console.log(`SUCCESS: ${outputPath} (${sizeBytes} Bytes)`);

  } catch (err: any) {
    console.error(`ERROR: Merge failed: ${err.message}`);
    process.exit(1);
  }
}
