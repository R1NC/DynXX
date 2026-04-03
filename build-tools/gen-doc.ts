import { existsSync, mkdirSync, writeFileSync } from 'node:fs';
import { join } from 'node:path';

import { exec, gotoParentPath, spawn } from './utils.js';

function normalizePath(path: string): string {
  return path.replace(/\\/g, '/');
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
      'C:\\Program Files\\doxygen\\bin\\doxygen.exe',
      'C:\\Program Files (x86)\\doxygen\\bin\\doxygen.exe',
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

  const typedocCli = join(root, 'build-tools', 'node_modules', 'typedoc', 'bin', 'typedoc');
  if (!existsSync(typedocCli)) {
    throw new Error(
      `typedoc is not installed. Run "npm install" in build-tools first. Missing: ${typedocCli}`
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

function writeLandingPage(siteDir: string): void {
  const html = [
    '<!doctype html>',
    '<html lang="en">',
    '<head>',
    '  <meta charset="utf-8">',
    '  <meta name="viewport" content="width=device-width, initial-scale=1">',
    '  <title>DynXX API Docs</title>',
    '  <style>',
    '    body{font-family:Arial,sans-serif;max-width:900px;margin:48px auto;padding:0 20px;color:#111;}',
    '    h1{margin-bottom:8px;}',
    '    p{color:#444;}',
    '    .cards{display:grid;grid-template-columns:repeat(auto-fit,minmax(240px,1fr));gap:16px;margin-top:24px;}',
    '    .card{display:block;padding:18px;border:1px solid #dcdcdc;border-radius:12px;text-decoration:none;color:inherit;}',
    '    .card:hover{border-color:#999;}',
    '    .title{font-size:20px;font-weight:600;margin-bottom:6px;}',
    '  </style>',
    '</head>',
    '<body>',
    '  <h1>DynXX API Docs</h1>',
    '  <p>Select the API set you want to browse.</p>',
    '  <div class="cards">',
    '    <a class="card" href="./DynXX-C/html/files.html">',
    '      <div class="title">DynXX-C</div>',
    '      <div>DynXX API for C</div>',
    '    </a>',
    '    <a class="card" href="./DynXX-Cxx/html/files.html">',
    '      <div class="title">DynXX-Cxx</div>',
    '      <div>DynXX API for C++</div>',
    '    </a>',
    '    <a class="card" href="./DynXX-TS/index.html">',
    '      <div class="title">DynXX-TS</div>',
    '      <div>DynXX API for TypeScript</div>',
    '    </a>',
    '  </div>',
    '</body>',
    '</html>',
    '',
  ].join('\n');

  writeFileSync(join(siteDir, 'index.html'), html, 'utf8');
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
  if (!systemDoxygen) {
    throw new Error(
      'doxygen is not installed. Please install doxygen manually and retry. '
      + 'You can also set DOXYGEN_BIN to the doxygen executable path.'
    );
  }
  console.log(`Using system doxygen: ${systemDoxygen}`);

  for (const variant of variants) {
    console.log(`Generating docs for ${variant.name}...`);
    generateVariant(root, variant, systemDoxygen);
  }

  console.log('Generating docs for DynXX-TS...');
  generateTsDocs(root, siteDir);

  writeLandingPage(siteDir);
  console.log(`Done. Site root: ${siteDir}`);
  console.log(`Open: ${join(siteDir, 'index.html')}`);
}

main();
