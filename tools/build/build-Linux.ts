import { join } from 'node:path';

import {
  gotoParentPath,
} from '../utils.js';
import {
  checkArtifacts, copyStaticLibs, exportCompileCommands, getOutputLibPath, getVcpkgLibPath,
  mergeLibs, resolveBuildType, runCMake, setBuildOutputEnv, setupVcpkgEnv, shouldConfigureOnly
} from './build-utils.js';
import { getGtestReportPaths, renderGtestXmlToHtml, runCtest, setupGtestEnv, shouldBuildTests } from '../test/gtest-utils.js';
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
  const buildTests = shouldBuildTests();
  const coverageEnabled = shouldEnableCoverage();
  const configureOnly = shouldConfigureOnly();

  const configureArgs = [`-DDYNXX_BUILD_TESTS=${buildTests ? "ON" : "OFF"}`];
  if (coverageEnabled) {
    configureArgs.push(...getCoverageCMakeConfigureArgs());
  }

  runCMake(preset, buildFolder, outputFolder, true, configureArgs);

  exportCompileCommands(buildFolder, root);
  if (configureOnly) {
    return;
  }

  const buildArtifacts = [join(outputLibPath, "libDynXX.a")];
  checkArtifacts(buildArtifacts);

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
