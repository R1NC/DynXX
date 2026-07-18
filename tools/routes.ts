const buildTargetMap: Record<string, string> = {
    android: "android",
    harmonyos: "ohos",
    ohos: "ohos",
    linux: "linux",
    wasm: "wasm",
    webassembly: "wasm",
    windows: "windows",
    win: "windows",
    ios: "ios",
    macos: "macos",
    mac: "macos",
};

const commandMap: Record<string, string> = {
    "setup:llvm": "find-llvm.ts",
    "setup:vcpkg": "setup-vcpkg.ts",
    "build:android": "build/build-Android.ts",
    "build:ohos": "build/build-OHOS.ts",
    "build:linux": "build/build-Linux.ts",
    "build:wasm": "build/build-WASM.ts",
    "build:windows": "build/build-Windows.ts",
    "build:ios": "build/build-iOS.ts",
    "build:macos": "build/build-macOS.ts",
    "gen:doc": "gen-doc.ts",
};

function printUsage(): void {
    console.error("Usage:");
    console.error("  npm run setup:llvm");
    console.error("  npm run setup:vcpkg");
    console.error("  npm run build:<android|ohos|linux|wasm|windows|ios|macos> -- [--debug] [--test] [--coverage]");
    console.error("  npm run gen:doc");
}

type ResolvedCommand = {
    command: string;
    scriptArgs: string[];
};

function resolveCommand(args: string[]): ResolvedCommand | null {
    const action = (args[0] || "").toLowerCase();
    const value = (args[1] || "").toLowerCase();

    if (!action) return null;

    if (action === "setup") {
        if (value === "llvm" || value === "setup-llvm" || value === "setup:llvm") {
            return { command: "setup:llvm", scriptArgs: args.slice(2) };
        }
        return { command: "setup:vcpkg", scriptArgs: args.slice(1) };
    }

    if (action === "setup-llvm" || action === "setup:llvm") {
        return { command: "setup:llvm", scriptArgs: args.slice(1) };
    }

    if (action === "setup-vcpkg" || action === "setup:vcpkg") {
        return { command: "setup:vcpkg", scriptArgs: args.slice(1) };
    }

    if (action === "build") {
        const target = buildTargetMap[value];
        return target ? { command: `build:${target}`, scriptArgs: args.slice(2) } : null;
    }

    if (action.startsWith("build:") || action.startsWith("build.")) {
        const targetName = action.slice(6);
        const target = buildTargetMap[targetName];
        return target ? { command: `build:${target}`, scriptArgs: args.slice(1) } : null;
    }

    if (action === "gen" && value === "doc") {
        return { command: "gen:doc", scriptArgs: args.slice(2) };
    }

    if (action === "doc" || action === "gendoc" || action === "gen-doc") {
        return { command: "gen:doc", scriptArgs: args.slice(1) };
    }

    if (commandMap[action]) {
        return { command: action, scriptArgs: args.slice(1) };
    }

    const target = buildTargetMap[action];
    return target ? { command: `build:${target}`, scriptArgs: args.slice(1) } : null;
}

function getResolvedCommand(): ResolvedCommand {
    const resolved = resolveCommand(process.argv.slice(2));
    if (resolved != null) {
        return resolved;
    }
    printUsage();
    process.exit(1);
    throw new Error("Unreachable");
}

async function main(): Promise<void> {
    const { command, scriptArgs } = getResolvedCommand();
    const scriptFile = commandMap[command];
    if (!scriptFile) {
        printUsage();
        process.exit(1);
        throw new Error("Unreachable");
    }

    const previousArgv = process.argv;
    process.argv = [previousArgv[0] ?? "node", scriptFile, ...scriptArgs];

    try {
        await import(new URL(scriptFile, import.meta.url).href);
    } finally {
        process.argv = previousArgv;
    }
}

void main();
