import { join } from 'node:path';

import {
  gotoParentPath, setEnv,
} from '../utils.js';
import {
  checkArtifacts, copyStaticLibs, exportCompileCommands, getOutputLibPath, getVcpkgLibPath,
  mergeLibs, resolveBuildType, runCMake, setBuildOutputEnv, setupVcpkgEnv, shouldConfigureOnly
} from './build-utils.js';
import { getGtestCMakeConfigureArgs, shouldBuildTests } from '../test/gtest-utils.js';
import { getCoverageCMakeConfigureArgs, shouldEnableCoverage } from '../test/coverage-utils.js';

function main() {
  const root = gotoParentPath();

  const buildType = resolveBuildType();
  const platform = "OS64";
  const abi = "arm64";
  const ver = "15.0";

  const platformName = "iOS";
  const preset = `${platformName}-${buildType}`;

  setEnv("APPLE_TOOLCHAIN_FILE", join(root, "cmake", "toolchains", "Apple", "ios.toolchain.cmake"));
  setEnv("APPLE_PLATFORM", platform);
  setEnv("APPLE_ABI", abi);
  setEnv("APPLE_VER", ver);

  const buildFolder = join(`build.${platformName}`, buildType);
  const outputFolder = join(buildFolder, "output");
  const outputPath = join(root, outputFolder, abi);

  setBuildOutputEnv(buildFolder, outputPath);

  setupVcpkgEnv(`${abi}-ios`);

  const vcpkgLibPath = getVcpkgLibPath(root, buildFolder);
  const outputLibPath = getOutputLibPath();
  const buildTests = shouldBuildTests();
  const coverageEnabled = shouldEnableCoverage();
  const configureOnly = shouldConfigureOnly();

  runCMake(preset, buildFolder, outputFolder, true, [
    ...getGtestCMakeConfigureArgs(buildTests),
    ...getCoverageCMakeConfigureArgs(coverageEnabled)
  ]);

  exportCompileCommands(buildFolder, root);
  if (configureOnly) {
    return;
  }

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
}

main();
