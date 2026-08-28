# Podman Build Environment

Windows/macOS hosts lack cross-compile toolchains for Linux/Android/OHOS/WASM. The [Dockerfile](Dockerfile) provides four environment images — mount your source and verify compilation on any of these targets in two steps: `podman build` creates the image (once; toolchains are baked in), `podman run` mounts the source and builds.

Podman is fully Dockerfile-compatible, so the Dockerfile needs no changes — `docker build`/`docker run` map 1:1 to `podman build`/`podman run` with the same flags. First-time setup: install Podman Desktop (bundles the CLI), which creates and starts its VM on first launch (or from the CLI: `podman machine init && podman machine start`).

## Linux

Create the image (once) — the base stage clones vcpkg from GitHub, so pass the proxy when GitHub is unreachable. All four targets share these base layers, so this build (or any target's first build) caches them for the rest (see Notes):

```bash
podman build --build-arg HTTPS_PROXY=http://host.docker.internal:7890 --target dynxx-linux -t dynxx-linux .
```

Build and run tests (artifacts land in `build.Linux/`):

```bash
podman run --rm -it -v "${PWD}:/workspace" -v /workspace/tools/node_modules dynxx-linux bash -lc "
  cd tools && npm ci && npm run build:linux -- --test"
```

## Android

Create the image (once) — sdkmanager pulls its components from dl.google.com, so build with the proxy (it also covers the base vcpkg clone on a fresh cache):

```bash
podman build --build-arg HTTPS_PROXY=http://host.docker.internal:7890 --target dynxx-android -t dynxx-android .
```

Build the arm64-v8a library and package the AAR:

```bash
podman run --rm -it -v "${PWD}:/workspace" -v /workspace/tools/node_modules dynxx-android bash -lc "
  cd tools && npm ci && npm run build:android -- --test"
```

Before the run, temporarily move your host `platforms/Android/local.properties` aside (AGP prefers `sdk.dir` over the SDK baked into the image).

## OHOS

Create the image (once) — the SDK comes from the huaweicloud mirror (reachable directly), so no proxy is needed once the `dynxx-linux` base layers are cached; on a fresh cache (first build) add `--build-arg HTTPS_PROXY=http://host.docker.internal:7890` for the base vcpkg clone:

```bash
podman build --target dynxx-ohos -t dynxx-ohos .
```

Build the arm64 static library (artifacts land in `build.OHOS/`):

```bash
podman run --rm -it -v "${PWD}:/workspace" -v /workspace/tools/node_modules dynxx-ohos bash -lc "
  cd tools && npm ci && npm run build:harmonyos -- --test"
```

## WASM

Create the image (once) — emsdk downloads from GitHub, so add the proxy in China (the same proxy also covers the base vcpkg clone when the cache is fresh):

```bash
podman build --build-arg HTTPS_PROXY=http://host.docker.internal:7890 --target dynxx-wasm -t dynxx-wasm .
```

Produce `DynXX.wasm`/`DynXX.js`/`DynXX.html` (artifacts land in `build.WASM/`):

```bash
podman run --rm -it -v "${PWD}:/workspace" -v /workspace/tools/node_modules dynxx-wasm bash -lc "
  cd tools && npm ci && npm run build:wasm"
```

## Notes

* **Toolchain versions are frozen in the image**: vcpkg (`dev` branch, `/opt/vcpkg`), Node 24, emsdk 3.1.65, OHOS SDK 6.0.0.48, Android SDK/NDK. Versions are controlled by the Dockerfile ARGs — to upgrade, change the ARG and rebuild the image (vcpkg is a rolling branch; rebuilding picks up the latest).
* **China network**: base images already default to the DaoCloud mirror (`docker.m.daocloud.io`) — outside China pass `--build-arg REGISTRY=docker.io`; add `--build-arg HTTPS_PROXY=http://host.docker.internal:7890` if GitHub is unreachable. **All four targets share the `dynxx-linux` base stage, which clones vcpkg from GitHub — on a fresh cache (first build) every target therefore needs the proxy.** Once the base layers are cached, only OHOS is proxy-free (SDK from the huaweicloud mirror); Android still needs it (sdkmanager pulls components from dl.google.com) and so does WASM (emsdk install pulls toolchains from GitHub releases). The proxy is active only during the image build (apt/SDK downloads) and cleared from the final image, so container runs stay proxy-free — a baked proxy would make libcurl route every request through it and bypass DNS overrides. If dependency downloads during `podman run` need a proxy, add `--env https_proxy=http://host.docker.internal:7890`. Podman's VM provides the `host.docker.internal` alias like Docker Desktop; fall back to `host.containers.internal` if the connection fails.
* **`-v /workspace/tools/node_modules`**: anonymous volume that keeps the container's platform-specific `npm install` from overwriting your host `node_modules`.
* **Faster repeat builds**: add `-v dynxx-vcpkg-cache:/root/vcpkg-binary-cache` (dependency binary cache); Android additionally `-v dynxx-gradle-cache:/root/.gradle`.
* **Apple Silicon**: add `--platform linux/amd64` to both build and run.
* **Windows / macOS**: build natively (`npm run build:windows` / `npm run build:macos`) — no containers needed.
* **Artifact ownership**: builds run as root inside the container; `chown`/`chmod` on the host if needed.
