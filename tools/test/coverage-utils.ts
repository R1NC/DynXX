import { spawnSync } from 'node:child_process';
import { existsSync, mkdirSync, readdirSync, writeFileSync } from 'node:fs';
import { basename, join, resolve } from 'node:path';
import { getEnv, spawn } from '../utils.js';

const COVERAGE_IGNORE_DIRS = [
  'test',
  'vcpkg_installed',
  '_deps'
] as const;

const COVERAGE_IGNORE_FILES = [
  'src/bridge/ScriptAPI.hxx',
  'src/bridge/ScriptAPI.cxx',
  'src/core/concurrent/TimerTask.hxx',
  'src/core/concurrent/TimerTask.cxx',
  'src/core/util/MemUtil.hxx',
  'src/core/util/TimeUtil.hxx'
] as const;

function escapeRegexLiteral(input: string): string {
  return input.replace(/[.*+?^${}()|[\]\\]/g, '\\$&');
}

function toPathRegex(pathLike: string): string {
  return pathLike
    .split(/[\\/]+/)
    .filter(Boolean)
    .map((part) => escapeRegexLiteral(part))
    .join('[/\\\\]');
}

function buildCoverageIgnoreRegex(): string {
  const dirPatterns = COVERAGE_IGNORE_DIRS.map((dir) => `[/\\\\]${toPathRegex(dir)}[/\\\\]`);
  const filePatterns = COVERAGE_IGNORE_FILES.map((file) => `[/\\\\]${toPathRegex(file)}$`);
  return `(${[...dirPatterns, ...filePatterns].join('|')})`;
}

const COVERAGE_IGNORE_REGEX = buildCoverageIgnoreRegex();

type CoverageCMakeFlags = {
  cFlags: string;
  cxxFlags: string;
};

function resolveLlvmToolPath(toolName: string): string {
  const llvmToolsHome = getEnv('LLVM_HOME');
  const toolBinary = process.platform === 'win32' ? `${toolName}.exe` : toolName;
  if (!llvmToolsHome) {
    return toolBinary;
  }
  return join(llvmToolsHome, toolBinary);
}

function resolveCoverageFlagFromArgs(): boolean | undefined {
  const args = process.argv.slice(2).map((arg) => arg.toLowerCase());
  if (args.includes('--coverage')) {
    return true;
  }

  const explicit = args.find((arg) => arg.startsWith('--coverage='));
  if (!explicit) {
    return undefined;
  }

  const value = explicit.slice('--coverage='.length);
  if (['1', 'true', 'on', 'yes'].includes(value)) {
    return true;
  }
  return undefined;
}

export function shouldEnableCoverage(): boolean {
  const coverageFlag = resolveCoverageFlagFromArgs();
  if (coverageFlag !== undefined) {
    return coverageFlag;
  }
  return false;
}

function getCoverageCMakeFlags(): CoverageCMakeFlags {
  if (process.platform === 'win32') {
    return {
      cFlags: '/clang:-fprofile-instr-generate /clang:-fcoverage-mapping',
      cxxFlags: '/EHsc /clang:-fprofile-instr-generate /clang:-fcoverage-mapping'
    };
  }
  return {
    cFlags: '-fprofile-instr-generate -fcoverage-mapping',
    cxxFlags: '-fprofile-instr-generate -fcoverage-mapping'
  };
}

export function getCoverageCMakeConfigureArgs(): string[] {
  const flags = getCoverageCMakeFlags();
  return [
    `-DCMAKE_C_FLAGS="${flags.cFlags}"`,
    `-DCMAKE_CXX_FLAGS="${flags.cxxFlags}"`
  ];
}

export function setupCoverageEnv(buildFolder: string): void {
  const { rawDir } = getCoverageReportPaths(buildFolder);
  mkdirSync(rawDir, { recursive: true });
  process.env.LLVM_PROFILE_FILE = join(rawDir, 'coverage-%p-%m.profraw');
}

export function getCoverageReportPaths(buildFolder: string): {
  reportDir: string;
  rawDir: string;
  summaryPath: string;
  htmlDir: string;
  lcovPath: string;
} {
  const reportDir = resolve(buildFolder, 'output', 'test', 'coverage');
  const rawDir = join(reportDir, 'raw');
  const summaryPath = join(reportDir, 'coverage-summary.txt');
  const htmlDir = join(reportDir, 'html');
  const lcovPath = join(reportDir, 'lcov.info');
  return { reportDir, rawDir, summaryPath, htmlDir, lcovPath };
}

function generateCoverageSummary(
  llvmCov: string,
  coverageExecutable: string,
  profdataPath: string,
  summaryPath: string
): boolean {
  const reportOutput = runLlvmCov(llvmCov, [
    'report',
    coverageExecutable,
    `-instr-profile=${profdataPath}`,
    `-ignore-filename-regex=${COVERAGE_IGNORE_REGEX}`
  ], 'failed to generate coverage summary');
  if (reportOutput === undefined) {
    return false;
  }
  writeFileSync(summaryPath, reportOutput, 'utf8');
  return true;
}

function generateCoverageHtml(
  llvmCov: string,
  coverageExecutable: string,
  profdataPath: string,
  htmlDir: string
): boolean {
  const showOutput = runLlvmCov(llvmCov, [
    'show',
    coverageExecutable,
    `-instr-profile=${profdataPath}`,
    '-format=html',
    `-output-dir=${htmlDir}`,
    `-ignore-filename-regex=${COVERAGE_IGNORE_REGEX}`
  ], 'failed to generate coverage html');
  if (showOutput === undefined) {
    return false;
  }
  return true;
}

function generateCodecovLcov(
  llvmCov: string,
  coverageExecutable: string,
  profdataPath: string,
  lcovPath: string
): boolean {
  const lcovOutput = runLlvmCov(llvmCov, [
    'export',
    coverageExecutable,
    `-instr-profile=${profdataPath}`,
    '-format=lcov',
    `-ignore-filename-regex=${COVERAGE_IGNORE_REGEX}`
  ], 'failed to generate codecov lcov');
  if (lcovOutput === undefined) {
    return false;
  }
  writeFileSync(lcovPath, lcovOutput, 'utf8');
  return true;
}

function runLlvmCov(llvmCov: string, args: string[], failureMessage: string): string | undefined {
  const result = spawn(llvmCov, args, { allowFailure: true });
  if (!result) {
    console.warn(`[WARN] ${failureMessage}`);
    return undefined;
  }
  return result.toString();
}

export function generateCoverageReport(buildFolder: string, testExecutable: string): void {
  const llvmProfdata = resolveLlvmToolPath('llvm-profdata');
  const llvmCov = resolveLlvmToolPath('llvm-cov');
  const { reportDir, rawDir, summaryPath, htmlDir, lcovPath } = getCoverageReportPaths(buildFolder);
  const coverageDir = rawDir;
  if (!existsSync(coverageDir)) {
    console.warn(`[WARN] coverage directory not found: ${coverageDir}`);
    return;
  }

  const profrawFiles = readdirSync(coverageDir)
    .filter((fileName) => fileName.endsWith('.profraw'))
    .map((fileName) => join(coverageDir, fileName));
  if (profrawFiles.length === 0) {
    console.warn(`[WARN] no profraw files found in ${coverageDir}`);
    return;
  }

  let coverageExecutable = testExecutable;
  if (!existsSync(coverageExecutable) && process.platform === 'darwin') {
    const appExecutable = join(`${testExecutable}.app`, 'Contents', 'MacOS', basename(testExecutable));
    if (existsSync(appExecutable)) {
      coverageExecutable = appExecutable;
    }
  }
  if (!existsSync(coverageExecutable)) {
    console.warn(`[WARN] test executable not found for coverage: ${testExecutable}`);
    return;
  }

  const profdataPath = join(coverageDir, 'coverage.profdata');
  const mergeResult = spawnSync(llvmProfdata, ['merge', '-sparse', ...profrawFiles, '-o', profdataPath], {
    stdio: ['ignore', 'pipe', 'pipe'],
    shell: false
  });
  if (mergeResult.error || mergeResult.status !== 0) {
    const msg = mergeResult.error?.message ?? mergeResult.stderr.toString().trim();
    console.warn(`[WARN] failed to merge coverage profiles: ${msg}`);
    return;
  }

  mkdirSync(reportDir, { recursive: true });

  if (!generateCoverageSummary(llvmCov, coverageExecutable, profdataPath, summaryPath)) {
    return;
  }

  if (!generateCoverageHtml(llvmCov, coverageExecutable, profdataPath, htmlDir)) {
    return;
  }

  if (!generateCodecovLcov(llvmCov, coverageExecutable, profdataPath, lcovPath)) {
    return;
  }

  console.log(`[Coverage] summary: ${summaryPath}`);
  console.log(`[Coverage] html: ${join(htmlDir, 'index.html')}`);
  console.log(`[Coverage] codecov: ${lcovPath}`);
}
