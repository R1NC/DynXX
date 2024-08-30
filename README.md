# EngineXX

A cross-platform C/C++ framework.

![Arch](/res/arch.png)

### Supported Platforms:

| Platform        | Build              | Demo               |
| :-------------- | :----------------: | :----------------: |
| Android         | :heavy_check_mark: | :heavy_check_mark: |
| iOS             | :heavy_check_mark: | :heavy_check_mark: |
| HarmonyOS(NEXT) | :heavy_check_mark: | :heavy_check_mark: |
| Windows         | :heavy_check_mark: | :heavy_check_mark: |
| macOS           | :heavy_check_mark: | :heavy_check_mark: |
| Linux           | :heavy_check_mark: | :heavy_check_mark: |
| WebAssembly     | :heavy_check_mark: | :heavy_check_mark: |

### Unsupported Features in WebAssembly:

* Load Lua script with file;(Will trigger a prompt window.)
* Network request with cURL;(Domain can not be resolved, WebSocket timeout.)
* [C/C++ callback JS function in async thread][2];

### Requirements:

* C99, C++11;
* Clang;
* CMake;
* Xcode, for iOS & macOS;
* Android Studio(or IntelliJ IDEA) with NDK;
* DevEco Studio NEXT with Native SDK, for HarmonyOS;
* [Emscripten][1], for WebAssembly;

[1]: https://emscripten.org/
[2]: https://github.com/emscripten-core/emscripten/issues/16567
