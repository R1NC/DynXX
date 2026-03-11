import * as fs from 'node:fs';
import * as path from 'node:path';
import * as https from 'node:https';
import { execSync } from 'node:child_process';
import { pipeline } from 'node:stream';
import { promisify } from 'node:util';

const asyncPipeline = promisify(pipeline);

const VCPKG_URL = "https://github.com/R1NC/vcpkg/archive/refs/heads/dev.zip";
const VCPKG_ROOT = process.env.RUNNER_TEMP 
  ? path.join(process.env.RUNNER_TEMP, 'vcpkg') 
  : path.join(process.cwd(), 'temp_vcpkg'); // Fallback for local run

async function downloadFile(url: string, dest: string): Promise<void> {
  return new Promise((resolve, reject) => {
    const file = fs.createWriteStream(dest);
    https.get(url, (response) => {
      if (response.statusCode === 301 || response.statusCode === 302) {
        // Handle redirect manually for simple cases, or use a library like 'follow-redirects'
        // For GitHub raw/release links, usually direct, but archive links might redirect
        const redirectUrl = response.headers.location;
        if (redirectUrl) {
          https.get(redirectUrl, (res) => {
             asyncPipeline(res, file).then(resolve).catch(reject);
          }).on('error', reject);
          return;
        }
      }
      asyncPipeline(response, file).then(resolve).catch(reject);
    }).on('error', reject);
  });
}

function unzipFile(zipPath: string, destDir: string): void {
  const zipAbsPath = path.resolve(zipPath);
  const destAbsPath = path.resolve(destDir);

  if (process.platform === 'win32') {
    const psZipPath = zipAbsPath.replace(/'/g, "''");
    const psDestPath = destAbsPath.replace(/'/g, "''");
    execSync(
      `powershell -NoProfile -NonInteractive -ExecutionPolicy Bypass -Command "Expand-Archive -LiteralPath '${psZipPath}' -DestinationPath '${psDestPath}' -Force"`,
      { stdio: 'inherit' }
    );
    return;
  }

  try {
    execSync(`unzip -q "${zipAbsPath}" -d "${destAbsPath}"`, { stdio: 'inherit' });
  } catch {
    execSync(`tar -xf "${zipAbsPath}" -C "${destAbsPath}"`, { stdio: 'inherit' });
  }
}

function runGitCommand(cwd: string, args: string[]): void {
  execSync(`git ${args.join(' ')}`, { cwd, stdio: 'inherit' });
}

async function main() {
  console.log(`Starting vcpkg setup at: ${VCPKG_ROOT}`);

  try {
    if (fs.existsSync(VCPKG_ROOT)) {
      console.log('Cleaning existing vcpkg directory...');
      fs.rmSync(VCPKG_ROOT, { recursive: true, force: true });
    }
    fs.mkdirSync(VCPKG_ROOT, { recursive: true });

    const vcpkgParentDir = path.dirname(VCPKG_ROOT);
    const zipPath = path.join(vcpkgParentDir, 'vcpkg-dev.zip');
    console.log(`Downloading from ${VCPKG_URL}...`);
    await downloadFile(VCPKG_URL, zipPath);

    console.log('Unzipping...');
    unzipFile(zipPath, vcpkgParentDir);

    const sourceDir = path.join(vcpkgParentDir, 'vcpkg-dev');
    
    if (!fs.existsSync(sourceDir)) {
      throw new Error(`Extracted folder not found: ${sourceDir}. Check zip structure.`);
    }
    
    fs.renameSync(sourceDir, VCPKG_ROOT);
    fs.unlinkSync(zipPath);
    console.log('Extracted and moved.');

    console.log('Initializing Git repository...');
    runGitCommand(VCPKG_ROOT, ['init']);
    runGitCommand(VCPKG_ROOT, ['config', 'user.email', '"ci@local"']);
    runGitCommand(VCPKG_ROOT, ['config', 'user.name', '"ci"']);
    runGitCommand(VCPKG_ROOT, ['add', '-A']);
    runGitCommand(VCPKG_ROOT, ['commit', '-m', '"init vcpkg snapshot"']);

    const baseline = execSync('git rev-parse HEAD', { 
      cwd: VCPKG_ROOT, 
      encoding: 'utf-8' 
    }).trim();
    console.log(`Baseline commit: ${baseline}`);

    const vcpkgJsonPath = path.join(process.env.GITHUB_WORKSPACE || process.cwd(), 'vcpkg.json');

    if (!fs.existsSync(vcpkgJsonPath)) {
      throw new Error(`vcpkg.json not found at ${vcpkgJsonPath}`);
    }

    const content = JSON.parse(fs.readFileSync(vcpkgJsonPath, 'utf-8'));
    content['builtin-baseline'] = baseline;
    
    fs.writeFileSync(vcpkgJsonPath, JSON.stringify(content, null, 2) + '\n', 'utf-8');
    console.log('vcpkg.json updated.');

    console.log(`::set-output name=VCPKG_ROOT::${VCPKG_ROOT}`); 

    console.log('Bootstrapping vcpkg...');
    const isWindows = process.platform === 'win32';
    const bootstrapScript = isWindows 
      ? path.join(VCPKG_ROOT, 'bootstrap-vcpkg.bat')
      : path.join(VCPKG_ROOT, 'bootstrap-vcpkg.sh');
    
    execSync(`"${bootstrapScript}"`, { stdio: 'inherit' });
    
    console.log('vcpkg setup complete!');

    const githubEnv = process.env.GITHUB_ENV;
    if (githubEnv) {
      const envContent = `CI_VCPKG_HOME=${VCPKG_ROOT}\n`;
      fs.appendFileSync(githubEnv, envContent);
      console.log(`Exported CI_VCPKG_HOME=${VCPKG_ROOT} to GITHUB_ENV`);
    } else {
      console.warn('GITHUB_ENV not found (running locally?). Set CI_VCPKG_HOME manually.');
      console.log(`CI_VCPKG_HOME=${VCPKG_ROOT}`);
    }

  } catch (error) {
    console.error('Fatal error during vcpkg setup:', error);
    process.exit(1);
  }
}

main();
