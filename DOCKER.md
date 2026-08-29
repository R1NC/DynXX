# Docker / Podman Build Environment

Windows/macOS hosts lack cross-compile toolchains for Linux/Android/OHOS/WASM. The [Dockerfile](Dockerfile) provides four environment images — mount your source and verify compilation on any of these targets in two steps: `podman build` creates the image (once; toolchains are baked in), `podman run` mounts the source and builds.

Podman is fully Dockerfile-compatible, so the Dockerfile needs no changes — `docker build`/`docker run` map 1:1 to `podman build`/`podman run` with the same flags. First-time setup: install Podman Desktop (bundles the CLI), which creates and starts its VM on first launch (or from the CLI: `podman machine init && podman machine start`).

> **`<WSL-GATEWAY>`** in the commands below is your machine's gateway IP into the Podman VM (the WSL virtual switch `vEthernet (WSL ...)`), assigned per machine — substitute your own value (e.g. `http://<WSL-GATEWAY>:7890`). Find it with:
>
> ```powershell
> (Get-NetIPAddress -InterfaceAlias 'vEthernet (WSL)*' -AddressFamily IPv4).IPAddress
> ```
>
> It stays stable across Podman VM restarts, but changes if the WSL network is recreated (see Notes).

## Linux

Create the image (once) — the base stage clones vcpkg from GitHub, so pass the proxy when GitHub is unreachable. All four targets share these base layers, so this build (or any target's first build) caches them for the rest (see Notes):

```bash
podman build --build-arg HTTPS_PROXY=http://<WSL-GATEWAY>:7890 --target dynxx-linux -t dynxx-linux .
```

The proxy address is the Podman VM's gateway into the host. Unlike Docker Desktop, Podman's `host.docker.internal`/`host.containers.internal` do not forward host ports (connections are refused), so use the gateway IP instead (see Notes).

Build and run tests (artifacts land in `build.Linux/`):

```bash
podman run --rm -it -v "${PWD}:/workspace" -v /workspace/tools/node_modules dynxx-linux bash -lc "
  cd tools && npm ci && npm run build:linux -- --test"
```

## Android

Create the image (once) — sdkmanager pulls its components from dl.google.com, so build with the proxy (it also covers the base vcpkg clone on a fresh cache):

```bash
podman build --build-arg HTTPS_PROXY=http://<WSL-GATEWAY>:7890 --target dynxx-android -t dynxx-android .
```

Build the arm64-v8a library and package the AAR:

```bash
podman run --rm -it -v "${PWD}:/workspace" -v /workspace/tools/node_modules dynxx-android bash -lc "
  cd tools && npm ci && npm run build:android -- --test"
```

Before the run, temporarily move your host `platforms/Android/local.properties` aside (AGP prefers `sdk.dir` over the SDK baked into the image).

## OHOS

Create the image (once) — the SDK comes from the huaweicloud mirror (reachable directly), but the build command must still pass the same proxy args as the other targets: cache keys include the build environment, so omitting them misses the base layer cache and re-runs the vcpkg clone without a proxy:

```bash
podman build --build-arg HTTPS_PROXY=http://<WSL-GATEWAY>:7890 --target dynxx-ohos -t dynxx-ohos .
```

Build the arm64 static library (artifacts land in `build.OHOS/`):

```bash
podman run --rm -it -v "${PWD}:/workspace" -v /workspace/tools/node_modules dynxx-ohos bash -lc "
  cd tools && npm ci && npm run build:harmonyos -- --test"
```

## WASM

Create the image (once) — the emsdk repo clone pulls from GitHub, so add the proxy in China (the toolchain binaries come from storage.googleapis.com, reachable directly; the same proxy also covers the base vcpkg clone when the cache is fresh):

```bash
podman build --build-arg HTTPS_PROXY=http://<WSL-GATEWAY>:7890 --target dynxx-wasm -t dynxx-wasm .
```

Produce `DynXX.wasm`/`DynXX.js`/`DynXX.html` (artifacts land in `build.WASM/`):

```bash
podman run --rm -it -v "${PWD}:/workspace" -v /workspace/tools/node_modules dynxx-wasm bash -lc "
  cd tools && npm ci && npm run build:wasm"
```

## Proxies and networking

Dependency downloads (vcpkg clone, sdkmanager, emsdk) happen during the image build — the proxy is active only for build-time RUN steps and cleared before the stage finishes, so container runs stay proxy-free (a baked proxy would make libcurl route every request through it and bypass DNS overrides).

* **Build args must be identical across targets**: layer cache keys include the build environment, so a target built without the proxy args misses the base cache and re-runs the vcpkg clone (which fails without a proxy on the GFW). Always pass the same `--build-arg HTTPS_PROXY`/`HTTP_PROXY` values, even for OHOS.
* **Why the gateway IP?** `host.docker.internal`/`host.containers.internal` resolve inside the VM but do not forward host loopback ports (Podman's gvproxy, unlike Docker Desktop's, refuses those connections). The proxy must listen on the WSL gateway (`<WSL-GATEWAY>`) — enable Allow LAN on FlClash, or forward a loopback-only proxy: `netsh interface portproxy add v4tov4 listenaddress=<WSL-GATEWAY> listenport=7890 connectaddress=127.0.0.1 connectport=7890`. If the gateway IP changes (WSL network recreated), re-run the lookup at the top and update the addresses.
* **Build-time downloads per target**: base images default to the DaoCloud mirror (`docker.m.daocloud.io`; outside China pass `--build-arg REGISTRY=docker.io`). The shared `dynxx-linux` base stage clones vcpkg from GitHub, so on a fresh cache every target needs the proxy; once cached, only Android (sdkmanager from dl.google.com) and WASM (emsdk repo clone) still do — OHOS's SDK comes from the huaweicloud mirror.
* **Run-time downloads**: vcpkg downloads on a fresh container need `--env https_proxy=http://<WSL-GATEWAY>:7890 --env http_proxy=http://<WSL-GATEWAY>:7890`; the Gradle JVM ignores `http_proxy`, so Android additionally needs `--env "GRADLE_OPTS=-Dhttp.proxyHost=<WSL-GATEWAY> -Dhttp.proxyPort=7890 -Dhttps.proxyHost=<WSL-GATEWAY> -Dhttps.proxyPort=7890"`. Test runs must NOT have the proxy (DNS-override tests) — use a two-step pattern: step 1 builds with the proxy envs, step 2 runs `-- --test` without them plus `--env DYNXX_GTEST_FILTER=-DynXXDeviceTestSuite.*` (containers have no DMI data, so the device manufacturer/model tests fail).

## Troubleshooting

* **Gradle fails silently** — `GRADLE_OPTS` (-D flags) differ from `gradle.properties` `org.gradle.jvmargs`, so Gradle forks a single-use daemon whose output is not echoed to the console. Read the daemon log: `podman run --rm -v dynxx-gradle-home:/root/.gradle dynxx-android bash -lc "tail -100 /root/.gradle/daemon/*/*.log"`.
* **Gradle daemon dies on a small VM** — the Android daemon needs its `-Xmx2048m` heap; a 2 GiB VM (the `podman machine init --memory 2048` default, which writes `[wsl2] memory=2147483648` into `%UserProfile%\.wslconfig`) kills it silently. Raise it in `.wslconfig` (`memory=8589934592`), then `wsl --shutdown` + `podman machine start` — `podman machine set --memory` does not work for WSL machines.
* **`Permission denied` on mounted dirs** — directories created under Docker (or via a Windows git checkout) carry NTFS EA Unix metadata (uid 1000, mode 0755) that the 9p mount maps to `nobody:nogroup r-x`, so the container cannot write or delete them (vcpkg lock files, Gradle `fileHashes.lock`, AGP `Unable to delete directory .../intermediates/...`). `chmod` inside the container or WSL fails too; delete the directory on the Windows side (`Remove-Item -Recurse -Force`) and let the container recreate it. Affects `build.*/`, `platforms/Android/.gradle`, `platforms/Android/DynXX-lib/build`, `DynXX-lib/.cxx`.

## Notes

* **Toolchain versions are frozen in the image**: vcpkg (`dev` branch, `/opt/vcpkg`), Node 24, emsdk 3.1.65, OHOS SDK 6.0.0.48, Android SDK/NDK — controlled by Dockerfile ARGs; rebuild the image to upgrade (vcpkg is a rolling branch).
* **`-v /workspace/tools/node_modules`**: anonymous volume that keeps the container's platform-specific `npm install` from overwriting your host `node_modules`.
* **Faster repeat builds**: named volumes keep caches across `--rm` runs — `-v dynxx-vcpkg-cache:/root/.cache/vcpkg` (binary cache), `-v dynxx-vcpkg-downloads:/opt/vcpkg/downloads` (toolchain tarballs), and for Android `-v dynxx-gradle-home:/root/.gradle` (wrapper dist, dependency cache, daemon logs).
* **Apple Silicon**: add `--platform linux/amd64` to both build and run.
* **Windows / macOS**: build natively (`npm run build:windows` / `npm run build:macos`) — no containers needed.
* **Artifact ownership**: builds run as root inside the container; `chown`/`chmod` on the host if needed.
