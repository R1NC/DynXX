#
# DynXX Docker build environments (mirror the GitHub Actions runner toolchains)
#
# Targets:
#   dynxx-linux   - Ubuntu 24.04 (same as ubuntu-latest runner): clang + ninja + vcpkg toolchain
#   dynxx-android - dynxx-linux + JDK 17 + Android SDK cmdline-tools/platform-tools
#                   + NDK r30 (same as CI-Android-Ubuntu.yml); AGP auto-downloads the
#                   remaining SDK components (platforms/build-tools/cmake) on first build
#
# Build an environment image:
#   docker build --target dynxx-linux -t dynxx-linux .
#   docker build --target dynxx-android -t dynxx-android .
#
# In regions where Docker Hub is unreachable, prefix base images with a registry
# mirror, e.g. for China: --build-arg REGISTRY=docker.m.daocloud.io
#
# Build the project with the source mounted from the host (artifacts land in build.*/):
#   Linux:
#     docker run --rm -it -v "${PWD}:/workspace" -v /workspace/tools/node_modules -w /workspace dynxx-linux bash -lc "
#       cd tools && npm ci && npm run setup:llvm && npm run setup:vcpkg \
#       && npm run build:linux -- --test"
#   Android (see DOCKER.md for the local.properties caveat):
#     docker run --rm -it -v "${PWD}:/workspace" -v /workspace/tools/node_modules -w /workspace dynxx-android bash -lc "
#       cd tools && npm ci && npm run setup:llvm && npm run setup:vcpkg \
#       && npm run build:android -- --test"
#
# Notes:
#   - The anonymous volume -v <mount>/tools/node_modules keeps the container's
#     npm installs from overwriting the host platform's node_modules (esbuild is
#     platform-specific and would break the other side).
#   - vcpkg is re-cloned on every run (tools/setup-vcpkg.ts removes it first), so the
#     first build after `npm ci` downloads the toolchain; later builds reuse the
#     vcpkg binary cache under the home dir (~/vcpkg-binary-cache) when persisted
#     with `-v dynxx-vcpkg-cache:/root/vcpkg-binary-cache`.
#   - Build outputs are owned by root inside the container; adjust with chown/chmod
#     on the host if needed.

# Base image registry; override for regions where Docker Hub is unreachable,
# e.g. --build-arg REGISTRY=docker.m.daocloud.io
ARG REGISTRY=docker.io

############################## Linux ##############################
FROM ${REGISTRY}/library/ubuntu:24.04 AS dynxx-linux

ENV DEBIAN_FRONTEND=noninteractive

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
# CMake 3.28 finds it next to clang in PATH.
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
    && rm -rf /var/lib/apt/lists/*

# Node 24, same major as actions/setup-node@v6 (node-version: '24') in the CI.
RUN curl -fsSL https://deb.nodesource.com/setup_24.x | bash - \
    && apt-get install -y --no-install-recommends nodejs \
    && rm -rf /var/lib/apt/lists/*

# setup-vcpkg.ts clones vcpkg into tools/temp_vcpkg inside the mounted workspace
# (its RUNNER_TEMP/GITHUB_ENV machinery is CI-only); bake the var into the image
# so container commands need no manual export (CMakePresets.json resolves the
# vcpkg toolchain via $env{VCPKG_HOME}). Placed last to keep the layer cache
# warm (the var is consumed at runtime, not during the build).
ENV CI_VCPKG_HOME=/workspace/tools/temp_vcpkg

WORKDIR /workspace

CMD ["bash"]

############################## Android ##############################
# Mirror of CI-Android-Ubuntu.yml on top of the Linux stage:
#   JDK 17 (openjdk here, temurin in CI - same language level),
#   Android SDK cmdline-tools + platform-tools, NDK r30 (30.0.15729638).
# platforms;android-37 / build-tools;37.0.0 / cmake;4.1.2 are NOT
# preinstalled on purpose: AGP auto-downloads them on first build
# (licenses accepted below), exactly as it does on the CI runner.
FROM dynxx-linux AS dynxx-android

RUN apt-get update \
    && apt-get install -y --no-install-recommends openjdk-17-jdk-headless \
    && rm -rf /var/lib/apt/lists/*

# build-Android.ts reads CI_ANDROID_NDK_HOME first, then ANDROID_NDK_HOME.
ENV JAVA_HOME=/usr/lib/jvm/java-17-openjdk-amd64 \
    ANDROID_HOME=/opt/android-sdk \
    CI_ANDROID_NDK_HOME=/opt/android-sdk/ndk/30.0.15729638 \
    ANDROID_NDK_HOME=/opt/android-sdk/ndk/30.0.15729638

RUN mkdir -p /opt/android-sdk/cmdline-tools \
    && curl -fsSL -o /tmp/cmdline-tools.zip \
        https://dl.google.com/android/repository/commandlinetools-linux-11076708_latest.zip \
    && unzip -q /tmp/cmdline-tools.zip -d /opt/android-sdk/cmdline-tools \
    && mv /opt/android-sdk/cmdline-tools/cmdline-tools /opt/android-sdk/cmdline-tools/latest \
    && rm /tmp/cmdline-tools.zip \
    && yes | /opt/android-sdk/cmdline-tools/latest/bin/sdkmanager --licenses >/dev/null \
    && /opt/android-sdk/cmdline-tools/latest/bin/sdkmanager --install "platform-tools" "ndk;30.0.15729638"