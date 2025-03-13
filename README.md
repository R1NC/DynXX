# NGenXX

A cross-platform framework based on modern C++ that also supports JS and Lua.

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

- ❔ : Unknown;
- ⭕ : Not supported;
- ❌ : With compiling error;
- 🛠️ : Under developing;
- ⁉️ : With running error;
- ✔️ : Running well.

### Unsupported Features in WebAssembly

- Load Lua script with file;(Will trigger a prompt window)
- [C/C++ callback JS function in async thread][2].

## Requirements

- C99, C++23;
- Clang 15+;
- CMake 3.10+;
- Xcode 14.0+, for iOS & macOS;
- Android Studio(or IntelliJ IDEA) with NDK r25+;
- DevEco Studio with Native SDK, for HarmonyOS;
- [Emscripten][1], for WebAssembly.

## C++ New Features Compatibility

| Feature | Standard | Unsupported Platforms |
| :-- | :--: | :--: |
| [std::format][12] | C++20 | iOS 16.3-, macOS 13.3-, HarmonyOS Next; |
| [std::ranges][13] | C++20 | iOS 16.3-, macOS 13.3-, HarmonyOS Next; |
| [std::ranges][14] | C++23 | *Not clear, maybe all?* |

## Dependencies

- [ada-url][3]: URL parser;(Requires `std::ranges` in C++20)
- [curl][4]: Networking;
- [cJSON][5]: JSON parser;
- [libuv][6]: Asynchronous I/O;
- [lua][7]: Lua runtime;
- [MMKV][8]: Key-value storage;
- [openssl][9]: Crypto;
- [quickjs][10]: JS runtime;
- [sqlite][11]: DB storage.

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
[12]: https://en.cppreference.com/w/cpp/utility/format/format
[13]: https://en.cppreference.com/w/cpp/ranges
[14]: https://en.cppreference.com/w/cpp/ranges/chunk_view