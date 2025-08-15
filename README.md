# NGenXX

[<img src="https://img.shields.io/github/actions/workflow/status/R1NC/NGenXX/Android-Mac.yml?branch=main&label=Build%20for%20Android%20on%20macOS&logo=android&logoColor=3DDC84"/>][41] [<img src="https://img.shields.io/github/actions/workflow/status/R1NC/NGenXX/Android-Ubuntu.yml?branch=main&label=Build%20for%20Android%20on%20Ubuntu&logo=android&logoColor=3DDC84"/>][32]  
[<img src="https://img.shields.io/github/actions/workflow/status/R1NC/NGenXX/iOS.yml?branch=main&label=Build%20for%20iOS&logo=Apple&logoColor=white"/>][33] [<img src="https://img.shields.io/github/actions/workflow/status/R1NC/NGenXX/macOS.yml?branch=main&label=Build%20for%20macOS&logo=Apple&logoColor=white"/>][34]  
[<img src="https://img.shields.io/github/actions/workflow/status/R1NC/NGenXX/HarmonyOS-Mac.yml?branch=main&label=Build%20for%20HarmonyOS%20on%20macOS&logo=Huawei&logoColor=FF0000"/>][43]  
[<img src="https://img.shields.io/github/actions/workflow/status/R1NC/NGenXX/Win-MSVC.yml?branch=main&label=Build%20for%20Windows%20with%20MSVC&logo=gitforwindows&logoColor=0078D4"/>][35] [<img src="https://img.shields.io/github/actions/workflow/status/R1NC/NGenXX/Win-MinGW.yml?branch=main&label=Build%20for%20Windows%20with%20MinGW&logo=gitforwindows&logoColor=0078D4"/>][36]  
[<img src="https://img.shields.io/github/actions/workflow/status/R1NC/NGenXX/WASM-Mac.yml?branch=main&label=Build%20for%20WASM%20on%20macOS&logo=WebAssembly&logoColor=654FF0"/>][38] [<img src="https://img.shields.io/github/actions/workflow/status/R1NC/NGenXX/WASM-Ubuntu.yml?branch=main&label=Build%20for%20WASM%20on%20Ubuntu&logo=WebAssembly&logoColor=654FF0"/>][42]  
[<img src="https://img.shields.io/github/actions/workflow/status/R1NC/NGenXX/Linux-Ubuntu.yml?branch=main&label=Build%20for%20Linux%20on%20Ubuntu&logo=Linux&logoColor=FBB726"/>][37]  

A cross-platform framework based on modern C++, supporting biz dev via Lua & JS.

> :point_right: The Rust version([NGenRS][31]) is in works.
> <details>
> <summary>Why Rust?</summary>
> 
> * Guarantee memory safety by the compiler, not the programmer;
> * Powerful third-party dependency management tool - [Crates][23], no need to struggle with CMake;
> * First-class [WASM support][24], no other toolchain needed(Like [Emscripten][27]);
> * Complete cross-platform support - [FFI][25], [CXX][26], [jni][28], [ojbc2][29], [ohos-rs][30].
> </details>

## Architecture

![Arch](/res/arch.svg)

## Progress

| | Android | iOS | HarmonyOS  | Windows | macOS | Linux | WASM |
| :-- | :--: | :--: | :--: | :--: | :--: | :--: | :--: |
| Lua Runtime |:heavy_check_mark:|:heavy_check_mark:|:heavy_check_mark:|:heavy_check_mark:|:heavy_check_mark:|:heavy_check_mark:|:o:|
| JS Runtime |:heavy_check_mark:|:heavy_check_mark:|:heavy_check_mark:|:grey_exclamation:|:heavy_check_mark:|:heavy_check_mark:|:o:|
| Network |:heavy_check_mark:|:heavy_check_mark:|:heavy_check_mark:|:heavy_check_mark:|:heavy_check_mark:|:heavy_check_mark:|:heavy_check_mark:|
| SQLite |:heavy_check_mark:|:heavy_check_mark:|:heavy_check_mark:|:heavy_check_mark:|:heavy_check_mark:|:heavy_check_mark:|:o:|
| Key-Value Store |:heavy_check_mark:|:heavy_check_mark:|:heavy_check_mark:|:heavy_check_mark:|:heavy_check_mark:|:heavy_check_mark:|:o:|
| JSON Codec |:heavy_check_mark:|:heavy_check_mark:|:heavy_check_mark:|:grey_exclamation:|:heavy_check_mark:|:heavy_check_mark:|:heavy_check_mark:|
| Crypto |:heavy_check_mark:|:heavy_check_mark:|:heavy_check_mark:|:grey_exclamation:|:heavy_check_mark:|:heavy_check_mark:|:heavy_check_mark:|
| Zip |:heavy_check_mark:|:heavy_check_mark:|:grey_exclamation:|:grey_exclamation:|:heavy_check_mark:|:heavy_check_mark:|:heavy_check_mark:|
| Device Info |:heavy_check_mark:|:heavy_check_mark:|:heavy_check_mark:|:hammer:|:heavy_check_mark:|:hammer:|:x:|
| Log |:heavy_check_mark:|:heavy_check_mark:|:heavy_check_mark:|:grey_exclamation:|:heavy_check_mark:|:heavy_check_mark:|:heavy_check_mark:|

* :grey_question: : Unknown;
* :x: : Not supported;
* :o: : Limited supported;
* :hammer: : Under developing;
* :interrobang: : Failed to compile;
* :grey_exclamation: : Succeed to compile, not tested;
* :heavy_exclamation_mark: : With running error;
* :heavy_check_mark: : Tested.

<details>

<summary>Unsupported Features in WASM:</summary>

* Load Lua script with file;(Will trigger a prompt window)
* [C/C++ callback JS function in async thread][2].

</details>

## C++ New Features Compatibility

| Feature                             | Standard | Unsupported Platforms              |
| :---------------------------------- | :------: | :--------------------------------- |
| [std::from_chars][19] for integer   | C++17    | iOS 16.5-, macOS 13.4-;            |
| [std::from_chars][19] for float     | C++17    | All;                               |
| [std::format][12]                   | C++20    | iOS 16.3-, macOS 13.3-, HarmonyOS; |
| [std::ranges][13]                   | C++20    | iOS 16.3-, macOS 13.3-, HarmonyOS; |
| [std::jthread][39]                  | C++20    | Apple Clang;                       |
| [std::ranges::views::enumerate][20] | C++23    | All;                               |
| [std::ranges::views::chunk][14]     | C++23    | All;                               |

<details>

<summary>Reference:</summary>

* [C++ compiler support - cppreference.com][22];
* [C++ Language Support - Xcode - Apple Developer][21];

</details>

## Dependencies

* [ada-url][3]: URL parser;(Requires `std::ranges` in C++20)
* [android-ifaddrs][16]: The missing `ifaddrs` for Android;
* [curl][4]: Networking;
* [cJSON][5]: JSON codec;
* [iOS-cmake][18]: CMake toolchain for Apple platforms;
* [json.lua][40]: JSON codec for Lua;
* [libuv][6]: Asynchronous I/O;
* [lua][7]: Lua runtime;
* [MMKV][8]: Key-value storage;
* [openssl][9]: Crypto;
* [quickjs][10]: JS runtime;
* [spdlog][15]: File logging;
* [sqlite][11]: DB storage;
* [zlib][17]: Compression.

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
[23]: https://crates.io/
[24]: https://rustwasm.github.io/docs/book/
[25]: https://doc.rust-lang.org/nomicon/ffi.html
[26]: https://cxx.rs/
[27]: https://emscripten.org/
[28]: https://docs.rs/jni/latest/jni/
[29]: https://docs.rs/objc2/latest/objc2/
[30]: https://ohos.rs/
[31]: https://github.com/R1NC/NGenRS
[32]: ../../actions/workflows/Android-Ubuntu.yml
[33]: ../../actions/workflows/iOS.yml
[34]: ../../actions/workflows/macOS.yml
[35]: ../../actions/workflows/Win-MSVC.yml
[36]: ../../actions/workflows/Win-MinGW.yml
[37]: ../../actions/workflows/Linux-Ubuntu.yml
[38]: ../../actions/workflows/WASM-Mac.yml
[39]: https://en.cppreference.com/w/cpp/thread/jthread.html
[40]: https://gist.github.com/tylerneylon/59f4bcf316be525b30ab
[41]: ../../actions/workflows/Android-Mac.yml
[42]: ../../actions/workflows/WASM-Ubuntu.yml
[43]: ../../actions/workflows/HarmonyOS-Mac.yml
