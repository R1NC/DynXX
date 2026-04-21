import { copyFileSync, existsSync, mkdirSync, readdirSync, readlinkSync, rmSync, statSync, symlinkSync, unlinkSync, writeFileSync } from 'node:fs';
import { basename, dirname, extname, isAbsolute, join, relative, resolve } from 'node:path';

import { exec, getEnv, getHomeDir, goInTmpDir, isMacOS, isWindows, readCIEnv, setEnv, spawn } from '../utils.js';

export function resolveBuildType(defaultValue: "Release" | "Debug" = "Release"): "Release" | "Debug" {
  const args = process.argv.slice(2);
  const lowerArgs = args.map((arg) => arg.toLowerCase());
  const hasDebug = lowerArgs.includes("--debug") || lowerArgs.includes("-d");
  const hasRelease = lowerArgs.includes("--release") || lowerArgs.includes("-r");

  if (hasDebug && hasRelease) {
    console.error("[Args] `--debug` and `--release` cannot be used together.");
    process.exit(1);
  }

  if (hasDebug) {
    return "Debug";
  }
  if (hasRelease) {
    return "Release";
  }

  return defaultValue;
}

export function shouldConfigureOnly(): boolean {
  const lowerArgs = process.argv.slice(2).map((arg) => arg.toLowerCase());
  if (lowerArgs.includes("--config-only") || lowerArgs.includes("--config")) {
    return true;
  }

  const npmConfigOnly = (process.env.npm_config_config_only || "").toLowerCase();
  if (["1", "true", "on"].includes(npmConfigOnly)) {
    return true;
  }

  const npmConfig = (process.env.npm_config_config || "").toLowerCase();
  return ["1", "true", "on"].includes(npmConfig);
}

function clearCMakeCache(buildFolder: string) {
  const buildDir = resolve(process.cwd(), buildFolder);
  const cacheFile = join(buildDir, "CMakeCache.txt");
  const cacheDir = join(buildDir, "CMakeFiles");
  const depsDir = join(buildDir, "_deps");

  if (existsSync(cacheFile)) {
    rmSync(cacheFile, { force: true });
  }
  if (existsSync(cacheDir)) {
    rmSync(cacheDir, { recursive: true, force: true });
  }
  if (existsSync(depsDir)) {
    rmSync(depsDir, { recursive: true, force: true });
  }
}

function clearOutputDir(outputFolder: string) {
  const outputDir = resolve(process.cwd(), outputFolder);
  if (!existsSync(outputDir)) {
    return;
  }
  rmSync(outputDir, { recursive: true, force: true });
}

export function exportCompileCommands(buildFolder: string, root: string) {
  const f = 'compile_commands.json';
  const src = resolve(root, buildFolder, f);
  const dst = resolve(root, f);

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

export function runCMake(
  preset: string,
  buildFolder: string,
  outputFolder: string,
  needInstall: boolean,
  configureArgs: string[] = [],
) {
  const configureOnly = shouldConfigureOnly();
  const extraArgs = configureArgs.join(' ').trim();
  const extraArgsSegment = extraArgs.length > 0 ? ` ${extraArgs}` : '';
  if (configureOnly) {
    clearCMakeCache(buildFolder);
  }
  exec(`cmake --preset ${preset}${extraArgsSegment}`);
  if (configureOnly) {
    console.log(`[CMake] Reconfigured preset ${preset} after clearing cache.`);
    return;
  }
  clearOutputDir(outputFolder);
  exec(`cmake --build --preset ${preset}`);
  if (needInstall) {
    exec(`cmake --install ${buildFolder} --prefix ${outputFolder} --component headers`);
  }
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
    missing.forEach((path) => console.log(`ARTIFACT NOT FOUND: ${path}`));
    process.exit(1);
  }
}

export function copyStaticLibs(srcDir: string, destDir: string) {
  const srcPath = resolve(srcDir);
  const destPath = resolve(destDir);

  if (!existsSync(srcPath)) return;
  if (!existsSync(destPath)) mkdirSync(destPath, { recursive: true });

  readdirSync(srcPath).forEach((file) => {
    const lowerFile = file.toLowerCase();
    if (lowerFile.endsWith('.a') || lowerFile.endsWith('.lib')) {
      copyFileSync(join(srcPath, file), join(destPath, file));
    }
  });
}

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

      spawn(arTool, ['x', join(libDirPath, lib)], { cwd: extractDir });
    }

    const objFiles = collectObjectFiles(extractDirs);
    if (objFiles.length === 0) {
      console.error('ERROR: No object files (.o or .obj) found after extraction');
      process.exit(1);
    }

    const rspPath = join(tempRoot, 'ar-merge.rsp');
    writeFileSync(rspPath, objFiles.map((f) => `"${f}"`).join('\n'), 'utf8');
    spawn(arTool, ['rcs', outputPath, `@${rspPath}`]);
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

      const listOutput = spawn(toolPath, ['/LIST', libPath], { cwd: extractDir, allowFailure: false });
      if (!listOutput) process.exit(1);

      const members = listOutput.toString()
        .split(/\r?\n/)
        .map((line) => line.trim())
        .filter((line) => line.length > 0);

      for (const member of members) {
        spawn(toolPath, [`/EXTRACT:${member}`, libPath], { cwd: extractDir });
      }
    }

    const objFiles = collectObjectFiles(extractDirs);
    if (objFiles.length === 0) {
      console.error('ERROR: No object files found after extraction from .lib files');
      process.exit(1);
    }

    const rspPath = join(tempRoot, 'merge-lib.rsp');
    const rspContent = [`/OUT:"${outputPath}"`, ...objFiles.map((f) => `"${f}"`)].join('\n');
    writeFileSync(rspPath, rspContent, 'utf8');

    spawn(toolPath, [`@${rspPath}`], { cwd: tempRoot });
  });
}

function mergeLibsWithAppleLibTool(libDirPath: string, aFiles: string[], outputPath: string) {
  const args = ['-static', '-o', outputPath, ...aFiles.map((lib) => join(libDirPath, lib))];
  spawn('libtool', args, { cwd: libDirPath });
}

function resolveMergeOutputPath(libDirPath: string, outputLib: string): string {
  const shouldResolveFromCwd = outputLib.includes('/') || (isWindows() && outputLib.includes('\\'));
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
  const libFiles = files.filter((f) => f.toLowerCase().endsWith('.lib')).sort(sortAlphabetically);
  if (libFiles.length > 0) {
    return libFiles;
  }
  return files.filter((f) => f.endsWith('.a')).sort(sortAlphabetically);
}

function mergeWithExplicitTool(tool: string, libDirPath: string, libFiles: string[], outputPath: string) {
  console.log(`[MergeLibs] Using explicit tool path: ${tool}`);
  if (isWindows() && tool.toLowerCase().endsWith('lib.exe')) {
    mergeLibsWithWindowsLibExe(tool, libDirPath, libFiles, outputPath);
    return;
  }
  mergeLibsWithGenericAr(tool, libDirPath, libFiles, outputPath);
}

function mergeWithPlatformDefaultTool(libDirPath: string, libFiles: string[], outputPath: string) {
  if (isWindows()) {
    console.error('[MergeLibs] YOU MUST SPECIFY THE ABSOLUTE PATH OF lib.exe on Windows');
    process.exit(1);
  }
  if (isMacOS()) {
    console.log('[MergeLibs] Using native macOS libtool');
    mergeLibsWithAppleLibTool(libDirPath, libFiles, outputPath);
    return;
  }
  console.log('[MergeLibs] Using native Linux ar');
  mergeLibsWithGenericAr('ar', libDirPath, libFiles, outputPath);
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

  console.log('[MergeLibs] libFiles:', libFiles);

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
