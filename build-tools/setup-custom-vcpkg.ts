import * as fs from 'node:fs';
import * as path from 'node:path';
import { platform } from 'node:os';
import { execSafe, execShell, getHomeDir } from './utils.js';

const IS_WINDOWS = platform() === 'win32';
const VCPKG_URL = "https://github.com/R1NC/vcpkg/archive/refs/heads/dev.zip";

const VCPKG_ROOT = process.env.RUNNER_TEMP 
  ? path.join(process.env.RUNNER_TEMP, 'vcpkg') 
  : path.join(process.cwd(), 'temp_vcpkg');

async function main() {
  console.log(`Starting vcpkg setup at: ${VCPKG_ROOT}`);

  try {
    if (fs.existsSync(VCPKG_ROOT)) {
      console.log('Cleaning existing vcpkg directory...');
      fs.rmSync(VCPKG_ROOT, { recursive: true, force: true });
    }
    const parentDir = path.dirname(VCPKG_ROOT);
    fs.mkdirSync(parentDir, { recursive: true });

    const zipPath = path.join(parentDir, 'vcpkg-dev.zip');
    const extractDir = path.join(parentDir, 'vcpkg-dev');

    console.log(`Downloading from ${VCPKG_URL}...`);
    if (IS_WINDOWS) {
      execShell(`powershell -NoProfile -Command "Invoke-WebRequest -Uri '${VCPKG_URL}' -OutFile '${zipPath}'"`);
    } else {
      execShell(`curl -L -o '${zipPath}' '${VCPKG_URL}'`);
    }

    console.log('Unzipping...');
    if (IS_WINDOWS) {
      execShell(`powershell -NoProfile -Command "Expand-Archive -LiteralPath '${zipPath}' -DestinationPath '${parentDir}' -Force"`);
    } else {
      try {
        execShell(`unzip -q '${zipPath}' -d '${parentDir}'`);
      } catch {
        execShell(`tar -xf '${zipPath}' -C '${parentDir}'`);
      }
    }

    if (!fs.existsSync(extractDir)) {
      throw new Error(`Extracted folder not found: ${extractDir}. Check zip structure.`);
    }
    fs.renameSync(extractDir, VCPKG_ROOT);
    fs.unlinkSync(zipPath);

    console.log('Initializing Git repository...');
    
    execShell(`git init`, VCPKG_ROOT);
    execShell(`git config user.email ci@local`, VCPKG_ROOT);
    execShell(`git config user.name ci`, VCPKG_ROOT);
    execShell(`git add -A`, VCPKG_ROOT);
    execShell(`git commit -m "init vcpkg snapshot"`, VCPKG_ROOT);

    const output = execSafe('git', ['rev-parse', 'HEAD'], { 
      cwd: VCPKG_ROOT, 
      captureOutput: true 
    });

    if (!output) {
      throw new Error('Failed to get git revision (execSafe returned null)');
    }
    
    const baseline = output.toString().trim();
    console.log(`Baseline commit: ${baseline}`);

    const workspace = process.env.GITHUB_WORKSPACE || process.cwd();
    const vcpkgJsonPath = path.join(workspace, 'vcpkg.json');

    if (!fs.existsSync(vcpkgJsonPath)) {
      throw new Error(`vcpkg.json not found at ${vcpkgJsonPath}`);
    }

    const content = JSON.parse(fs.readFileSync(vcpkgJsonPath, 'utf-8'));
    content['builtin-baseline'] = baseline;
    
    fs.writeFileSync(vcpkgJsonPath, JSON.stringify(content, null, 2) + '\n', 'utf-8');
    console.log('vcpkg.json updated.');

    console.log(`::set-output name=VCPKG_ROOT::${VCPKG_ROOT}`); 

    console.log('Bootstrapping vcpkg...');
    const bootstrapScript = path.join(VCPKG_ROOT, IS_WINDOWS ? 'bootstrap-vcpkg.bat' : 'bootstrap-vcpkg.sh');
    
    if (!IS_WINDOWS) {
      fs.chmodSync(bootstrapScript, 0o755);
    }
    
    execShell(IS_WINDOWS ? `"${bootstrapScript}"` : `"${bootstrapScript}"`, VCPKG_ROOT);
    
    console.log('vcpkg setup complete!');

    const githubEnv = process.env.GITHUB_ENV;
    if (githubEnv) {
      fs.appendFileSync(githubEnv, `CI_VCPKG_HOME=${VCPKG_ROOT}\n`);
      console.log(`Exported CI_VCPKG_HOME=${VCPKG_ROOT} to GITHUB_ENV`);
    } else {
      console.warn('GITHUB_ENV not found. Set CI_VCPKG_HOME manually.');
      console.log(`CI_VCPKG_HOME=${VCPKG_ROOT}`);
    }

  } catch (error: any) {
    console.error('Fatal error during vcpkg setup:', error.message);
    process.exit(1);
  }
}

main();
