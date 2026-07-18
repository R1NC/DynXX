import { readdirSync } from 'node:fs';
import { resolve } from 'node:path';

import { isExistingDirectory } from '../utils.js';

export function getAndroidLlvmHome(ndkHome: string): string {
  const platformPaths: Record<string, string[]> = {
    'darwin': ['darwin-arm64', 'darwin-x86_64'],
    'linux': ['linux-x86_64'],
    'win32': ['windows-x86_64']
  };

  const candidates = platformPaths[process.platform];
  if (!candidates) {
    throw new Error(`Unsupported platform for NDK lookup: ${process.platform}`);
  }

  const prebuiltDir = resolve(ndkHome, 'toolchains', 'llvm', 'prebuilt');

  for (const tag of candidates) {
    const candidateDir = resolve(prebuiltDir, tag, 'bin');
    if (isExistingDirectory(candidateDir)) {
      return candidateDir;
    }
  }

  if (isExistingDirectory(prebuiltDir)) {
    for (const entry of readdirSync(prebuiltDir)) {
      const candidateDir = resolve(prebuiltDir, entry, 'bin');
      if (isExistingDirectory(candidateDir)) {
        return candidateDir;
      }
    }
  }

  throw new Error(`Cannot determine NDK llvm root (bin) under ${prebuiltDir}`);
}
