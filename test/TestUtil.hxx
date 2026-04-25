#pragma once

#include <cstdio>
#include <cstdlib>
#include <chrono>
#include <condition_variable>
#include <filesystem>
#include <mutex>
#include <string>
#include <string_view>
#include "../src/core/concurrent/TimerTask.hxx"

#if defined(__has_include)
#if __has_include(<source_location>)
#include <source_location>
#endif
#endif

namespace DynXX::TestUtil {
    inline std::string envValue(const char *name) {
#if defined(_WIN32)
        char *buffer = nullptr;
        size_t len = 0;
        if (_dupenv_s(&buffer, &len, name) != 0 || buffer == nullptr || len == 0) {
            return {};
        }
        std::string value(buffer);
        std::free(buffer);
        return value;
#else
        if (const auto *v = std::getenv(name); v != nullptr) {
            return v;
        }
        return {};
#endif
    }

    inline  bool envEnabled(const char *name, bool defaultValue) {
        const auto v = envValue(name);
        if (v.empty()) {
            return defaultValue;
        }
        return v == "1" || v == "true" || v == "TRUE" || v == "on" || v == "ON";
    }

    inline bool isRepoRootPath(const std::filesystem::path &path) {
        return std::filesystem::exists(path / "vcpkg.json");
    }

    inline std::filesystem::path resolveRepoRootPath() {
#if defined(__cpp_lib_source_location) && (__cpp_lib_source_location >= 201907L)
        if (const auto bySourceRoot = std::filesystem::path(std::source_location::current().file_name()).parent_path().parent_path();
            isRepoRootPath(bySourceRoot)) {
            return bySourceRoot;
        }
#endif
        auto current = std::filesystem::current_path();
        if (isRepoRootPath(current)) {
            return current;
        }
        while (current.has_parent_path()) {
            current = current.parent_path();
            if (isRepoRootPath(current)) {
                return current;
            }
        }
        return std::filesystem::current_path();
    }

    inline std::filesystem::path resolveTempPath() {
        if (const auto runnerTmp = envValue("RUNNER_TEMP"); !runnerTmp.empty()) {
            return {runnerTmp};
        }
        return std::filesystem::temp_directory_path();
    }

    inline std::FILE *openFile(const std::filesystem::path &path, const char *mode) {
#if defined(_WIN32)
        std::FILE *fp = nullptr;
        if (fopen_s(&fp, path.string().c_str(), mode) != 0) {
            return nullptr;
        }
        return fp;
#else
        return std::fopen(path.string().c_str(), mode);
#endif
    }

    inline std::filesystem::path resolveScriptsRootPath() {
        return resolveRepoRootPath() / "scripts";
    }

    inline std::filesystem::path resolveJsRuntimePath(std::string_view fileName) {
        return resolveScriptsRootPath() / "JS" / fileName;
    }

    inline std::filesystem::path resolveLuaRuntimePath(std::string_view fileName) {
        return resolveScriptsRootPath() / "Lua" / fileName;
    }

    inline bool waitByTimerTask(std::chrono::microseconds delay, std::chrono::milliseconds waitTimeout) {
        std::mutex mutex;
        std::condition_variable cv;
        bool fired = false;

        DynXX::Core::Concurrent::TimerTask timerTask(
            [&]() {
                const auto lock = std::scoped_lock(mutex);
                fired = true;
                cv.notify_one();
            },
            static_cast<size_t>(delay.count())
        );

        auto lock = std::unique_lock(mutex);
        return cv.wait_for(lock, waitTimeout, [&]() { return fired; });
    }
}
