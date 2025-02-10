# NGenXX

A cross-platform framework based on C/C++ that also supports Lua and JS.

![Arch](/res/arch.svg)

## Progress

| | Android | iOS | HarmonyOS  | Windows | macOS | Linux | WebAssembly |
| :-- | :--: | :--: |:--: |:--: | :--: | :--: |:--: |
| Lua Runtime |✔️|✔️|✔️|✔️|✔️|✔️|✔️|
| JS Runtime |✔️|✔️|✔️|❌|✔️|❌|🛠️|
| Network |✔️|✔️|✔️|✔️|✔️|✔️|⁉️|
| SQLite |✔️|✔️|✔️|✔️|✔️|✔️|🛠️|
| Key-Value Store |✔️|✔️|✔️|✔️|✔️|✔️|🛠️|
| JSON |✔️|✔️|✔️|🛠️|✔️|✔️|🛠️|
| Crypto |✔️|✔️|✔️|🛠️|✔️|✔️|🛠️|
| Zip |✔️|✔️|🛠️|🛠️|✔️|✔️|🛠️|
| Device Info |✔️|✔️|✔️|🛠️|✔️|🛠️|❔|

- ❔ : Unknown.
- ⭕ : Not supported.
- ❌ : With compiling error.
- 🛠️ : Under developing.
- ⁉️ : With running error.
- ✔️ : Running well.

### Unsupported Features in WebAssembly

- Load Lua script with file;(Will trigger a prompt window.)
- [C/C++ callback JS function in async thread][2];

## Requirements

- C99, C++20;
- Clang;
- CMake;
- Xcode, for iOS & macOS;
- Android Studio(or IntelliJ IDEA) with NDK;
- DevEco Studio with Native SDK, for HarmonyOS;
- [Emscripten][1], for WebAssembly;

## Dependencies

- [ada-url][3]: URL parser;
- [curl][4]: Networking;
- [cJSON][5]: JSON parser;
- [libuv][6]: Asynchronous I/O;
- [lua][7]: Lua runtime;
- [MMKV][8]: Key-value storage;
- [openssl][9]: Crypto;
- [quickjs][10]: JS runtime;
- [sqlite][11]: DB storage;

[1]: https://emscripten.org/docs/getting_started/downloads.html#sdk-download-and-install
[2]: https://github.com/emscripten-core/emscripten/issues/16567
[3]: https://github.com/ada-url/ada
[4]: https://github.com/curl/curl
[5]: https://github.com/DaveGamble/cJSON
[6]: https://github.com/libuv/libuv
[7]: https://github.com/lua/lua
[8]: https://github.com/Tencent/MMKV
[9]: https://github.com/openssl/openssl
[10]: https://github.com/bellard/quickjs
[11]: https://github.com/sqlite/sqlite
