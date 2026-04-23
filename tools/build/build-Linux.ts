import { join } from 'node:path';

import {
  gotoParentPath,
} from '../utils.js';
import {
  checkArtifacts, copyStaticLibs, exportCompileCommands, getOutputExePath, getOutputLibPath, getVcpkgLibPath,
  isQjscEnabled, mergeLibs, resolveBuildType, runCMake, setBuildOutputEnv, setupVcpkgEnv, shouldConfigureOnly
} from './build-utils.js';
import { getGtestCMakeConfigureArgs, getGtestReportPaths, renderGtestXmlToHtml, runCtest, setupGtestEnv, shouldBuildTests } from '../test/gtest-utils.js';
import { generateCoverageReport, getCoverageCMakeConfigureArgs, getCoverageReportPaths, setupCoverageEnv, shouldEnableCoverage } from '../test/coverage-utils.js';

function main() {
  const root = gotoParentPath();

  const buildType = resolveBuildType();
  const abi = "x64";

  const platformName = "Linux";
  const preset = `${platformName}-${buildType}`;

  const buildFolder = join(`build.${platformName}`, buildType);
  const outputFolder = join(buildFolder, "output");
  const outputPath = join(root, outputFolder, abi);

  setBuildOutputEnv(buildFolder, outputPath);

  setupVcpkgEnv(`${abi}-linux`);

  const vcpkgLibPath = getVcpkgLibPath(root, buildFolder);
  const outputLibPath = getOutputLibPath();
  const outputExePath = getOutputExePath();
  const requestedBuildTests = shouldBuildTests();
  const requestedCoverage = shouldEnableCoverage();
  const configureOnly = shouldConfigureOnly();
  const buildTests = requestedBuildTests;
  const coverageEnabled = requestedCoverage;

  const configureArgs = [
    ...getGtestCMakeConfigureArgs(buildTests),
    ...getCoverageCMakeConfigureArgs(coverageEnabled)
  ];

  runCMake(preset, buildFolder, outputFolder, true, configureArgs);

  exportCompileCommands(buildFolder, root);
  if (configureOnly) {
    return;
  }

  if (isQjscEnabled(buildFolder)) {
    checkArtifacts([join(outputExePath, "qjsc")]);
  } else {
    console.warn("[Artifact] Linux build has USE_QJS=OFF; skip qjsc executable artifact check.");
  }
  checkArtifacts([join(outputLibPath, "libDynXX.a")]);

  copyStaticLibs(vcpkgLibPath, outputLibPath);
  mergeLibs(outputLibPath, "libDynXX-All.a");
  checkArtifacts([join(outputLibPath, "libDynXX-All.a")]);

  if (buildTests) {
    const { xmlReport, htmlReport } = getGtestReportPaths();
    setupGtestEnv();
    if (coverageEnabled) {
      setupCoverageEnv(buildFolder);
    }
    runCtest(buildFolder);
    renderGtestXmlToHtml(xmlReport, htmlReport);
    if (coverageEnabled) {
      generateCoverageReport(buildFolder, join(outputPath, "bin", "DynXXCxxTests"));
      const { summaryPath, htmlDir, lcovPath } = getCoverageReportPaths(buildFolder);
      checkArtifacts([summaryPath, join(htmlDir, "index.html"), lcovPath]);
    }
    checkArtifacts([xmlReport, htmlReport]);
  }
}

main();
