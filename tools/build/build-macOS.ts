import { join } from 'node:path';

import {
  gotoParentPath, setEnv,
} from '../utils.js';
import {
  checkArtifacts, copyStaticLibs, exportCompileCommands, getOutputExePath, getOutputLibPath,
  getVcpkgLibPath, mergeLibs, resolveBuildType, runCMake, setBuildOutputEnv, setupVcpkgEnv, shouldConfigureOnly
} from './build-utils.js';
import { getGtestReportPaths, renderGtestXmlToHtml, runCtest, setupGtestEnv, shouldBuildTests } from '../test/gtest-utils.js';
import { generateCoverageReport, getCoverageCMakeConfigureArgs, getCoverageReportPaths, setupCoverageEnv, shouldEnableCoverage } from '../test/coverage-utils.js';

function main() {
  const root = gotoParentPath();

  const buildType = resolveBuildType();
  const platform = "MAC_UNIVERSAL";
  const abi = "arm64";
  const ver = "15.0";

  const platformName = "macOS";
  const preset = `${platformName}-${buildType}`;

  setEnv("APPLE_TOOLCHAIN_FILE", join(root, "cmake", "toolchains", "Apple", "ios.toolchain.cmake"));
  setEnv("APPLE_PLATFORM", platform);
  setEnv("APPLE_ABI", abi);
  setEnv("APPLE_VER", ver);

  const buildFolder = join(`build.${platformName}`, buildType);
  const outputFolder = join(buildFolder, "output");
  const outputPath = join(root, outputFolder, abi)

  setBuildOutputEnv(buildFolder, outputPath);

  setupVcpkgEnv(`${abi}-osx`);

  const vcpkgLibPath = getVcpkgLibPath(root, buildFolder);
  const outputLibPath = getOutputLibPath();
  const outputExePath = getOutputExePath();
  const requestedBuildTests = shouldBuildTests();
  const requestedCoverage = shouldEnableCoverage();
  const configureOnly = shouldConfigureOnly();
  const generatedProject = buildType === "Debug";
  if (generatedProject && (requestedBuildTests || requestedCoverage)) {
    console.warn("[Test] macOS Debug preset uses Xcode generator; skip gtest/coverage flow.");
  }
  const buildTests = requestedBuildTests && !generatedProject;
  const coverageEnabled = requestedCoverage && !generatedProject;

  runCMake(preset, buildFolder, outputFolder, true, [
    `-DDYNXX_BUILD_TESTS=${buildTests ? "ON" : "OFF"}`,
    ...(coverageEnabled ? getCoverageCMakeConfigureArgs() : [])
  ]);

  exportCompileCommands(buildFolder, root);
  if (configureOnly) {
    return;
  }

  if (generatedProject) {
    console.warn("[Artifact] macOS Debug preset skips qjsc executable artifact check.");
    console.warn("[Artifact] macOS Debug preset skips static library artifact checks.");
    console.warn("[Merge] macOS Debug preset skips static library merge.");
  } else {
    checkArtifacts([join(outputExePath, "qjsc.app", "Contents", "MacOS", "qjsc")]);
    checkArtifacts([join(outputLibPath, "libDynXX.a")]);
    copyStaticLibs(vcpkgLibPath, outputLibPath);
    mergeLibs(outputLibPath, "libDynXX-All.a");
    checkArtifacts([join(outputLibPath, "libDynXX-All.a")]);
  }

  if (buildTests) {
    const { xmlReport, htmlReport } = getGtestReportPaths();
    setupGtestEnv();
    if (coverageEnabled) {
      setupCoverageEnv(buildFolder);
    }
    runCtest(buildFolder);
    renderGtestXmlToHtml(xmlReport, htmlReport);
    if (coverageEnabled) {
      generateCoverageReport(buildFolder, join(outputPath, 'bin', 'DynXXCxxTests'));
      const { summaryPath, htmlDir, lcovPath } = getCoverageReportPaths(buildFolder);
      checkArtifacts([summaryPath, join(htmlDir, 'index.html'), lcovPath]);
    }
    checkArtifacts([xmlReport, htmlReport]);
  }
}

main();
