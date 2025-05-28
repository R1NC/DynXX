# NGenXX

[![Android Build](../../actions/workflows/Android.yml/badge.svg)](../../actions/workflows/Android.yml)
[![iOS Build](../../actions/workflows/iOS.yml/badge.svg)](../../actions/workflows/iOS.yml)
[![macOS Build](../../actions/workflows/macOS.yml/badge.svg)](../../actions/workflows/macOS.yml)
[![Linux Build](../../actions/workflows/Linux.yml/badge.svg)](../../actions/workflows/Linux.yml)
[![MSVC Build](../../actions/workflows/MSVC.yml/badge.svg)](../../actions/workflows/MSVC.yml)
[![MinGW Build](../../actions/workflows/MinGW.yml/badge.svg)](../../actions/workflows/MinGW.yml)
[![WASM Build](../../actions/workflows/WASM.yml/badge.svg)](../../actions/workflows/WASM.yml)

A cross-platform framework based on modern C++, supporting biz dev via Lua & JS.

> :point_right: The Rust version([NGenRS](https://github.com/R1NC/NGenRS)) is in works.
> <details>
> <summary>Why Rust?</summary>
> 
> * Guarantee memory safety by the compiler, not the programmer;
> * Powerful third-party dependency management tool - [Crates](https://crates.io/), no need to struggle with CMake;
> * First-class [WASM support](https://rustwasm.github.io/docs/book/), no other toolchain needed(Like [Emscripten](https://emscripten.org/));
> * Complete cross-platform support - [FFI](https://doc.rust-lang.org/nomicon/ffi.html), [CXX](https://cxx.rs/), [jni](https://docs.rs/jni/latest/jni/), [ojbc2](https://docs.rs/objc2/latest/objc2/), [ohos-rs](https://ohos.rs/).
> </details>

## Architecture

![Arch](/res/arch.svg)

## Progress

| | Android | iOS | HarmonyOS  | Windows | macOS | Linux | WASM |
| :-- | :--: | :--: |:--: |:--: | :--: | :--: |:--: |
| Lua Runtime |:heavy_check_mark:|:heavy_check_mark:|:heavy_check_mark:|:heavy_check_mark:|:heavy_check_mark:|:heavy_check_mark:|:heavy_check_mark:|
| JS Runtime |:heavy_check_mark:|:heavy_check_mark:|:heavy_check_mark:|:grey_exclamation:|:heavy_check_mark:|:heavy_check_mark:|:x:|
| Network |:heavy_check_mark:|:heavy_check_mark:|:heavy_check_mark:|:heavy_check_mark:|:heavy_check_mark:|:heavy_check_mark:|:heavy_exclamation_mark:|
| SQLite |:heavy_check_mark:|:heavy_check_mark:|:heavy_check_mark:|:heavy_check_mark:|:heavy_check_mark:|:heavy_check_mark:|:grey_exclamation:|
| Key-Value Store |:heavy_check_mark:|:heavy_check_mark:|:heavy_check_mark:|:heavy_check_mark:|:heavy_check_mark:|:heavy_check_mark:|:grey_exclamation:|
| JSON Codec |:heavy_check_mark:|:heavy_check_mark:|:heavy_check_mark:|:grey_exclamation:|:heavy_check_mark:|:heavy_check_mark:|:grey_exclamation:|
| Crypto |:heavy_check_mark:|:heavy_check_mark:|:heavy_check_mark:|:grey_exclamation:|:heavy_check_mark:|:heavy_check_mark:|:grey_exclamation:|
| Zip |:heavy_check_mark:|:heavy_check_mark:|:grey_exclamation:|:grey_exclamation:|:heavy_check_mark:|:heavy_check_mark:|:grey_exclamation:|
| Device Info |:heavy_check_mark:|:heavy_check_mark:|:heavy_check_mark:|:hammer:|:heavy_check_mark:|:hammer:|:interrobang:|
| Log |:heavy_check_mark:|:heavy_check_mark:|:heavy_check_mark:|:grey_exclamation:|:heavy_check_mark:|:heavy_check_mark:|:interrobang:|

- :interrobang: : Unknown;
- :o: : Not supported;
- :hammer: : Under developing;
- :x: : Failed to compile;
- :grey_exclamation: : Succeed to compile, not tested;
- :heavy_exclamation_mark: : With running error;
- :heavy_check_mark: : Tested.

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

| Feature                             | Standard | Unsupported Platforms              |
|:------------------------------------|:---------|:-----------------------------------|
| [std::from_chars][19] for integer   | C++17    | iOS 16.5-, macOS 13.4-;            |
| [std::from_chars][19] for float     | C++17    | All;                               |
| [std::format][12]                   | C++20    | iOS 16.3-, macOS 13.3-, HarmonyOS; |
| [std::ranges][13]                   | C++20    | iOS 16.3-, macOS 13.3-, HarmonyOS; |
| [std::ranges::views::enumerate][20] | C++23    | All;                               |
| [std::ranges::views::chunk][14]     | C++23    | All;                               |

<details>

<summary>Reference:</summary>

- [C++ compiler support - cppreference.com][22];
- [C++ Language Support - Xcode - Apple Developer][21];

</details>

## Dependencies

- [ada-url][3]: URL parser;(Requires `std::ranges` in C++20)
- [android-ifaddrs][16]: The missing `ifaddrs` for Android;
- [curl][4]: Networking;
- [cJSON][5]: JSON codec;
- [iOS-cmake][18]: CMake toolchain for Apple platforms;
- [libuv][6]: Asynchronous I/O;
- [lua][7]: Lua runtime;
- [MMKV][8]: Key-value storage;
- [openssl][9]: Crypto;
- [quickjs][10]: JS runtime;
- [spdlog][15]: File logging;
- [sqlite][11]: DB storage;
- [zlib][17]: Compression.

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
[16]: https://github.com/morristech/android-ifaddrs
[17]: https://github.com/madler/zlib
[18]: https://github.com/leetal/ios-cmake
[19]: https://en.cppreference.com/w/cpp/utility/from_chars
[20]: https://en.cppreference.com/w/cpp/ranges/enumerate_view
[21]: https://developer.apple.com/xcode/cpp/
[22]: https://en.cppreference.com/w/cpp/compiler_support
