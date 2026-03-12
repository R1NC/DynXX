import { spawnSync, execSync, SpawnSyncOptions, SpawnSyncReturns } from 'node:child_process';
import { 
  existsSync, unlinkSync, symlinkSync, copyFileSync, writeFileSync,
  mkdirSync, rmSync, readdirSync, statSync, mkdtempSync, readlinkSync, chmodSync
} from 'node:fs';
import { dirname, join, resolve, basename, extname, isAbsolute } from 'node:path';
import { homedir, tmpdir, platform } from 'node:os';
import { fileURLToPath } from 'node:url';

const IS_WINDOWS = platform() === 'win32';
const IS_APPLE = platform() === 'darwin';

// --- Path & Env Helpers ---

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
  return homedir() || process.env.HOME || process.env.USERPROFILE || "";
}

export function readCIEnv(ciEnvName: string, localEnvName: string): string {
  const ciValue = process.env[ciEnvName];
  if (ciValue && !process.env[localEnvName]) {
    process.env[localEnvName] = ciValue;
  }
  return process.env[localEnvName] || "";
}

export function setBuildOutputEnv(buildFolder: string, outputPath: string) {
  process.env.BUILD_FOLDER = buildFolder;
  process.env.OUTPUT_LIB_PATH = join(outputPath, "lib");
  process.env.OUTPUT_DLL_PATH = join(outputPath, "share");
  process.env.OUTPUT_EXE_PATH = join(outputPath, "bin");
}

function getRequiredEnv(name: string): string {
  const val = process.env[name];
  if (!val) throw new Error(`Environment variable ${name} is not set`);
  return val;
}

export const getOutputLibPath = (): string => getRequiredEnv("OUTPUT_LIB_PATH");
export const getOutputDllPath = (): string => getRequiredEnv("OUTPUT_DLL_PATH");
export const getOutputExePath = (): string => getRequiredEnv("OUTPUT_EXE_PATH");

export function setupVcpkgEnv(triplet: string) {
  readCIEnv("CI_VCPKG_HOME", "VCPKG_HOME");
  const home = getHomeDir();
  if (!process.env.VCPKG_BINARY_SOURCES) {
    process.env.VCPKG_BINARY_SOURCES = `files,${join(home, "vcpkg-binary-cache")},readwrite`;
  }
  if (!process.env.VCPKG_TARGET_TRIPLET) {
    process.env.VCPKG_TARGET_TRIPLET = triplet;
  }
}

export function getVcpkgLibPath(root: string, buildFolder: string): string {
  const triplet = getRequiredEnv("VCPKG_TARGET_TRIPLET");
  return join(root, buildFolder, "vcpkg_installed", triplet, "lib");
}

// --- Run Tools ---

/**
 * Executes a command string via shell.
 * Use for: Scripts, pipes, redirects, or simple commands where output logging is enough.
 */
export function runSafe(command: string, cwd?: string): void {
  try {
    execSync(command, {
      cwd,
      stdio: 'inherit',
      env: process.env,
      shell: IS_WINDOWS
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
 * Use for: Getting versions, hashes, lists, or when arguments contain spaces/special chars.
 */
export function runOut(
  cmd: string, 
  args: string[], 
  options: ExecOptions = {}
): Buffer | null {
  const { cwd = process.cwd(), tempDir, allowFailure = false } = options;

  const spawnOptions: SpawnSyncOptions = {
    cwd,
    stdio: ['ignore', 'pipe', 'pipe'], // stdin:ignore, stdout:pipe, stderr:pipe
    shell: false,
    windowsHide: true
  };

  const result: SpawnSyncReturns<Buffer> = spawnSync(cmd, args, spawnOptions);

  if (result.error) {
    console.error(`[ERROR] Failed to spawn ${cmd}:`, result.error.message);
    if (!allowFailure) process.exit(1);
    return null;
  }

  if (result.status !== 0) {
    const stderrMsg = result.stderr ? result.stderr.toString() : 'Unknown error';
    if (stderrMsg) console.error(`[ERROR] ${cmd} failed: ${stderrMsg}`);

    if (!allowFailure) {
      throw new Error(`Command failed with exit code ${result.status}: ${cmd} ${args.join(' ')}`);
    } else {
      console.warn(`[WARN] Command exited with ${result.status}: ${cmd}`);
      return null;
    }
  }

  if (tempDir && existsSync(tempDir)) {
    try {
      rmSync(tempDir, { recursive: true, force: true });
    } catch (e) {
      console.warn(`[WARN] Failed to cleanup temp dir ${tempDir}:`, e);
    }
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
        if (resolve(root, target) === src) return;
      }
      unlinkSync(dst);
    } catch (_) { unlinkSync(dst); }
  }

  if (existsSync(src)) {
    try {
      const type = IS_WINDOWS ? 'junction' : 'file';
      symlinkSync(src, dst, type);
    } catch (_) {
      copyFileSync(src, dst);
    }
  }
}

export function runCMake(preset: string, buildFolder: string, outputFolder: string, needInstall: boolean) {
  runSafe(`cmake --preset ${preset}`);
  runSafe(`cmake --build --preset ${preset}`);
  
  if (needInstall) {
    runSafe(`cmake --install ${buildFolder} --prefix ${outputFolder} --component headers`);
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
  const finalObjFiles = oFiles.length > 0
    ? oFiles
    : allEntries.filter((file) => file.toLowerCase().endsWith('.obj'));

  if (finalObjFiles.length > 0) {
    const ext = oFiles.length > 0 ? '.o' : '.obj';
    console.log(`[MergeLibs] Detected object format: ${ext} (Count: ${finalObjFiles.length})`);
  }

  return finalObjFiles;
}

function mergeLibsWithGenericAr(arTool: string, libDirPath: string, aFiles: string[], outputPath: string) {
  goInTmpDir((tempRoot) => {
    const extractDirs: { path: string }[] = [];
    
    for (const lib of aFiles) {
      const extractDir = join(tempRoot, `extract_${basename(lib, extname(lib))}`);
      mkdirSync(extractDir, { recursive: true });
      extractDirs.push({ path: extractDir });

      runOut(arTool, ["x", join(libDirPath, lib)], { cwd: extractDir });
    }

    const objFiles = collectObjectFiles(extractDirs);
    if (objFiles.length === 0) {
      console.error("ERROR: No object files (.o or .obj) found after extraction");
      process.exit(1);
    }

    runOut(arTool, ["rcs", outputPath, ...objFiles]);
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

      const listOutput = runOut(toolPath, ["/LIST", libPath], { cwd: extractDir, allowFailure: false });
      if (!listOutput) process.exit(1);

      const members = listOutput.toString()
        .split(/\r?\n/)
        .map(line => line.trim())
        .filter(line => line.length > 0);

      for (const member of members) {
        runOut(toolPath, [`/EXTRACT:${member}`, libPath], { cwd: extractDir });
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
    
    runOut(toolPath, [`@${rspPath}`], { cwd: tempRoot });
  });
}

function mergeLibsWithAppleLibTool(libDirPath: string, aFiles: string[], outputPath: string) {
  const args = ["-static", "-o", outputPath, ...aFiles.map(lib => join(libDirPath, lib))];
  runOut("libtool", args, { cwd: libDirPath });
}

export function mergeLibs(libDir: string, outputLib: string, tool?: string) {
  const libDirPath = resolve(libDir);
  
  let outputPath: string;
  if (isAbsolute(outputLib)) {
    outputPath = resolve(outputLib);
  } else if (outputLib.includes('/') || (IS_WINDOWS && outputLib.includes('\\'))) {
    outputPath = resolve(process.cwd(), outputLib);
  } else {
    outputPath = join(libDirPath, outputLib);
  }
  
  if (!existsSync(libDirPath)) {
    console.error(`ERROR: No such directory: ${libDirPath}`);
    process.exit(1);
  }

  const outputDir = dirname(outputPath);
  if (!existsSync(outputDir)) mkdirSync(outputDir, { recursive: true });

  const files = readdirSync(libDirPath);
  const hasLib = files.some(f => f.toLowerCase().endsWith(".lib"));
  const hasA = files.some(f => f.endsWith(".a"));

  let libFiles: string[] = [];
  if (hasLib) {
    libFiles = files.filter(f => f.toLowerCase().endsWith(".lib")).sort();
  } else if (hasA) {
    libFiles = files.filter(f => f.endsWith(".a")).sort();
  }

  if (libFiles.length === 0) {
    console.error(`ERROR: No static libraries (.a or .lib) found in ${libDirPath}`);
    process.exit(1);
  }

  try {
    if (tool) {
      console.log(`[MergeLibs] Using explicit tool path: ${tool}`);
      if (IS_WINDOWS && tool.toLowerCase().endsWith("lib.exe")) {
         mergeLibsWithWindowsLibExe(tool, libDirPath, libFiles, outputPath);
      } else {
         mergeLibsWithGenericAr(tool, libDirPath, libFiles, outputPath);
      }
    } else {
      if (IS_WINDOWS) {
        console.error(`[MergeLibs] YOU MUST SPECIFY THE ABSOLUTE PATH OF lib.exe on Windows`);
        process.exit(1);
      } else if (IS_APPLE) {
        console.log("[MergeLibs] Using native macOS libtool");
        mergeLibsWithAppleLibTool(libDirPath, libFiles, outputPath);
      } else {
        console.log("[MergeLibs] Using native Linux ar");
        mergeLibsWithGenericAr("ar", libDirPath, libFiles, outputPath);
      }
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
