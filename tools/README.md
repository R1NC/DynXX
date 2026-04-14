# DynXX Tools

This directory provides cross-platform setup and build scripts, with a unified entrypoint via `npm run ...`.

## 1. Environment Setup

- Initialize LLVM toolchain: `npm run setup:llvm`
- Initialize vcpkg: `npm run setup:vcpkg`

## 2. Build And Test Flow

- Basic format: `npm run build:<platform> -- [args]`
- Supported platforms: `android` / `ohos` / `linux` / `wasm` / `windows` / `ios` / `macos`

### Build

#### Command And Args

- Build type args: `--debug` / `-d` for `Debug`, `--release` / `-r` for `Release` (default)
- `--debug` and `--release` are mutually exclusive; passing both exits with an error

```bash
npm run build:windows -- --release
npm run build:linux -- --debug
```

#### Outputs

- Headers: `<buildFolder>/output/include` (installed by CMake `--component headers`)
- Static libraries: `<buildFolder>/output/<abi>/lib` (for example `libDynXX.a`, `DynXX.lib`, `libDynXX-All.a`, `DynXX-All.lib`)
- Executables: `<buildFolder>/output/<abi>/bin` (for example `qjsc`, `qjsc.exe`; on macOS, app bundles such as `qjsc.app` are also placed under this directory)

### Configure Only

- Use `--config` to clear CMake cache and regenerate configuration without compiling.

```bash
npm run build:windows -- --config --release
```

### Test

#### Command And Args

- `--test`: enable test-related build and test flow (platform support depends on each build script)
- `--coverage`: enable coverage flow (platform support depends on each build script, usually with `--test`)

```bash
npm run build:linux -- --debug --test
npm run build:linux -- --debug --test --coverage
```

#### Outputs

- GTest XML: `<buildFolder>/output/test/gtest/gtest-report.xml`
- GTest HTML: `<buildFolder>/output/test/gtest/gtest-report.html`
- Coverage summary: `<buildFolder>/output/test/coverage/coverage-summary.txt`
- Coverage HTML: `<buildFolder>/output/test/coverage/html/index.html`
- Coverage LCOV: `<buildFolder>/output/test/coverage/lcov.info`

## 3. API Docs

#### Command And Args

- Generate API docs: `npm run gen:doc`

#### Outputs

- Generated site root: `build.Docs/site`
- Entry page: `build.Docs/site/index.html`
