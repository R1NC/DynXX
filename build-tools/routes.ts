import { dirname, join } from "node:path";
import { fileURLToPath } from "node:url";
import { exec } from "./utils.js";

const buildTargetMap: Record<string, string> = {
    android: "android",
    harmonyos: "harmonyos",
    ohos: "harmonyos",
    linux: "linux",
    wasm: "wasm",
    windows: "windows",
    win: "windows",
    ios: "ios",
    macos: "macos",
    mac: "macos",
};

const commandMap: Record<string, string> = {
    "setup:vcpkg": "setup-vcpkg.ts",
    "build:android": "build-Android.ts",
    "build:harmonyos": "build-HarmonyOS.ts",
    "build:linux": "build-Linux.ts",
    "build:wasm": "build-WASM.ts",
    "build:windows": "build-Windows.ts",
    "build:ios": "build-iOS.ts",
    "build:macos": "build-macOS.ts",
    "gen:doc": "gen-doc.ts",
};

function printUsage(): void {
    console.error("Usage:");
    console.error("  npm run setup:vcpkg");
    console.error("  npm run build:android");
    console.error("  npm run gen:doc");
    console.error("  npm run build:<android|harmonyos|linux|wasm|windows|ios|macos>");
}

function resolveCommand(args: string[]): string | null {
    const action = (args[0] || "").toLowerCase();
    const value = (args[1] || "").toLowerCase();

    if (!action) return null;

    if (action === "setup" || action === "setup-vcpkg" || action === "setup:vcpkg") {
        return "setup:vcpkg";
    }

    if (action === "build") {
        const target = buildTargetMap[value];
        return target ? `build:${target}` : null;
    }

    if (action === "gen" && value === "doc") {
        return "gen:doc";
    }

    if (action === "doc" || action === "gendoc" || action === "gen-doc") {
        return "gen:doc";
    }

    if (commandMap[action]) {
        return action;
    }

    const target = buildTargetMap[action];
    return target ? `build:${target}` : null;
}

const command = resolveCommand(process.argv.slice(2));
if (!command) {
    printUsage();
    process.exit(1);
}

const scriptFile = commandMap[command];
if (!scriptFile) {
    printUsage();
    process.exit(1);
}

const toolsDir = dirname(fileURLToPath(import.meta.url));
const tsxCli = join(toolsDir, "node_modules", "tsx", "dist", "cli.mjs");
exec(`node "${tsxCli}" ${scriptFile}`, toolsDir);
