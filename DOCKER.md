# Docker Build Environment

The root [Dockerfile](Dockerfile) provides reproducible build environments for DynXX's Linux and Android targets. Each mirrors the toolchain of its GitHub Actions `ubuntu-latest` runner — same compilers, same vcpkg ABI hashes — so builds inside the containers are toolchain-identical with CI builds.

| Target | Base Image | Toolchain | Status |
| :-- | :-- | :-- | :-- |
| `dynxx-linux` | Ubuntu 24.04 | clang + ninja + vcpkg (x64) | :heavy_check_mark: Verified (full flow, 227/229 tests) |
| `dynxx-android` | Ubuntu 24.04 + JDK 17 + Android SDK/NDK r30 | clang + ninja + vcpkg (arm64-v8a) + Gradle | :hammer: Image built, full flow pending |

> A Windows container target existed briefly but was removed: Windows hosts build natively, the CI `windows-latest` runner already provides a clean Windows environment, and Linux/macOS hosts cannot run Windows containers at all — leaving no audience for it.

## Requirements

* Docker Desktop (or any Docker engine);
* Network access to Docker Hub and GitHub (vcpkg clone + toolchain download);
* ~4 GB free disk space for the image and build artifacts.

## Build the image

```bash
docker build --target dynxx-linux -t dynxx-linux .
```

> In regions where Docker Hub is unreachable, prefix base images with a registry
> mirror, e.g. for China: `--build-arg REGISTRY=docker.m.daocloud.io`

## Run the full verification flow

```bash
docker run --rm -it -v "${PWD}:/workspace" -v /workspace/tools/node_modules -w /workspace dynxx-linux bash -lc "
  cd tools && npm ci && npm run setup:llvm && npm run setup:vcpkg \
  && npm run build:linux -- --test"
```

What it does: `npm ci` (deps) → `setup:llvm` (toolchain detection) → `setup:vcpkg` (clones vcpkg into `tools/temp_vcpkg` inside the mounted workspace) → `build:linux -- --test` (CMake configure + vcpkg manifest install + build + ctest). Build artifacts land in `build.Linux/`.

## Running on Apple Silicon (arm64 Mac)

Docker Desktop runs Linux containers through its built-in VM, so the image itself works on any Mac. However, the build scripts hardcode the `x64` vcpkg triplet ([build-Linux.ts](tools/build/build-Linux.ts)) to match the x64 CI runner — an arm64 container would install x64 packages that cannot execute. Force the amd64 platform instead (enable **Rosetta** in Docker Desktop for near-native speed; QEMU emulation works too, just slower):

```bash
docker build --platform linux/amd64 --target dynxx-linux -t dynxx-linux .
```

```bash
docker run --platform linux/amd64 --rm -it -v "${PWD}:/workspace" -v /workspace/tools/node_modules -w /workspace dynxx-linux bash -lc "
  cd tools && npm ci && npm run setup:llvm && npm run setup:vcpkg \
  && npm run build:linux -- --test"
```

Both `build` and `run` need the `--platform linux/amd64` flag (applies to both `dynxx-linux` and `dynxx-android`).

> This path follows Docker's standard amd64-emulation behavior, but has not been verified on real Apple Silicon hardware — treat the first run as a smoke test.

## Android build (`dynxx-android`)

Same flow as Linux, plus the Gradle AAR packaging:

```bash
docker build --target dynxx-android -t dynxx-android .
```

```bash
docker run --rm -it -v "${PWD}:/workspace" -v /workspace/tools/node_modules -w /workspace dynxx-android bash -lc "
  cd tools && npm ci && npm run setup:llvm && npm run setup:vcpkg \
  && npm run build:android -- --test"
```

* The C++ library is built by CMake + vcpkg (`arm64-v8a`, NDK's own clang), then `gradlew :DynXX-lib:assembleRelease` packages the AAR into `build.Android/`;
* `platforms;android-37`, `build-tools;37.0.0` and `cmake;4.1.2` are **not** baked into the image — AGP auto-downloads them on first build (licenses are pre-accepted), exactly like the CI runner;
* The mounted workspace must not contain a host-specific `platforms/Android/local.properties` with an `sdk.dir` (it is gitignored, so CI checkouts never have one) — AGP prefers `sdk.dir` over `ANDROID_HOME`; temporarily rename it on the host before the run;
* Gradle's distribution and dependency downloads live in `/root/.gradle` — persist with `-v dynxx-gradle-cache:/root/.gradle` to speed up repeat builds.

## Notes

* The anonymous volume `-v <mount>/tools/node_modules` keeps the container's platform-specific `npm install` (esbuild) from overwriting the host's `node_modules`;
* vcpkg is re-cloned on every run; persist the binary cache with `-v dynxx-vcpkg-cache:/root/vcpkg-binary-cache` to speed up repeat builds;
* Build outputs are owned by root inside the container; adjust ownership with `chown`/`chmod` on the host if needed.
