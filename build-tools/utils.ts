import { spawnSync } from 'node:child_process';
import { 
  existsSync, 
  unlinkSync, 
  symlinkSync, 
  copyFileSync, 
  mkdirSync, 
  rmSync, 
  readdirSync, 
  statSync,
  mkdtempSync
} from 'node:fs';
import { join, resolve, basename } from 'node:path';
import { tmpdir } from 'node:os';

function run(cmd: string, args: string[], cwd?: string) {
  const result = spawnSync(cmd, args, {
    cwd,
    stdio: 'inherit',
    shell: false
  });

  if (result.error) {
    throw result.error;
  }
  
  if (result.status !== 0) {
    process.exit(result.status || 1);
  }
}

function exportCompileCommands(buildFolder: string, root: string) {
  const src = resolve(root, buildFolder, "compile_commands.json");
  const dst = resolve(root, "compile_commands.json");

  if (existsSync(dst)) {
    unlinkSync(dst);
  }

  if (existsSync(src)) {
    try {
      symlinkSync(src, dst);
    } catch (_) {
      copyFileSync(src, dst);
    }
  }
}

function checkArtifacts(paths: string[]) {
  const missing: string[] = [];
  for (const path of paths) {
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

function copyStaticLibs(srcDir: string, destDir: string) {
  const srcPath = resolve(srcDir);
  const destPath = resolve(destDir);

  if (!existsSync(srcPath)) {
    return;
  }

  const files = readdirSync(srcPath);
  for (const file of files) {
    if (file.endsWith(".a")) {
      copyFileSync(join(srcPath, file), join(destPath, file));
    }
  }
}

function mergeLibs(libDir: string, outputLib: string, arTool: string) {
  const libDirPath = resolve(libDir);
  const outputPath = join(libDirPath, outputLib);
  const arName = basename(arTool);

  const files = readdirSync(libDirPath);
  const aFiles = files.filter(f => f.endsWith(".a")).sort();

  if (aFiles.length === 0) {
    console.log(`ERROR: No static libraries found in ${libDirPath}`);
    process.exit(1);
  }

  if (arName.includes("libtool")) {
    run(arTool, ["-static", "-o", outputPath, ...aFiles], libDirPath);
  } else {
    const tempDir = mkdtempSync(join(tmpdir(), "temp_merge_"));
    try {
      const extractDirs: string[] = [];
      
      for (let index = 0; index < aFiles.length; index++) {
        const lib = aFiles[index];
        const extractDir = join(tempDir, `lib${index}`);
        mkdirSync(extractDir, { recursive: true });
        extractDirs.push(extractDir);
        run(arTool, ["x", join(libDirPath, lib)], extractDir);
      }

      const objFiles: string[] = [];
      for (const dir of extractDirs) {
        const subFiles = readdirSync(dir);
        for (const f of subFiles) {
          if (f.endsWith(".o")) {
            objFiles.push(join(dir, f));
          }
        }
      }

      if (objFiles.length === 0) {
        console.log("ERROR: No object files found after extraction");
        process.exit(1);
      }

      run(arTool, ["rcs", outputPath, ...objFiles], libDirPath);
    } finally {
      rmSync(tempDir, { recursive: true, force: true });
    }
  }

  if (!existsSync(outputPath)) {
    console.log(`ERROR: Output library ${outputPath} was not created`);
    process.exit(1);
  }

  const sizeBytes = statSync(outputPath).size;
  console.log(`FOUND: ${outputPath} (${sizeBytes} Bytes)`);
}

export { run, exportCompileCommands, checkArtifacts, copyStaticLibs, mergeLibs };
