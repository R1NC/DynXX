import { join } from 'node:path';

import {
  gotoParentPath, setEnv,
} from '../utils.js';
import {
  checkArtifacts, copyStaticLibs, exportCompileCommands, getOutputExePath, getOutputLibPath,
  getVcpkgLibPath, mergeLibs, resolveBuildType, runCMake, setBuildOutputEnv, setupVcpkgEnv
} from './build-utils.js';
import { renderGtestXmlToHtml } from '../test/gen-test-report.js';
import { getGtestReportPaths, runCtest, setupGtestEnv, shouldBuildTests } from '../test/test-utils.js';

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
  const buildTests = shouldBuildTests();

  runCMake(preset, buildFolder, outputFolder, true, [`-DDYNXX_BUILD_TESTS=${buildTests ? "ON" : "OFF"}`]);

  exportCompileCommands(buildFolder, root);

  const buildArtifacts = [
    join(outputLibPath, "libDynXX.a"),
    join(outputExePath, "qjsc.app", "Contents", "MacOS", "qjsc"),
  ];
  if (buildTests) {
    buildArtifacts.push(
      join(outputLibPath, "libDynXXTest.a"),
      join(root, outputFolder, "include", "DynXXTest.hxx"),
    );
  }
  checkArtifacts(buildArtifacts);

  copyStaticLibs(vcpkgLibPath, outputLibPath);

  mergeLibs(outputLibPath, "libDynXX-All.a");
  
  checkArtifacts([join(outputLibPath, "libDynXX-All.a")]);

  if (buildTests) {
    const { xmlReport, htmlReport } = getGtestReportPaths();
    setupGtestEnv();
    runCtest(buildFolder);
    renderGtestXmlToHtml(xmlReport, htmlReport);
    checkArtifacts([xmlReport, htmlReport]);
  }
}

main();
