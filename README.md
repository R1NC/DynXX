# DynXX

[<img src="https://img.shields.io/sonar/quality_gate/R1NC_DynXX/main?server=https%3A%2F%2Fsonarcloud.io&logo=sonar&label=Sonar%20-%20Quality%20Gate"/>][16]
[<img src="https://img.shields.io/sonar/violations/R1NC_DynXX/main?server=https%3A%2F%2Fsonarcloud.io&format=long&logo=sonar&label=Sonar%20-%20Violations"/>][25]
 
[<img src="https://img.shields.io/github/actions/workflow/status/R1NC/DynXX/CI-Android-macOS.yml?branch=main&label=CI%20-%20Android&logo=android&logoColor=3DDC84"/>][26]
[<img src="https://img.shields.io/github/actions/workflow/status/R1NC/DynXX/CI-iOS.yml?branch=main&label=CI%20-%20iOS&logo=Apple&logoColor=white"/>][33]
[<img src="https://img.shields.io/github/actions/workflow/status/R1NC/DynXX/CI-HarmonyOS-macOS.yml?branch=main&label=CI%20-%20HarmonyOS&logo=Huawei&logoColor=FF0000"/>][24]       
[<img src="https://img.shields.io/github/actions/workflow/status/R1NC/DynXX/CI-Windows.yml?branch=main&label=CI%20-%20Windows&logo=gitforwindows&logoColor=0078D4"/>][35]
[<img src="https://img.shields.io/github/actions/workflow/status/R1NC/DynXX/CI-macOS.yml?branch=main&label=CI%20-%20macOS&logo=Apple&logoColor=white"/>][34]
[<img src="https://img.shields.io/github/actions/workflow/status/R1NC/DynXX/CI-Linux-Ubuntu.yml?branch=main&label=CI%20-%20Linux&logo=Linux&logoColor=FBB726"/>][30]      
[<img src="https://img.shields.io/github/actions/workflow/status/R1NC/DynXX/CI-WASM-macOS.yml?branch=main&label=CI%20-%20WASM&logo=WebAssembly&logoColor=654FF0"/>][29] 

A cross-platform framework based on modern C++, supporting biz dev via Lua & JS.

> :point_right: The Rust version([DynRS][31]) is in works.


## Architecture

<img src="/res/arch.svg" width="555px"/>

## Progress

| | Android | iOS | HarmonyOS  | Windows | macOS | Linux | WASM |
| :-- | :--: | :--: | :--: | :--: | :--: | :--: | :--: |
| Lua Runtime |:heavy_check_mark:|:heavy_check_mark:|:heavy_check_mark:|:heavy_check_mark:|:heavy_check_mark:|:heavy_check_mark:|:o:|
| JS Runtime |:heavy_check_mark:|:heavy_check_mark:|:heavy_check_mark:|:heavy_check_mark:|:heavy_check_mark:|:heavy_check_mark:|:o:|
| Network |:heavy_check_mark:|:heavy_check_mark:|:heavy_check_mark:|:heavy_check_mark:|:heavy_check_mark:|:heavy_check_mark:|:heavy_check_mark:|
| SQLite |:heavy_check_mark:|:heavy_check_mark:|:heavy_check_mark:|:heavy_check_mark:|:heavy_check_mark:|:heavy_check_mark:|:o:|
| Key-Value Store |:heavy_check_mark:|:heavy_check_mark:|:heavy_check_mark:|:heavy_check_mark:|:heavy_check_mark:|:heavy_check_mark:|:o:|
| JSON Codec |:heavy_check_mark:|:heavy_check_mark:|:heavy_check_mark:|:heavy_check_mark:|:heavy_check_mark:|:heavy_check_mark:|:heavy_check_mark:|
| Crypto |:heavy_check_mark:|:heavy_check_mark:|:heavy_check_mark:|:heavy_check_mark:|:heavy_check_mark:|:heavy_check_mark:|:heavy_check_mark:|
| Zip |:heavy_check_mark:|:heavy_check_mark:|:grey_exclamation:|:heavy_check_mark:|:heavy_check_mark:|:heavy_check_mark:|:heavy_check_mark:|
| Device Info |:heavy_check_mark:|:heavy_check_mark:|:heavy_check_mark:|:grey_exclamation:|:heavy_check_mark:|:grey_exclamation:|:x:|
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

<table>
  <caption>C++ New Features Compatibility</caption>
  <thead>
    <tr>
      <th>Standard</th>
      <th>Feature</th>
      <th>Unsupported Platforms</th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td rowspan="2">C++17</td>
      <td><a href="https://en.cppreference.com/w/cpp/utility/from_chars">std::from_chars</a> for integer</td>
      <td>iOS 16.5-, macOS 13.4-;</td>
    </tr>
    <tr>
      <td><a href="https://en.cppreference.com/w/cpp/utility/from_chars">std::from_chars</a> for float</td>
      <td>All;</td>
    </tr>
    <tr>
      <td rowspan="4">C++20</td>
      <td><a href="https://en.cppreference.com/w/cpp/utility/format/format">std::format</a></td>
      <td>iOS 16.3-, macOS 13.3-, HarmonyOS;</td>
    </tr>
    <tr>
      <td><a href="https://en.cppreference.com/w/cpp/thread/jthread.html">std::jthread</a></td>
      <td>Apple;</td>
    </tr>
    <tr>
      <td><a href="https://en.cppreference.com/w/cpp/ranges">std::ranges</a></td>
      <td>iOS 16.3-, macOS 13.3-, HarmonyOS;</td>
    </tr>
    <tr>
      <td><a href="https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p0919r3.html">Heterogeneous lookup for hash map/set</a></td>
      <td>HarmonyOS;</td>
    </tr>
    <tr>
      <td rowspan="5">C++23</td>
      <td><a href="https://en.cppreference.com/w/cpp/utility/functional/move_only_function.html">std::move_only_function</a></td>
      <td>Clang, Apple &amp; HarmonyOS;</td>
    </tr>
    <tr>
      <td><a href="https://en.cppreference.com/w/cpp/ranges/to.html">std::ranges::to</a></td>
      <td>Clang, Apple &amp; HarmonyOS;</td>
    </tr>
    <tr>
      <td><a href="https://en.cppreference.com/w/cpp/ranges/chunk_view">std::ranges::views::chunk</a></td>
      <td>Clang, Apple &amp; HarmonyOS;</td>
    </tr>
    <tr>
      <td><a href="https://en.cppreference.com/w/cpp/ranges/enumerate_view">std::ranges::views::enumerate</a></td>
      <td>Clang, Apple &amp; HarmonyOS;</td>
    </tr>
    <tr>
      <td><a href="https://en.cppreference.com/w/cpp/memory/out_ptr_t/out_ptr">std::out_ptr</a></td>
      <td>HarmonyOS;</td>
    </tr>
  </tbody>
</table>

<details>

<summary>Reference:</summary>

* [C++ compiler support - cppreference.com][22];
* [C++ Language Support - Xcode - Apple Developer][21];

</details>

## Dependencies

<table>
  <thead>
    <tr>
      <th>Repo</th>
      <th>Desc</th>
      <th>Remark</th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td><a href="https://github.com/ada-url/ada">ada-url</a></td>
      <td>URL parser;</td>
      <td>Requires <code>std::ranges</code> in C++20</td>
    </tr>
    <tr>
      <td><a href="https://github.com/curl/curl">curl</a></td>
      <td>Networking;</td>
      <td></td>
    </tr>
    <tr>
      <td><a href="https://github.com/DaveGamble/cJSON">cJSON</a></td>
      <td>JSON codec;</td>
      <td></td>
    </tr>
    <tr>
      <td><a href="https://github.com/leetal/ios-cmake">iOS-cmake</a></td>
      <td>CMake toolchain for Apple platforms;</td>
      <td></td>
    </tr>
    <tr>
      <td><a href="https://gist.github.com/tylerneylon/59f4bcf316be525b30ab">json.lua</a></td>
      <td>JSON codec for Lua;</td>
      <td></td>
    </tr>
    <tr>
      <td><a href="https://github.com/libuv/libuv">libuv</a></td>
      <td>Asynchronous I/O;</td>
      <td></td>
    </tr>
    <tr>
      <td><a href="https://github.com/lua/lua">lua</a></td>
      <td>Lua runtime;</td>
      <td></td>
    </tr>
    <tr>
      <td><a href="https://github.com/Tencent/MMKV">MMKV</a></td>
      <td>Key-value storage;</td>
      <td></td>
    </tr>
    <tr>
      <td><a href="https://github.com/openssl/openssl">openssl</a></td>
      <td>Crypto;</td>
      <td></td>
    </tr>
    <tr>
      <td><a href="https://github.com/bellard/quickjs">quickjs</a></td>
      <td>JS runtime;</td>
      <td></td>
    </tr>
    <tr>
      <td><a href="https://github.com/gabime/spdlog">spdlog</a></td>
      <td>File logging;</td>
      <td></td>
    </tr>
    <tr>
      <td><a href="https://github.com/sqlite/sqlite">sqlite</a></td>
      <td>DB storage;</td>
      <td></td>
    </tr>
    <tr>
      <td><a href="https://github.com/madler/zlib">zlib</a></td>
      <td>Compression.</td>
      <td></td>
    </tr>
  </tbody>
</table>

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
[16]: https://sonarcloud.io/project/overview?id=R1NC_DynXX
[17]: https://github.com/madler/zlib
[18]: https://github.com/leetal/ios-cmake
[19]: https://en.cppreference.com/w/cpp/utility/from_chars
[20]: https://en.cppreference.com/w/cpp/ranges/enumerate_view
[21]: https://developer.apple.com/xcode/cpp/
[22]: https://en.cppreference.com/w/cpp/compiler_support
[23]: https://en.cppreference.com/w/cpp/utility/functional/move_only_function.html
[24]: ../../actions/workflows/CI-HarmonyOS-macOS.yml
[25]: https://sonarcloud.io/project/issues?issueStatuses=OPEN%2CCONFIRMED&id=R1NC_DynXX
[26]: ../../actions/workflows/CI-Android-macOS.yml
[27]: https://gist.github.com/tylerneylon/59f4bcf316be525b30ab
[28]: https://en.cppreference.com/w/cpp/thread/jthread.html
[29]: ../../actions/workflows/CI-WASM-macOS.yml
[30]: ../../actions/workflows/CI-Linux-Ubuntu.yml
[31]: https://github.com/R1NC/DynRS
[32]: https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p0919r3.html
[33]: ../../actions/workflows/CI-iOS.yml
[34]: ../../actions/workflows/CI-macOS.yml
[35]: ../../actions/workflows/CI-Windows.yml
[36]: https://en.cppreference.com/w/cpp/ranges/to.html
[37]: https://en.cppreference.com/w/cpp/memory/out_ptr_t/out_ptr
