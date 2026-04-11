import * as fs from 'node:fs';
import * as path from 'node:path';
import { platform } from 'node:os';
import { exec, getEnv } from './utils.js';

const IS_WINDOWS = platform() === 'win32';
const VCPKG_REPO = "https://github.com/R1NC/vcpkg.git";
const VCPKG_REF = "dev";

const RUNNER_TEMP = getEnv("RUNNER_TEMP");
const VCPKG_ROOT = RUNNER_TEMP
  ? path.join(RUNNER_TEMP, 'vcpkg')
  : path.join(process.cwd(), 'temp_vcpkg');

async function main() {
  try {
    if (fs.existsSync(VCPKG_ROOT)) {
      fs.rmSync(VCPKG_ROOT, { recursive: true, force: true });
    }
    const parentDir = path.dirname(VCPKG_ROOT);
    fs.mkdirSync(parentDir, { recursive: true });

    exec(`git clone --branch ${VCPKG_REF} "${VCPKG_REPO}" "${VCPKG_ROOT}"`, parentDir);
    exec(`git -C "${VCPKG_ROOT}" remote set-url origin "${VCPKG_REPO}"`);
    const bootstrapScript = path.join(VCPKG_ROOT, IS_WINDOWS ? 'bootstrap-vcpkg.bat' : 'bootstrap-vcpkg.sh');
    
    if (!IS_WINDOWS) {
      fs.chmodSync(bootstrapScript, 0o755);
    }
    
    exec(`"${bootstrapScript}"`, VCPKG_ROOT);

    const githubEnv = getEnv("GITHUB_ENV");
    if (githubEnv) {
      fs.appendFileSync(githubEnv, `CI_VCPKG_HOME=${VCPKG_ROOT}\n`);
      console.log(`Exported CI_VCPKG_HOME=${VCPKG_ROOT} to GITHUB_ENV`);
    }

  } catch (error: any) {
    console.error('Fatal error during vcpkg setup:', error.message);
    process.exit(1);
  }
}

main();
