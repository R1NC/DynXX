import { join } from 'node:path';

import {
  gotoParentPath,
} from '../utils.js';
import {
  checkArtifacts, copyStaticLibs, exportCompileCommands, getOutputLibPath, getVcpkgLibPath,
  mergeLibs, resolveBuildType, runCMake, setBuildOutputEnv, setupVcpkgEnv
} from './build-utils.js';
import { getGtestReportPaths, renderGtestXmlToHtml, runCtest, setupGtestEnv, shouldBuildTests } from '../test/gtest-utils.js';

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

  runCMake(preset, buildFolder, outputFolder, true, [`-DDYNXX_BUILD_TESTS=${buildTests ? "ON" : "OFF"}`]);

  exportCompileCommands(buildFolder, root);

  const buildArtifacts = [join(outputLibPath, "libDynXX.a")];
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
