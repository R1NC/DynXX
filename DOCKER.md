# Docker Build Environment

Windows/macOS hosts lack cross-compile toolchains for Linux/Android/OHOS/WASM. The [Dockerfile](Dockerfile) provides four environment images — mount your source and verify compilation on any of these targets in two steps: `docker build` creates the image (once; toolchains are baked in), `docker run` mounts the source and builds.

## Linux

Create the image (once):

```bash
docker build --target dynxx-linux -t dynxx-linux .
```

Build and run tests (artifacts land in `build.Linux/`):

```bash
docker run --rm -it -v "${PWD}:/workspace" -v /workspace/tools/node_modules dynxx-linux bash -lc "
  cd tools && npm ci && npm run build:linux -- --test"
```

## Android

Create the image (once):

```bash
docker build --target dynxx-android -t dynxx-android .
```

Build the arm64-v8a library and package the AAR:

```bash
docker run --rm -it -v "${PWD}:/workspace" -v /workspace/tools/node_modules dynxx-android bash -lc "
  cd tools && npm ci && npm run build:android -- --test"
```

Before the run, temporarily move your host `platforms/Android/local.properties` aside (AGP prefers `sdk.dir` over the SDK baked into the image).

## OHOS

Create the image (once):

```bash
docker build --target dynxx-ohos -t dynxx-ohos .
```

Build the arm64 static library (artifacts land in `build.OHOS/`):

```bash
docker run --rm -it -v "${PWD}:/workspace" -v /workspace/tools/node_modules dynxx-ohos bash -lc "
  cd tools && npm ci && npm run build:harmonyos -- --test"
```

## WASM

Create the image (once) — emsdk downloads from GitHub, so add the proxy in China:

```bash
docker build --build-arg HTTPS_PROXY=http://host.docker.internal:7890 --target dynxx-wasm -t dynxx-wasm .
```

Produce `DynXX.wasm`/`DynXX.js`/`DynXX.html` (artifacts land in `build.WASM/`):

```bash
docker run --rm -it -v "${PWD}:/workspace" -v /workspace/tools/node_modules dynxx-wasm bash -lc "
  cd tools && npm ci && npm run build:wasm"
```

## Notes

* **Toolchain versions are frozen in the image**: vcpkg (`dev` branch, `/opt/vcpkg`), Node 24, emsdk 3.1.65, OHOS SDK 6.0.0.48, Android SDK/NDK. Versions are controlled by the Dockerfile ARGs — to upgrade, change the ARG and rebuild the image (vcpkg is a rolling branch; rebuilding picks up the latest).
* **China network**: add `--build-arg REGISTRY=docker.m.daocloud.io` if Docker Hub is unreachable; `--build-arg HTTPS_PROXY=http://host.docker.internal:7890` if GitHub is (the proxy is baked into the image and inherited by `docker run`).
* **`-v /workspace/tools/node_modules`**: anonymous volume that keeps the container's platform-specific `npm install` from overwriting your host `node_modules`.
* **Faster repeat builds**: add `-v dynxx-vcpkg-cache:/root/vcpkg-binary-cache` (dependency binary cache); Android additionally `-v dynxx-gradle-cache:/root/.gradle`.
* **Apple Silicon**: add `--platform linux/amd64` to both build and run.
* **Windows / macOS**: build natively (`npm run build:windows` / `npm run build:macos`) — no Docker needed.
* **Artifact ownership**: builds run as root inside the container; `chown`/`chmod` on the host if needed.
