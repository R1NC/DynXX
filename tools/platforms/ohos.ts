import { resolve } from 'node:path';

import { isExistingDirectory } from '../utils.js';

export function getOhosLlvmHome(sdkRoot: string): string {
  const llvmDir = resolve(sdkRoot, 'native', 'llvm', 'bin');
  if (isExistingDirectory(llvmDir)) {
    return llvmDir;
  }
  throw new Error(`Cannot determine HarmonyOS llvm root (bin) under ${resolve(sdkRoot, 'native', 'llvm')}`);
}
