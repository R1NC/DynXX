import { existsSync, mkdirSync, writeFileSync } from 'node:fs';
import { join } from 'node:path';

import { exec, gotoParentPath, spawn } from './utils.js';

function normalizePath(path: string): string {
  return path.replaceAll('\\', '/');
}

function buildDoxygenConfig(projectName: string, inputDir: string, outputDir: string): string {
  return [
    `PROJECT_NAME = "${projectName}"`,
    `OUTPUT_DIRECTORY = ${normalizePath(outputDir)}`,
    'GENERATE_HTML = YES',
    'GENERATE_LATEX = NO',
    'RECURSIVE = YES',
    `INPUT = ${normalizePath(inputDir)}`,
    'FILE_PATTERNS = *.h *.hpp *.hxx',
    'EXTRACT_ALL = YES',
    'QUIET = YES',
    'WARN_IF_UNDOCUMENTED = NO',
    'FULL_PATH_NAMES = NO',
    'HAVE_DOT = NO',
    '',
  ].join('\n');
}

function findSystemDoxygen(root: string): string | null {
  const fromPath = spawn('doxygen', ['--version'], { cwd: root, allowFailure: true });
  if (fromPath) return 'doxygen';

  const fromEnv = process.env.DOXYGEN_BIN?.trim();
  if (fromEnv && existsSync(fromEnv)) {
    const ok = spawn(fromEnv, ['--version'], { cwd: root, allowFailure: true });
    if (ok) return fromEnv;
  }

  const candidates = process.platform === 'win32'
    ? [
      String.raw`C:\Program Files\doxygen\bin\doxygen.exe`,
      String.raw`C:\Program Files (x86)\doxygen\bin\doxygen.exe`,
    ]
    : [
      '/usr/bin/doxygen',
      '/usr/local/bin/doxygen',
    ];

  for (const candidate of candidates) {
    if (!existsSync(candidate)) continue;
    const ok = spawn(candidate, ['--version'], { cwd: root, allowFailure: true });
    if (ok) return candidate;
  }

  return null;
}

interface DocVariant {
  name: string;
  inputDir: string;
  outputDir: string;
}

function findSystemLDoc(root: string): string | null {
  const fromPath = spawn('ldoc', ['--version'], { cwd: root, allowFailure: true });
  if (fromPath) return 'ldoc';

  const fromEnv = process.env.LDOC_BIN?.trim();
  if (fromEnv && existsSync(fromEnv)) {
    const ok = spawn(fromEnv, ['--version'], { cwd: root, allowFailure: true });
    if (ok) return fromEnv;
  }

  const candidates = process.platform === 'win32'
    ? [
      String.raw`C:\Program Files\Lua\bin\ldoc.bat`,
      String.raw`C:\Program Files\Lua\bin\ldoc.exe`,
    ]
    : [
      '/usr/bin/ldoc',
      '/usr/local/bin/ldoc',
    ];

  for (const candidate of candidates) {
    if (!existsSync(candidate)) continue;
    const ok = spawn(candidate, ['--version'], { cwd: root, allowFailure: true });
    if (ok) return candidate;
  }

  return null;
}

function generateVariant(root: string, variant: DocVariant, systemDoxygen: string): void {
  if (!existsSync(variant.inputDir)) {
    throw new Error(`Input path does not exist: ${variant.inputDir}`);
  }

  mkdirSync(variant.outputDir, { recursive: true });
  const doxyfilePath = join(variant.outputDir, 'Doxyfile');
  writeFileSync(
    doxyfilePath,
    buildDoxygenConfig(variant.name, variant.inputDir, variant.outputDir),
    'utf8'
  );

  console.log(`[${variant.name}] Input: ${variant.inputDir}`);
  console.log(`[${variant.name}] Doxyfile: ${doxyfilePath}`);

  exec(`"${systemDoxygen}" "${doxyfilePath}"`, root);
}

function generateTsDocs(root: string, siteDir: string): void {
  const tsInput = join(root, 'scripts', 'JS', 'DynXX.d.ts');
  if (!existsSync(tsInput)) {
    throw new Error(`TS declaration file does not exist: ${tsInput}`);
  }

  const typedocCli = join(root, 'tools', 'node_modules', 'typedoc', 'bin', 'typedoc');
  if (!existsSync(typedocCli)) {
    throw new Error(
      `typedoc is not installed. Run "npm install" in tools first. Missing: ${typedocCli}`
    );
  }

  const tsOutput = join(siteDir, 'DynXX-TS');
  const tsConfig = join(siteDir, 'typedoc-tsconfig.json');
  mkdirSync(tsOutput, { recursive: true });
  writeFileSync(
    tsConfig,
    JSON.stringify({
      files: [tsInput],
      compilerOptions: {
        lib: ['es2015', 'dom'],
        skipLibCheck: true,
      },
    }, null, 2),
    'utf8'
  );
  exec(
    `node "${typedocCli}" --tsconfig "${tsConfig}" --entryPoints "${tsInput}" --out "${tsOutput}" --name "DynXX-TS" --readme none`,
    root
  );
}

function generateLuaDocs(root: string, siteDir: string): void {
  const luaInput = join(root, 'scripts', 'Lua', 'DynXX.lua');
  if (!existsSync(luaInput)) {
    throw new Error(`Lua API file does not exist: ${luaInput}`);
  }

  const ldocBin = findSystemLDoc(root);
  if (!ldocBin) {
    throw new Error(
      'ldoc is not installed. Please install ldoc manually and retry. '
      + 'You can also set LDOC_BIN to the ldoc executable path.'
    );
  }

  const luaOutput = join(siteDir, 'DynXX-Lua');
  mkdirSync(luaOutput, { recursive: true });

  const bin = ldocBin === 'ldoc' ? 'ldoc' : `"${ldocBin}"`;
  exec(
    `${bin} -a -i -p "DynXX-Lua" -t "DynXX Lua API" -d "${luaOutput}" "${luaInput}"`,
    root
  );
}

function printDocError(scope: string, error: unknown): void {
  const message = error instanceof Error ? error.message : String(error);
  console.error(`[${scope}] ${message}`);
}

function main() {
  const root = gotoParentPath();
  const includeRoot = join(root, 'include', 'DynXX');
  const docBuildDir = join(root, 'build.Docs');
  const siteDir = join(docBuildDir, 'site');
  mkdirSync(siteDir, { recursive: true });

  const variants: DocVariant[] = [
    {
      name: 'DynXX-C',
      inputDir: join(includeRoot, 'C'),
      outputDir: join(siteDir, 'DynXX-C'),
    },
    {
      name: 'DynXX-Cxx',
      inputDir: join(includeRoot, 'CXX'),
      outputDir: join(siteDir, 'DynXX-Cxx'),
    },
  ];

  const systemDoxygen = findSystemDoxygen(root);
  if (systemDoxygen === null) {
    printDocError(
      'DynXX-C/DynXX-Cxx',
      'doxygen is not installed. Please install doxygen manually. '
      + 'You can also set DOXYGEN_BIN to the doxygen executable path.'
    );
  } else {
    console.log(`Using system doxygen: ${systemDoxygen}`);
    for (const variant of variants) {
      console.log(`Generating docs for ${variant.name}...`);
      try {
        generateVariant(root, variant, systemDoxygen);
      } catch (error) {
        printDocError(variant.name, error);
      }
    }
  }

  try {
    console.log('Generating docs for DynXX-TS...');
    generateTsDocs(root, siteDir);
  } catch (error) {
    printDocError('DynXX-TS', error);
  }

  try {
    console.log('Generating docs for DynXX-Lua...');
    generateLuaDocs(root, siteDir);
  } catch (error) {
    printDocError('DynXX-Lua', error);
  }

  console.log(`Done. Site root: ${siteDir}`);
}

main();
