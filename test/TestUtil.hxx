#pragma once

#include <cstdlib>
#include <filesystem>

#if defined(__has_include)
#if __has_include(<source_location>)
#include <source_location>
#endif
#endif

namespace DynXX::TestUtil {
    inline  bool envEnabled(const char *name, bool defaultValue) {
        const auto *v = std::getenv(name);
        if (v == nullptr || *v == '\0') {
            return defaultValue;
        }
        const std::string s(v);
        return s == "1" || s == "true" || s == "TRUE" || s == "on" || s == "ON";
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
        if (const auto runnerTmp = std::getenv("RUNNER_TEMP"); runnerTmp != nullptr) {
            return {runnerTmp};
        }
        return std::filesystem::temp_directory_path();
    }
}
