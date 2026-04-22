import { join } from 'node:path';

import {
  getMsvcToolsHome, gotoParentPath,
  isWindows,
} from '../utils.js';
import {
  checkArtifacts, copyStaticLibs, exportCompileCommands, getOutputExePath, getOutputLibPath,
  getVcpkgLibPath, mergeLibs, resolveBuildType, runCMake, setBuildOutputEnv, setupVcpkgEnv, shouldConfigureOnly
} from './build-utils.js';
import { getGtestReportPaths, renderGtestXmlToHtml, runCtest, setupGtestEnv, shouldBuildTests } from '../test/gtest-utils.js';
import { generateCoverageReport, getCoverageCMakeConfigureArgs, getCoverageReportPaths, setupCoverageEnv, shouldEnableCoverage } from '../test/coverage-utils.js';

function main() {
  if (!isWindows()) {
    throw new Error('build:windows is only supported on Windows hosts');
  }

  const root = gotoParentPath();

  const buildType = resolveBuildType();
  const abi = "x64";
  
  const platformName = "Windows";

  const preset = `${platformName}-${buildType}`;
  const buildFolder = join(`build.${platformName}`, buildType);
  const outputFolder = join(buildFolder, "output");
  const outputPath = join(root, outputFolder, abi);

  setBuildOutputEnv(buildFolder, outputPath);
  setupVcpkgEnv(`${abi}-windows-static`);

  const vcpkgLibPath = getVcpkgLibPath(root, buildFolder);
  const outputLibPath = getOutputLibPath();
  const requestedBuildTests = shouldBuildTests();
  const requestedCoverage = shouldEnableCoverage();
  const configureOnly = shouldConfigureOnly();
  const skipHostTests = buildType === "Debug";
  if (skipHostTests && (requestedBuildTests || requestedCoverage)) {
    console.warn("[Test] Windows Debug preset generates VS projects; skip gtest/coverage flow.");
  }
  const buildTests = requestedBuildTests && !skipHostTests;
  const coverageEnabled = requestedCoverage && !skipHostTests;
  const skipLibMerge = skipHostTests;

  const configureArgs = [`-DDYNXX_BUILD_TESTS=${buildTests ? "ON" : "OFF"}`];
  if (coverageEnabled) {
    configureArgs.push(...getCoverageCMakeConfigureArgs());
  }
  runCMake(preset, buildFolder, outputFolder, true, configureArgs);
  
  exportCompileCommands(buildFolder, root);
  if (configureOnly) {
    return;
  }

  if (skipLibMerge) {
    console.warn("[Merge] Windows Debug preset skips static library merge.");
  } else {
    copyStaticLibs(vcpkgLibPath, outputLibPath);
    const libExePath = join(getMsvcToolsHome(), "lib.exe");
    mergeLibs(outputLibPath, "DynXX-All.lib", libExePath);
    checkArtifacts([join(outputLibPath, "DynXX-All.lib")]);
  }

  if (skipHostTests) {
    console.warn("[Artifact] Windows Debug preset skips qjsc executable artifact check.");
  } else {
    checkArtifacts([join(getOutputExePath(), "qjsc.exe")]);
  }

  if (buildTests) {
    const { xmlReport, htmlReport } = getGtestReportPaths();
    setupGtestEnv();
    if (coverageEnabled) {
      setupCoverageEnv(buildFolder);
    }
    runCtest(buildFolder, buildType);
    renderGtestXmlToHtml(xmlReport, htmlReport);
    if (coverageEnabled) {
      generateCoverageReport(buildFolder, join(outputPath, "bin", "DynXXCxxTests.exe"));
      const { summaryPath, htmlDir, lcovPath } = getCoverageReportPaths(buildFolder);
      checkArtifacts([summaryPath, join(htmlDir, "index.html"), lcovPath]);
    }
    checkArtifacts([xmlReport, htmlReport]);
  }
}

main();
