# NGenXX

A cross-platform framework based on modern C++, supporting biz dev via Lua & JS.

> :point_right: The Rust version([NGenRS](https://github.com/R1NC/NGenRS)) is in works.
> <details>
> <summary>Why Rust?</summary>
> 
> * Guarantee memory safety by the compiler, not the programmer;
> * Powerful third-party dependency management tool - [Crates](https://crates.io/), no need to struggle with CMake;
> * First-class [WASM support](https://rustwasm.github.io/docs/book/), no other toolchain needed(Like [Emscripten](https://emscripten.org/));
> * Complete cross-platform support - [FFI](https://doc.rust-lang.org/nomicon/ffi.html), [CXX](https://cxx.rs/), [jni](https://docs.rs/jni/latest/jni/), [ojbc2](https://docs.rs/objc2/latest/objc2/), [ohos-rs](https://ohos.rs/).

## Architecture

![Arch](/res/arch.svg)

## Progress

| | Android | iOS | HarmonyOS  | Windows | macOS | Linux | WebAssembly |
| :-- | :--: | :--: |:--: |:--: | :--: | :--: |:--: |
| Lua Runtime |✔️|✔️|✔️|✔️|✔️|✔️|✔️|
| JS Runtime |✔️|✔️|✔️|❌|✔️|❌|🛠️|
| Network |✔️|✔️|✔️|✔️|✔️|✔️|⁉️|
| SQLite |✔️|✔️|✔️|✔️|✔️|✔️|🛠️|
| Key-Value Store |✔️|✔️|✔️|✔️|✔️|✔️|🛠️|
| JSON Codec |✔️|✔️|✔️|🛠️|✔️|✔️|🛠️|
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
| [std::ranges::chunk_view][14] | C++23 | *Not clear, maybe all?* |

## Dependencies

- [ada-url][3]: URL parser;(Requires `std::ranges` in C++20)
- [curl][4]: Networking;
- [cJSON][5]: JSON codec;
- [libuv][6]: Asynchronous I/O;
- [lua][7]: Lua runtime;
- [MMKV][8]: Key-value storage;
- [openssl][9]: Crypto;
- [quickjs][10]: JS runtime;
- [spdlog][15]: File logging;
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
[15]: https://github.com/gabime/spdlog
