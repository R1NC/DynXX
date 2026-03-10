import { spawnSync, SpawnSyncOptions } from 'node:child_process';
import { 
  existsSync, 
  unlinkSync, 
  symlinkSync, 
  copyFileSync, 
  writeFileSync,
  mkdirSync, 
  rmSync, 
  readdirSync, 
  statSync,
  mkdtempSync,
  readlinkSync
} from 'node:fs';
import { dirname, join, resolve, basename, extname, isAbsolute } from 'node:path';
import { homedir, tmpdir, platform } from 'node:os';
import { fileURLToPath } from 'node:url';

const IS_WINDOWS = platform() === 'win32';
const IS_APPLE = platform() === 'darwin';

// Path Operations:

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

// Env Operations:

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
  if (!val) {
    throw new Error(`Environment variable ${name} is not set`);
  }
  return val;
}

export function getOutputLibPath(): string {
  return getRequiredEnv("OUTPUT_LIB_PATH");
}

export function getOutputDllPath(): string {
  return getRequiredEnv("OUTPUT_DLL_PATH");
}

export function getOutputExePath(): string {
  return getRequiredEnv("OUTPUT_EXE_PATH");
}

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

// Compile Operations:

function run(cmd: string, args: string[], cwd?: string, captureOutput?: boolean): Buffer | null {
  const options: SpawnSyncOptions = {
    cwd: cwd || process.cwd(),
    stdio: captureOutput ? 'pipe' : 'inherit',
    shell: false,
    windowsHide: true
  };

  const result = spawnSync(cmd, args, options);

  if (result.error) {
    throw result.error;
  }
  
  if (result.status !== 0) {
    if (!captureOutput && result.stderr) {
      console.error(result.stderr.toString());
    }
    if (!captureOutput) {
       process.exit(result.status || 1);
    } else {
       throw new Error(`Command failed with exit code ${result.status}`);
    }
  }

  return captureOutput ? result.stdout || null : null;
}

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
    } catch (_) {
      unlinkSync(dst);
    }
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
  run("cmake", ["--preset", preset]);
  run("cmake", ["--build", "--preset", preset]);
  if (needInstall) {
    run("cmake", ["--install", buildFolder, "--prefix", outputFolder, "--component", "headers"]);
  }
}

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
    for (const path of missing) {
      console.log(`ARTIFACT NOT FOUND: ${path}`);
    }
    process.exit(1);
  }
}

export function copyStaticLibs(srcDir: string, destDir: string) {
  const srcPath = resolve(srcDir);
  const destPath = resolve(destDir);

  if (!existsSync(srcPath)) {
    return;
  }

  if (!existsSync(destPath)) {
    mkdirSync(destPath, { recursive: true });
  }

  const files = readdirSync(srcPath);
  for (const file of files) {
    const lowerFile = file.toLowerCase();
    if (lowerFile.endsWith(".a") || lowerFile.endsWith(".lib")) {
      copyFileSync(join(srcPath, file), join(destPath, file));
    }
  }
}

// Merge Libs:

function collectFilesRecursive(rootPath: string): string[] {
  const files: string[] = [];
  const entries = readdirSync(rootPath);
  for (const entry of entries) {
    const fullPath = join(rootPath, entry);
    const stat = statSync(fullPath);
    if (stat.isDirectory()) {
      files.push(...collectFilesRecursive(fullPath));
    } else if (stat.isFile()) {
      files.push(fullPath);
    }
  }
  return files;
}

function collectObjectFilesAuto(extractDirs: { name: string, path: string }[]): string[] {
  const finalObjFiles: string[] = [];
  let foundExt: string | null = null;

  for (const dirInfo of extractDirs) {
    const subFiles = collectFilesRecursive(dirInfo.path);
    for (const f of subFiles) {
      if (f.toLowerCase().endsWith('.o')) {
        if (!foundExt) foundExt = '.o';
        if (foundExt === '.o') {
          finalObjFiles.push(f);
        }
      }
    }
  }

  if (finalObjFiles.length === 0) {
    for (const dirInfo of extractDirs) {
      const subFiles = collectFilesRecursive(dirInfo.path);
      for (const f of subFiles) {
        if (f.toLowerCase().endsWith('.obj')) {
          if (!foundExt) foundExt = '.obj';
          if (foundExt === '.obj') {
            finalObjFiles.push(f);
          }
        }
      }
    }
  }

  if (foundExt) {
    console.log(`[MergeLibs] Detected object format: ${foundExt}`);
  }

  return finalObjFiles;
}

function mergeLibsWithGenericAr(arTool: string, libDirPath: string, aFiles: string[], outputPath: string) {
  const tempRoot = mkdtempSync(join(tmpdir(), "temp_merge_ar_"));
  try {
    const extractDirs: { name: string, path: string }[] = [];
    
    for (const lib of aFiles) {
      const extractDir = join(tempRoot, `extract_${basename(lib, extname(lib))}`);
      mkdirSync(extractDir, { recursive: true });
      extractDirs.push({ name: lib, path: extractDir });
      
      const result = spawnSync(arTool, ["x", join(libDirPath, lib)], {
        cwd: extractDir,
        stdio: 'pipe',
        shell: false
      });
      
      if (result.status !== 0) {
        console.log(`ERROR: Failed to extract ${lib} with ${arTool}`);
        console.error(result.stderr?.toString() || result.error?.message);
        process.exit(1);
      }
    }

    const objFiles = collectObjectFilesAuto(extractDirs);

    if (objFiles.length === 0) {
      console.log("ERROR: No object files (.o or .obj) found after extraction");
      for (const dirInfo of extractDirs) {
        const files = readdirSync(dirInfo.path);
        if (files.length > 0) console.log(`Debug: ${dirInfo.path} contains`, files);
      }
      process.exit(1);
    }

    run(arTool, ["rcs", outputPath, ...objFiles]);

  } finally {
    rmSync(tempRoot, { recursive: true, force: true });
  }
}

function mergeLibsWithWindowsLibExe(toolPath: string, libDirPath: string, libFiles: string[], outputPath: string) {
  if (!existsSync(toolPath)) {
    throw new Error(`Specified lib.exe not found at: ${toolPath}`);
  }

  const tempRoot = mkdtempSync(join(tmpdir(), "temp_merge_lib_"));
  try {
    const extractDirs: { name: string, path: string }[] = [];

    for (const lib of libFiles) {
      const libPath = join(libDirPath, lib);
      const extractDir = join(tempRoot, `extract_${basename(lib, '.lib')}`);
      mkdirSync(extractDir, { recursive: true });
      extractDirs.push({ name: lib, path: extractDir });

      const listResult = spawnSync(toolPath, ["/LIST", libPath], {
        cwd: extractDir,
        stdio: 'pipe',
        shell: false
      });

      if (listResult.status !== 0) {
        console.log(`ERROR: Failed to list members from ${lib} with ${toolPath}. Exit code: ${listResult.status}`);
        console.error(listResult.stderr?.toString() || listResult.error?.message);
        process.exit(1);
      }

      const members = (listResult.stdout?.toString() || "")
        .split(/\r?\n/)
        .map((line) => line.trim())
        .filter((line) => line.length > 0);

      for (const member of members) {
        const extractResult = spawnSync(toolPath, [`/EXTRACT:${member}`, libPath], {
          cwd: extractDir,
          stdio: 'pipe',
          shell: false
        });

        if (extractResult.status !== 0) {
          console.log(`ERROR: Failed to extract ${member} from ${lib} with ${toolPath}. Exit code: ${extractResult.status}`);
          console.error(extractResult.stderr?.toString() || extractResult.error?.message);
          process.exit(1);
        }
      }
    }

    const objFiles = collectObjectFilesAuto(extractDirs);

    if (objFiles.length === 0) {
      console.log("ERROR: No object files found after extraction from .lib files");
      process.exit(1);
    }

    const rspPath = join(tempRoot, "merge-lib.rsp");
    const rspContent = [
      `/OUT:"${outputPath}"`,
      ...objFiles.map((file) => `"${file}"`)
    ].join("\n");
    writeFileSync(rspPath, rspContent, "utf8");
    run(toolPath, [`@${rspPath}`], tempRoot);

  } finally {
    rmSync(tempRoot, { recursive: true, force: true });
  }
}

function mergeLibsWithAppleLibTool(libDirPath: string, aFiles: string[], outputPath: string) {
  const tool = "libtool";
  const args = ["-static", "-o", outputPath];
  for (const lib of aFiles) {
    args.push(join(libDirPath, lib));
  }

  run(tool, args, libDirPath);
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
    console.log(`ERROR: No such directory: ${libDirPath}`);
    process.exit(1);
  }

  const outputDir = dirname(outputPath);
  if (!existsSync(outputDir)) {
    mkdirSync(outputDir, { recursive: true });
  }

  const files = readdirSync(libDirPath);
  
  let libFiles: string[] = [];
  const hasLib = files.some(f => f.toLowerCase().endsWith(".lib"));
  const hasA = files.some(f => f.endsWith(".a"));

  if (hasLib) {
    libFiles = files.filter((f: string) => f.toLowerCase().endsWith(".lib")).sort();
  } else if (hasA) {
    libFiles = files.filter((f: string) => f.endsWith(".a")).sort();
  }

  if (libFiles.length === 0) {
    console.log(`ERROR: No static libraries (.a or .lib) found in ${libDirPath}`);
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
        console.log(`[MergeLibs] YOU MUST SPECIFY THE ABSOLUTE PATH OF lib.exe`);
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
      console.log(`ERROR: Output library ${outputPath} was not created`);
      process.exit(1);
    }

    const sizeBytes = statSync(outputPath).size;
    console.log(`FOUND: ${outputPath} (${sizeBytes} Bytes)`);

  } catch (err: any) {
    console.log(`ERROR: Merge failed: ${err.message}`);
    process.exit(1);
  }
}
