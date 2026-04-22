import { join } from 'node:path';

import {
  getMsvcToolsHome, gotoParentPath,
  isWindows,
} from '../utils.js';
import {
  checkArtifacts, copyStaticLibs, exportCompileCommands, getOutputExePath, getOutputLibPath,
  getVcpkgLibPath, isQjscEnabled, mergeLibs, resolveBuildType, runCMake, setBuildOutputEnv, setupVcpkgEnv, shouldConfigureOnly
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
  const outputExePath = getOutputExePath();
  const requestedBuildTests = shouldBuildTests();
  const requestedCoverage = shouldEnableCoverage();
  const configureOnly = shouldConfigureOnly();
  const generatedProject = buildType === "Debug";
  if (generatedProject && (requestedBuildTests || requestedCoverage)) {
    console.warn("[Test] Windows Debug preset generates VS projects; skip gtest/coverage flow.");
  }
  const buildTests = requestedBuildTests && !generatedProject;
  const coverageEnabled = requestedCoverage && !generatedProject;

  const configureArgs = [`-DDYNXX_BUILD_TESTS=${buildTests ? "ON" : "OFF"}`];
  if (coverageEnabled) {
    configureArgs.push(...getCoverageCMakeConfigureArgs());
  }
  runCMake(preset, buildFolder, outputFolder, true, configureArgs);
  
  exportCompileCommands(buildFolder, root);
  if (configureOnly) {
    return;
  }

  if (generatedProject) {
    console.warn("[Artifact] Windows Debug preset skips qjsc executable artifact check.");
    console.warn("[Artifact] Windows Debug preset skips static library artifact checks.");
    console.warn("[Merge] Windows Debug preset skips static library merge.");
  } else {
    if (isQjscEnabled(buildFolder)) {
      checkArtifacts([join(outputExePath, "qjsc.exe")]);
    } else {
      console.warn("[Artifact] Windows build has USE_QJS=OFF; skip qjsc executable artifact check.");
    }
    checkArtifacts([join(outputLibPath, "DynXX.lib")]);
    copyStaticLibs(vcpkgLibPath, outputLibPath);
    const libExePath = join(getMsvcToolsHome(), "lib.exe");
    mergeLibs(outputLibPath, "DynXX-All.lib", libExePath);
    checkArtifacts([join(outputLibPath, "DynXX-All.lib")]);
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
