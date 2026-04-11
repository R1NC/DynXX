import { getEnv } from '../utils.js';
import { mkdirSync } from 'node:fs';
import { spawnSync } from 'node:child_process';
import { dirname, resolve, join } from 'node:path';

function resolveTestFlagFromArgs(): boolean | undefined {
  const args = process.argv.slice(2).map((arg) => arg.toLowerCase());
  if (args.includes('--test')) {
    return true;
  }

  const explicit = args.find((arg) => arg.startsWith('--test='));
  if (!explicit) {
    return undefined;
  }

  const value = explicit.slice('--test='.length);
  if (['1', 'true', 'on', 'yes'].includes(value)) {
    return true;
  }
  return undefined;
}

export function shouldBuildTests(): boolean {
  const testFlag = resolveTestFlagFromArgs();
  if (testFlag !== undefined) {
    return testFlag;
  }
  return false;
}

export function getGtestReportPaths(): {
  reportDir: string;
  xmlReport: string;
  htmlReport: string;
} {
  const buildFolder = getEnv("BUILD_FOLDER");
  if (!buildFolder) {
    throw new Error("BUILD_FOLDER is empty; call setBuildOutputEnv() before test setup");
  }
  const reportDir = resolve(buildFolder, "output", "test-reports");
  const xmlReport = resolve(buildFolder, "test", "dynxx-gtest-report.xml");
  return {
    reportDir,
    xmlReport,
    htmlReport: join(reportDir, "gtest-report.html")
  };
}

export function setupGtestEnv(): void {
  const { reportDir, xmlReport } = getGtestReportPaths();
  mkdirSync(reportDir, { recursive: true });
  mkdirSync(dirname(xmlReport), { recursive: true });
  process.env.DYNXX_GTEST_OUTPUT = `xml:${xmlReport}`;
  process.env.GTEST_OUTPUT = `xml:${xmlReport}`;
}

export function runCtest(buildFolder: string, buildType?: string): number {
  const args = [
    '--test-dir', buildFolder,
    '--output-on-failure',
    '--stop-time', '300',
    '--parallel', '4',
    '--no-tests=error'
  ];
  if (buildType) {
    args.push('-C', buildType);
  }

  const result = spawnSync('ctest', args, {
    stdio: 'inherit',
    shell: false,
    env: process.env
  });

  if (result.error) {
    throw result.error;
  }
  const exitCode = result.status ?? 1;
  if (exitCode !== 0) {
    console.warn(`[WARN] ctest exited with code ${exitCode}, continue to generate test report`);
  }
  return exitCode;
}
