import { existsSync } from 'node:fs';
import { dirname } from 'node:path';

import { setEnv, spawn } from '../utils.js';

export function getMsvcToolsHome(): string {
  const result = spawn("where.exe", ["lib.exe"], { allowFailure: true });
  if (result) {
    const libPath = result.toString().trim().split(/\r?\n/, 1)[0];
    if (libPath && existsSync(libPath)) {
      const toolsHome = dirname(libPath);
      setEnv("MSVC_TOOLS_HOME", toolsHome);
      return toolsHome;
    }
  }
  throw new Error("Cannot determine MSVC_TOOLS_HOME from lib.exe in PATH");
}
