# NGenXX

A cross-platform C/C++ framework.

![Arch](/res/arch.png)

### Progress:

| | Android | iOS | HarmonyOS  | Windows | macOS | Linux | WebAssembly |
| :-- | :--: | :--: |:--: |:--: | :--: | :--: |:--: |
| Network |✔️|✔️|✔️|✔️|✔️|✔️|⁉️|
| SQLite |✔️|✔️|✔️|🛠️|✔️|✔️|🛠️|
|Key-Value Store|✔️|✔️|✔️|🛠️|✔️|✔️|🛠️|
| Lua Runtime |✔️|✔️|✔️|✔️|✔️|✔️|✔️|
| JS Runtime|🛠️|🛠️|🛠️|❌|🛠️|🛠️|🛠️|
| JSON |🛠️|🛠️|🛠️|🛠️|🛠️|🛠️|🛠️|
| Device Info |🛠️|✔️|🛠️|🛠️|✔️|🛠️|❔|

- ❔ : Unknown.
- ⭕ : Not supported.
- ❌ : With compiling error.
- 🛠️ : Under developing.
- ⁉️ : With running error.
- ✔️ : Running well.

### Unsupported Features in WebAssembly:

- Load Lua script with file;(Will trigger a prompt window.)
- [C/C++ callback JS function in async thread][2];

### Requirements:

- C99, C++17;
- Clang;
- CMake;
- Xcode, for iOS & macOS;
- Android Studio(or IntelliJ IDEA) with NDK;
- DevEco Studio with Native SDK, for HarmonyOS;
- [Emscripten][1], for WebAssembly;

[1]: https://emscripten.org/docs/getting_started/downloads.html#sdk-download-and-install
[2]: https://github.com/emscripten-core/emscripten/issues/16567
