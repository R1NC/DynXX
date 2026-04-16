# DynXX

A cross-platform framework based on modern C++, supporting biz dev via Lua & JS.

[<img src="https://img.shields.io/sonar/quality_gate/R1NC_DynXX/main?server=https%3A%2F%2Fsonarcloud.io&logo=sonar&label=Sonar%20-%20Quality%20Gate"/>][2]
[<img src="https://img.shields.io/sonar/violations/R1NC_DynXX/main?server=https%3A%2F%2Fsonarcloud.io&format=long&logo=sonar&label=Sonar%20-%20Violations"/>][5] 
[<img src="https://img.shields.io/codecov/c/github/R1NC/DynXX/main?logo=codecov&label=Codecov"/>][6] 
[![Docs](https://img.shields.io/badge/API_Docs-Test_Reports-blue?logo=github)](https://R1NC.github.io/DynXX/)

<table>
   <thead>
      <tr>
         <th colspan="2" rowspan="2">CI Status</th>
         <th colspan="4">Host</th>
      </tr>
      <tr>
         <th align="center"><img src="https://www.svgrepo.com/download/501427/windows.svg" width="20px"/></th>
         <th align="center"><img src="https://www.svgrepo.com/download/508761/apple.svg" width="20px"/></th>
         <th align="center"><img src="https://www.svgrepo.com/download/341997/linux.svg" width="20px"/></th>
      </tr>
   </thead>
   <tbody>
      <tr>
         <td rowspan="8"><strong>Target</strong></td>
         <td align="center"><strong>Android</strong></td>
         <td align="center"><a href="https://github.com/R1NC/DynXX/actions/workflows/CI-Android-Win.yml"><img src="https://img.shields.io/github/actions/workflow/status/R1NC/DynXX/CI-Android-Win.yml?branch=main&label=Android-Win"/></a></td>
         <td align="center"><a href="https://github.com/R1NC/DynXX/actions/workflows/CI-Android-Mac.yml"><img src="https://img.shields.io/github/actions/workflow/status/R1NC/DynXX/CI-Android-Mac.yml?branch=main&label=Android-Mac"/></a></td>
         <td align="center"><a href="https://github.com/R1NC/DynXX/actions/workflows/CI-Android-Ubuntu.yml"><img src="https://img.shields.io/github/actions/workflow/status/R1NC/DynXX/CI-Android-Ubuntu.yml?branch=main&label=Android-Ubuntu"/></a></td>
      </tr>
      <tr>
         <td align="center"><strong>iOS</strong></td>
         <td align="center">N/A</td>
         <td align="center"><a href="https://github.com/R1NC/DynXX/actions/workflows/CI-iOS-Mac.yml"><img src="https://img.shields.io/github/actions/workflow/status/R1NC/DynXX/CI-iOS-Mac.yml?branch=main&label=iOS-Mac"/></a></td>
         <td align="center">N/A</td>
      </tr>
      <tr>
         <td align="center"><strong>OHOS</strong></td>
         <td align="center"><a href="https://github.com/R1NC/DynXX/actions/workflows/CI-OHOS-Win.yml"><img src="https://img.shields.io/github/actions/workflow/status/R1NC/DynXX/CI-OHOS-Win.yml?branch=main&label=OHOS-Win"/></a></td>
         <td align="center"><a href="https://github.com/R1NC/DynXX/actions/workflows/CI-OHOS-Mac.yml"><img src="https://img.shields.io/github/actions/workflow/status/R1NC/DynXX/CI-OHOS-Mac.yml?branch=main&label=OHOS-Mac"/></a></td>
         <td align="center"><a href="https://github.com/R1NC/DynXX/actions/workflows/CI-OHOS-Ubuntu.yml"><img src="https://img.shields.io/github/actions/workflow/status/R1NC/DynXX/CI-OHOS-Ubuntu.yml?branch=main&label=OHOS-Ubuntu"/></a></td>
      </tr>
      <tr>
         <td align="center"><strong>Windows</strong></td>
         <td align="center"><a href="https://github.com/R1NC/DynXX/actions/workflows/CI-Windows-Win.yml"><img src="https://img.shields.io/github/actions/workflow/status/R1NC/DynXX/CI-Windows-Win.yml?branch=main&label=Windows-Win"/></a></td>
         <td align="center">N/A</td>
         <td align="center">N/A</td>
      </tr>
      <tr>
         <td align="center"><strong>macOS</strong></td>
         <td align="center">N/A</td>
         <td align="center"><a href="https://github.com/R1NC/DynXX/actions/workflows/CI-macOS-Mac.yml"><img src="https://img.shields.io/github/actions/workflow/status/R1NC/DynXX/CI-macOS-Mac.yml?branch=main&label=macOS-Mac"/></a></td>
         <td align="center">N/A</td>
      </tr>
      <tr>
         <td align="center"><strong>Linux</strong></td>
         <td align="center">N/A</td>
         <td align="center">N/A</td>
         <td align="center"><a href="https://github.com/R1NC/DynXX/actions/workflows/CI-Linux-Ubuntu.yml"><img src="https://img.shields.io/github/actions/workflow/status/R1NC/DynXX/CI-Linux-Ubuntu.yml?branch=main&label=Linux-Ubuntu"/></a></td>
      </tr>
      <tr>
         <td align="center"><strong>WASM</strong></td>
         <td align="center"><a href="https://github.com/R1NC/DynXX/actions/workflows/CI-WASM-Win.yml"><img src="https://img.shields.io/github/actions/workflow/status/R1NC/DynXX/CI-WASM-Win.yml?branch=main&label=WASM-Win"/></a></td>
         <td align="center"><a href="https://github.com/R1NC/DynXX/actions/workflows/CI-WASM-Mac.yml"><img src="https://img.shields.io/github/actions/workflow/status/R1NC/DynXX/CI-WASM-Mac.yml?branch=main&label=WASM-Mac"/></a></td>
         <td align="center"><a href="https://github.com/R1NC/DynXX/actions/workflows/CI-WASM-Ubuntu.yml"><img src="https://img.shields.io/github/actions/workflow/status/R1NC/DynXX/CI-WASM-Ubuntu.yml?branch=main&label=WASM-Ubuntu"/></a></td>
      </tr>
   </tbody>
</table>

## :classical_building: Architecture

<img src="res/arch.svg" width="555px"/>

## :clipboard: Progress

| | Android | iOS | OHOS  | Windows | macOS | Linux | WASM |
| :-- | :--: | :--: | :--: | :--: | :--: | :--: | :--: |
| Lua Runtime | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | :o: |
| JS Runtime | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | :o: |
| Network | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: |
| SQLite | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | :o: |
| Key-Value Store | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | :o: |
| JSON Codec | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: |
| Crypto | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: |
| Zip | :heavy_check_mark: | :heavy_check_mark: | :grey_exclamation: | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: |
| Device Info | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | :grey_exclamation: | :heavy_check_mark: | :grey_exclamation: | :x: |
| Log | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | :grey_exclamation: | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: |

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

* Load Lua script with file; (Will trigger a prompt window)
* [C/C++ callback JS function in async thread][1].

</details>

## :sparkles: C++ New Features Compatibility

<table>
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
      <td rowspan="5">C++20</td>
      <td><a href="https://en.cppreference.com/w/cpp/utility/format/format">std::format</a></td>
      <td>iOS 16.3-, macOS 13.3-, OHOS;</td>
    </tr>
    <tr>
      <td><a href="https://en.cppreference.com/w/cpp/thread/jthread.html">std::jthread</a></td>
      <td>Apple;</td>
    </tr>
    <tr>
      <td><a href="https://en.cppreference.com/w/cpp/ranges">std::ranges</a></td>
      <td>iOS 16.3-, macOS 13.3-, OHOS;</td>
    </tr>
    <tr>
      <td><a href="https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p0919r3.html">Heterogeneous lookup for hash map/set</a></td>
      <td>OHOS;</td>
    </tr>
    <tr>
      <td><a href="https://en.cppreference.com/w/cpp/utility/source_location">std::source_location</a></td>
      <td>OHOS;</td>
    </tr>
    <tr>
      <td rowspan="5">C++23</td>
      <td><a href="https://en.cppreference.com/w/cpp/utility/functional/move_only_function.html">std::move_only_function</a></td>
      <td>Clang, Apple &amp; OHOS;</td>
    </tr>
    <tr>
      <td><a href="https://en.cppreference.com/w/cpp/ranges/to.html">std::ranges::to</a></td>
      <td>Clang, Apple &amp; OHOS;</td>
    </tr>
    <tr>
      <td><a href="https://en.cppreference.com/w/cpp/ranges/chunk_view">std::ranges::views::chunk</a></td>
      <td>Clang, Apple &amp; OHOS;</td>
    </tr>
    <tr>
      <td><a href="https://en.cppreference.com/w/cpp/ranges/enumerate_view">std::ranges::views::enumerate</a></td>
      <td>Clang, Apple &amp; OHOS;</td>
    </tr>
    <tr>
      <td><a href="https://en.cppreference.com/w/cpp/memory/out_ptr_t/out_ptr">std::out_ptr</a></td>
      <td>OHOS;</td>
    </tr>
  </tbody>
</table>

<details>

<summary>Reference:</summary>

* [C++ compiler support - cppreference.com][4];
* [C++ Language Support - Xcode - Apple Developer][3];

</details>

## :package: Dependencies

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

[1]: https://github.com/emscripten-core/emscripten/issues/16567
[2]: https://sonarcloud.io/project/overview?id=R1NC_DynXX
[3]: https://developer.apple.com/xcode/cpp/
[4]: https://en.cppreference.com/w/cpp/compiler_support
[5]: https://sonarcloud.io/project/issues?issueStatuses=OPEN%2CCONFIRMED&id=R1NC_DynXX
[6]: https://app.codecov.io/gh/R1NC/DynXX
