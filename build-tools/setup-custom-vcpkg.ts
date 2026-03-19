import * as fs from 'node:fs';
import * as path from 'node:path';
import { platform } from 'node:os';
import { exec, getEnv, spawn } from './utils.js';

const IS_WINDOWS = platform() === 'win32';
const VCPKG_URL = "https://github.com/R1NC/vcpkg/archive/refs/heads/dev.zip";

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

    const zipPath = path.join(parentDir, 'vcpkg-dev.zip');
    const extractDir = path.join(parentDir, 'vcpkg-dev');

    if (IS_WINDOWS) {
      exec(`powershell -NoProfile -Command "Invoke-WebRequest -Uri '${VCPKG_URL}' -OutFile '${zipPath}'"`);
    } else {
      exec(`curl -L -o '${zipPath}' '${VCPKG_URL}'`);
    }

    if (IS_WINDOWS) {
      exec(`powershell -NoProfile -Command "Expand-Archive -LiteralPath '${zipPath}' -DestinationPath '${parentDir}' -Force"`);
    } else {
      try {
        exec(`unzip -q '${zipPath}' -d '${parentDir}'`);
      } catch {
        exec(`tar -xf '${zipPath}' -C '${parentDir}'`);
      }
    }

    if (!fs.existsSync(extractDir)) {
      throw new Error(`Extracted folder not found: ${extractDir}. Check zip structure.`);
    }
    fs.renameSync(extractDir, VCPKG_ROOT);
    fs.unlinkSync(zipPath);
    
    exec(`git init`, VCPKG_ROOT);
    exec(`git config user.email ci@local`, VCPKG_ROOT);
    exec(`git config user.name ci`, VCPKG_ROOT);
    exec(`git add -A`, VCPKG_ROOT);
    exec(`git commit -m "init vcpkg snapshot"`, VCPKG_ROOT);

    const output = spawn('git', ['rev-parse', 'HEAD'], { 
      cwd: VCPKG_ROOT
    });

    if (!output) {
      throw new Error('Failed to get git revision (spawn returned null)');
    }
    
    const baseline = output.toString().trim();
    const workspace = getEnv("GITHUB_WORKSPACE") || process.cwd();
    const vcpkgJsonPath = path.join(workspace, 'vcpkg.json');

    if (!fs.existsSync(vcpkgJsonPath)) {
      throw new Error(`vcpkg.json not found at ${vcpkgJsonPath}`);
    }

    const content = JSON.parse(fs.readFileSync(vcpkgJsonPath, 'utf-8'));
    content['builtin-baseline'] = baseline;
    
    fs.writeFileSync(vcpkgJsonPath, JSON.stringify(content, null, 2) + '\n', 'utf-8');

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
