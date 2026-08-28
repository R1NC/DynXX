#
# DynXX Docker build environments for Linux-hosted targets (mirror the GitHub
# Actions ubuntu-latest runner toolchains):
#
# Targets:
#   dynxx-linux   - Linux x86_64: clang + ninja + vcpkg (CI-Linux-Ubuntu.yml)
#   dynxx-android - Android arm64-v8a: + JDK 17 + Android SDK/NDK r30 (CI-Android-Ubuntu.yml)
#   dynxx-ohos    - HarmonyOS arm64: + HarmonyOS SDK 6.0.0.48 (CI-OHOS-Ubuntu.yml)
#   dynxx-wasm    - WASM32: + Emscripten 3.1.65 (CI-WASM-Ubuntu.yml)
#
# Build an environment image (Podman-compatible; podman build/run replace the
# docker equivalents with the same flags):
#   podman build --target dynxx-linux -t dynxx-linux .
#   podman build --target dynxx-android -t dynxx-android .
#   podman build --target dynxx-ohos -t dynxx-ohos .
#   podman build --target dynxx-wasm -t dynxx-wasm .
# (Behind a firewall add --build-arg HTTPS_PROXY=... to the build - every
# target needs it on a fresh cache; once the base layers are cached only OHOS
# skips it. See DOCKER.md.)
#
# Base images default to the DaoCloud registry mirror (docker.m.daocloud.io)
# for China network stability; outside China, pass --build-arg REGISTRY=docker.io
# In regions where GitHub releases are unreachable, route build-time downloads
# through a local proxy: --build-arg HTTPS_PROXY=http://host.docker.internal:7890
# (the proxy env is baked for the build-time RUN steps only and cleared before
# the stage finishes, so container runs stay proxy-free - a baked proxy would
# make libcurl route every request through it and bypass CURLOPT_RESOLVE).
# All targets share the dynxx-linux base stage, which clones vcpkg from GitHub:
# on a fresh cache (first build) every target needs the proxy. Once the base
# layers are cached, only OHOS is proxy-free (huaweicloud mirror); Android
# (sdkmanager downloads from dl.google.com) and WASM (emsdk install) still do.
#
# Build the project with the source mounted from the host (artifacts land in build.*/).
# The toolchains are all baked into the image (clang/LLVM, Node, vcpkg at /opt/vcpkg,
# plus the per-target SDKs), so runs only need `npm ci && npm run build:<target>`:
#   Linux:
#     podman run --rm -it -v "${PWD}:/workspace" -v /workspace/tools/node_modules dynxx-linux bash -lc "
#       cd tools && npm ci && npm run build:linux -- --test"
#   Android (see DOCKER.md for the local.properties caveat):
#     podman run --rm -it -v "${PWD}:/workspace" -v /workspace/tools/node_modules dynxx-android bash -lc "
#       cd tools && npm ci && npm run build:android -- --test"
#   OHOS:
#     podman run --rm -it -v "${PWD}:/workspace" -v /workspace/tools/node_modules dynxx-ohos bash -lc "
#       cd tools && npm ci && npm run build:harmonyos -- --test"
#   WASM:
#     podman run --rm -it -v "${PWD}:/workspace" -v /workspace/tools/node_modules dynxx-wasm bash -lc "
#       cd tools && npm ci && npm run build:wasm"
#
# Notes:
#   - The anonymous volume -v <mount>/tools/node_modules keeps the container's
#     npm installs from overwriting the host platform's node_modules (esbuild is
#     platform-specific and would break the other side).
#   - vcpkg is baked into the image at /opt/vcpkg (git clone + bootstrap during the
#     image build; `dev` is a rolling branch, rebuild the image to update it).
#     setup:llvm/setup:vcpkg are CI-only steps (they export GITHUB_ENV) and are not
#     needed in containers. Dependency builds can be reused across runs via the
#     vcpkg binary cache under the home dir (~/vcpkg-binary-cache) when persisted
#     with `-v dynxx-vcpkg-cache:/root/vcpkg-binary-cache`.
#   - Build outputs are owned by root inside the container; adjust with chown/chmod
#     on the host if needed.

# Base image registry. Defaults to the DaoCloud mirror for China (Docker Hub is
# flaky there); outside China pass --build-arg REGISTRY=docker.io for the official
# registry.
ARG REGISTRY=docker.m.daocloud.io

# Optional proxy for build-time downloads (GitHub releases etc.); pass
# --build-arg HTTPS_PROXY=http://host.docker.internal:7890 behind a firewall.
# Active only during build-time RUN steps (see the ENV clears below); container
# runs are proxy-free so libcurl does not bypass DNS overrides.
ARG HTTP_PROXY=
ARG HTTPS_PROXY=
ARG NO_PROXY=localhost,127.0.0.1

############################## Linux ##############################
FROM ${REGISTRY}/library/ubuntu:24.04 AS dynxx-linux

# ARGs must precede the ENV that references them (they are only usable inside
# the stage that declares them).
ARG HTTP_PROXY
ARG HTTPS_PROXY
ARG NO_PROXY

ENV DEBIAN_FRONTEND=noninteractive \
    http_proxy=$HTTP_PROXY \
    https_proxy=$HTTPS_PROXY \
    no_proxy=$NO_PROXY

# Match CI (CI-Linux-Ubuntu.yml sets CC/CXX to clang): some deps (quickjs qjsc)
# only compile warning-free with clang, and vcpkg's ABI hashes must match CI.
ENV CC=clang CXX=clang++

# Same toolchain as the ubuntu-latest runner (Base-Setup.yml + CI-Linux-Ubuntu.yml):
# build-essential, cmake, clang, ninja-build, llvm (llvm-cov/llvm-profdata), git.
# zip/unzip are required by vcpkg's bootstrap-vcpkg.sh; pkg-config by vcpkg ports
# that run vcpkg_fixup_pkgconfig (e.g. ada-url); zlib1g-dev by the MMKV fetch
# (Checksum.h includes zlib.h) - preinstalled on the GitHub runner.
# clang-tools-18 provides clang-scan-deps, which CMake's Ninja generator requires
# for the Clang C++ module (P1689) probe; the unversioned name is symlinked so
# CMake 3.28 finds it next to clang in PATH. The same unversioned symlinks are
# made for llvm-cov/llvm-profdata - the --coverage flow resolves them via LLVM_HOME
# and the apt packages only ship the versioned (-18) names.
RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        build-essential \
        cmake \
        clang \
        clang-tools-18 \
        llvm \
        ninja-build \
        git \
        ca-certificates \
        curl \
        zip \
        unzip \
        pkg-config \
        zlib1g-dev \
    && ln -sf clang-scan-deps-18 /usr/bin/clang-scan-deps \
    && ln -sf llvm-cov-18 /usr/bin/llvm-cov \
    && ln -sf llvm-profdata-18 /usr/bin/llvm-profdata \
    && rm -rf /var/lib/apt/lists/*

# Node 24, same major as actions/setup-node@v6 (node-version: '24') in the CI.
# deb.nodesource.com is unreliable from behind the GFW (both direct and
# proxied connections stall or get TLS-reset), so install the official
# nodejs.org tarball from the huaweicloud mirror instead.
# (overridable with --build-arg NODE_VERSION=...).
ARG NODE_VERSION=24.20.0
RUN curl -fsSL -o /tmp/node.tar.xz \
        https://mirrors.huaweicloud.com/nodejs/v${NODE_VERSION}/node-v${NODE_VERSION}-linux-x64.tar.xz \
    && tar -xJf /tmp/node.tar.xz -C /usr/local --strip-components=1 \
    && rm /tmp/node.tar.xz \
    && node --version

# vcpkg is baked into the image instead of re-cloned per run: setup-vcpkg.ts
# follows CI's "fresh runner every time" semantics (deletes + full git clone into
# the mounted workspace - slow over the volume mount and repeats the download on
# every run). Pinning the clone in the image freezes the toolchain for
# reproducibility (dev is a rolling branch; rebuild the image to update).
# Needs GitHub reachable at build time (see the proxy args above). This is the
# one GitHub download shared by all four targets - on a fresh cache, run any
# target's build with the proxy first so the base layers get cached.
RUN git clone --branch dev https://github.com/rinc-xyz/vcpkg.git /opt/vcpkg \
    && /opt/vcpkg/bootstrap-vcpkg.sh

# CI_VCPKG_HOME/VCPKG_HOME point CMakePresets.json at the baked vcpkg toolchain
# ($env{VCPKG_HOME}); LLVM_HOME makes the --coverage flow find llvm-cov/llvm-profdata
# (setup:llvm's GITHUB_ENV export is CI-only and would not reach the build process).
# The build-time proxy envs are cleared here so the runtime image is proxy-free:
# libcurl reads http_proxy/https_proxy and would route every request through the
# proxy, where CURLOPT_RESOLVE (DNS overrides) does not apply. Pass
# --env https_proxy=... to podman run when dependency downloads need a proxy.
ENV CI_VCPKG_HOME=/opt/vcpkg \
    VCPKG_HOME=/opt/vcpkg \
    LLVM_HOME=/usr/bin \
    http_proxy= \
    https_proxy=

# Podman (rootless) maps the container user to the host user, so keep /workspace
# world-writable: outputs written by the container stay manageable from the host
# without chown. A bind mount shadows this directory at run time anyway - this
# only covers the case where /workspace is not mounted.
RUN mkdir -p /workspace && chmod 777 /workspace

WORKDIR /workspace

CMD ["bash"]

############################## Android ##############################
# Mirror of CI-Android-Ubuntu.yml on top of the Linux stage:
#   JDK 17 (openjdk here, temurin in CI - same language level),
#   Android SDK cmdline-tools + platform-tools, NDK r30 (version pinned by the
#   ANDROID_NDK_VERSION ARG below, same name as CI's ANDROID_NDK_VERSION env).
# platforms;android-37.0 / build-tools;37.0.0 / cmake;4.1.2 and the Gradle NDK
# (ANDROID_NDK_GRADLE_VERSION) are preinstalled via sdkmanager because AGP's
# auto-downloader cannot reach dl.google.com from behind the GFW (its Java
# HTTP stack ignores the baked proxy env and the SDK repo fetch fails
# silently); sdkmanager honors the env proxy, so the components bake in.
# Versions mirror platforms/Android/DynXX-lib/build.gradle.kts.
FROM dynxx-linux AS dynxx-android

# ARGs are scoped to the stage that declares them - re-declare the proxy ARGs
# here or $HTTP_PROXY expands empty in the ENV below (the base stage's ARGs do
# not carry over) and sdkmanager silently gets no proxy.
ARG HTTP_PROXY
ARG HTTPS_PROXY

# Same variable name as CI-Android-*.yml's ANDROID_NDK_VERSION env; bump the
# NDK version here (overridable with --build-arg ANDROID_NDK_VERSION=...).
ARG ANDROID_NDK_VERSION=30.0.16138531
# NDK used by Gradle's externalNativeBuild (build.gradle.kts ndkVersion),
# plus the SDK components AGP would otherwise auto-download on first build.
ARG ANDROID_NDK_GRADLE_VERSION=30.0.15729638
# API 37+ platforms are versioned "android-37.0" in the SDK repository (the
# legacy "android-37" id no longer exists).
ARG ANDROID_PLATFORM=android-37.0
ARG ANDROID_BUILD_TOOLS=37.0.0
ARG ANDROID_CMAKE=4.1.2

# sdkmanager downloads dl.google.com components - re-enable the build-time
# proxy (the base stage cleared it), then clear it again for the runtime image.
ENV http_proxy=$HTTP_PROXY \
    https_proxy=$HTTPS_PROXY

RUN apt-get update \
    && apt-get install -y --no-install-recommends openjdk-17-jdk-headless \
    && rm -rf /var/lib/apt/lists/*

# build-Android.ts reads CI_ANDROID_NDK_HOME first, then ANDROID_NDK_HOME.
ENV JAVA_HOME=/usr/lib/jvm/java-17-openjdk-amd64 \
    ANDROID_HOME=/opt/android-sdk \
    CI_ANDROID_NDK_HOME=/opt/android-sdk/ndk/${ANDROID_NDK_VERSION} \
    ANDROID_NDK_HOME=/opt/android-sdk/ndk/${ANDROID_NDK_VERSION}

# cmdline-tools zip: dl.google.com is behind the GFW, and of the Chinese
# mirrors only Tencent Cloud still syncs Google's android/repository (TUNA /
# Aliyun / Huawei all return 404), so pull it from there - reachable without
# the build-time proxy. sdkmanager below still fetches its components from
# dl.google.com and relies on the re-enabled proxy above.
RUN mkdir -p /opt/android-sdk/cmdline-tools \
    && curl -fsSL -o /tmp/cmdline-tools.zip \
        https://mirrors.cloud.tencent.com/AndroidSDK/commandlinetools-linux-11076708_latest.zip \
    && unzip -q /tmp/cmdline-tools.zip -d /opt/android-sdk/cmdline-tools \
    && mv /opt/android-sdk/cmdline-tools/cmdline-tools /opt/android-sdk/cmdline-tools/latest \
    && rm /tmp/cmdline-tools.zip \
    && yes | /opt/android-sdk/cmdline-tools/latest/bin/sdkmanager --licenses >/dev/null \
    && /opt/android-sdk/cmdline-tools/latest/bin/sdkmanager --install \
        "platform-tools" \
        "ndk;${ANDROID_NDK_VERSION}" \
        "ndk;${ANDROID_NDK_GRADLE_VERSION}" \
        "platforms;${ANDROID_PLATFORM}" \
        "build-tools;${ANDROID_BUILD_TOOLS}" \
        "cmake;${ANDROID_CMAKE}"

# All SDK components are baked - clear the proxy for the runtime image (the
# stage's ENV carried it through the sdkmanager RUN above).
ENV http_proxy= \
    https_proxy=

############################## OHOS ##############################
# Mirror of CI-OHOS-Ubuntu.yml: HarmonyOS SDK 6.0.0.48 (native linux-x64) from
# the huaweicloud mirror (versions pinned by the OHOS_SDK_* ARGs below).
# build-OHOS.ts reads CI_OHOS_SDK_ROOT first, then OHOS_SDK_ROOT (readCIEnv).
FROM dynxx-linux AS dynxx-ohos

# Same versions as CI-OHOS-*.yml's cache key and mirror URL; bump in one place
# (overridable with --build-arg OHOS_SDK_VERSION=... / OHOS_SDK_RELEASE=...).
ARG OHOS_SDK_VERSION=6.0.0.48
ARG OHOS_SDK_RELEASE=6.0.0.1-Release

RUN mkdir -p /opt/ohos-sdk \
    && curl -fL -o /tmp/ohos-sdk.tar.gz \
        https://mirrors.huaweicloud.com/harmonyos/os/${OHOS_SDK_RELEASE}/ohos-sdk-windows_linux-public.tar.gz \
    && tar -xzf /tmp/ohos-sdk.tar.gz -C /opt/ohos-sdk \
    && rm /tmp/ohos-sdk.tar.gz \
    && unzip -q /opt/ohos-sdk/ohos-sdk/linux/native-linux-x64-${OHOS_SDK_VERSION}-Release.zip -d /opt/ohos-sdk/ohos-sdk/linux \
    && rm /opt/ohos-sdk/ohos-sdk/linux/native-linux-x64-${OHOS_SDK_VERSION}-Release.zip

ENV CI_OHOS_SDK_ROOT=/opt/ohos-sdk/ohos-sdk/linux \
    OHOS_SDK_ROOT=/opt/ohos-sdk/ohos-sdk/linux

############################## WASM ##############################
# Mirror of CI-WASM-Ubuntu.yml: Emscripten 3.1.65 via emsdk (version pinned by
# the EMSDK_VERSION ARG below). The emsdk bootstrap downloads its toolchains
# from GitHub releases - pass --build-arg HTTPS_PROXY=... when GitHub is
# unreachable. Like Android's sdkmanager (dl.google.com), this stage still needs
# the proxy after the base layers are cached; only OHOS (huaweicloud mirror) is
# proxy-free.
FROM dynxx-linux AS dynxx-wasm

# Re-declare the proxy ARGs (stage-scoped, see the Android stage comment).
ARG HTTP_PROXY
ARG HTTPS_PROXY

# The base stage bakes CC=clang/CXX=clang++ for CI-Linux parity, but vcpkg's
# emscripten triplet only overrides the CMake compiler variables, not the
# CC/CXX env vars. make-based ports (openssl) read $ENV{CC} directly, so they
# must see emcc here - with the inherited clang, or unset (defaults to 'cc'),
# the wasm configure step picks the host compiler and fails on glibc headers.
# emsdk install downloads its toolchains from GitHub releases - re-enable the
# build-time proxy here (the base stage cleared it), then clear it again below.
ENV CC=emcc \
    CXX=em++ \
    http_proxy=$HTTP_PROXY \
    https_proxy=$HTTPS_PROXY

# Same version as CI-WASM-*.yml's setup-emsdk step; bump in one place
# (overridable with --build-arg EMSDK_VERSION=...).
ARG EMSDK_VERSION=3.1.65

RUN git clone --depth 1 --branch ${EMSDK_VERSION} https://github.com/emscripten-core/emsdk.git /opt/emsdk \
    && /opt/emsdk/emsdk install ${EMSDK_VERSION} \
    && /opt/emsdk/emsdk activate ${EMSDK_VERSION}

ENV CI_WASM_SDK_HOME=/opt/emsdk \
    WASM_SDK_HOME=/opt/emsdk \
    EMSDK=/opt/emsdk \
    PATH=/opt/emsdk/upstream/emscripten:/opt/emsdk:$PATH \
    http_proxy= \
    https_proxy=