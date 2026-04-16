#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <string_view>
#include <vector>
#include <DynXX/CXX/JS.hxx>
#include "TestUtil.hxx"

namespace {
    using JsRuntimePaths = std::pair<std::filesystem::path, std::filesystem::path>;

    std::filesystem::path jsRuntimePath(std::string_view fileName) {
        return DynXX::TestUtil::resolveRepoRootPath() / "scripts" / "JS" / fileName;
    }

    JsRuntimePaths jsRuntimePaths() {
        return {jsRuntimePath("DynXX.js"), jsRuntimePath("biz.js")};
    }

    void assertJsRuntimeFilesExist(const JsRuntimePaths &paths) {
        ASSERT_TRUE(std::filesystem::exists(paths.first));
        ASSERT_TRUE(std::filesystem::exists(paths.second));
    }

    std::string readAll(const std::filesystem::path &file) {
        std::ifstream in(file, std::ios::binary);
        std::stringstream ss;
        ss << in.rdbuf();
        return ss.str();
    }

    std::vector<std::string> extractJsCallableFunctions(std::string_view script) {
        static const std::regex kPattern(R"((?:^|\n)\s*(?:async\s+)?function\s+(Test[A-Za-z0-9_]*)\s*\()");
        std::vector<std::string> funcs;
        for (auto it = std::sregex_iterator(script.cbegin(), script.cend(), kPattern);
             it != std::sregex_iterator();
             ++it) {
            const auto func = (*it)[1].str();
            funcs.push_back(func);
        }
        return funcs;
    }

}

TEST(JS, DynxxJsLoadF) {
    const auto paths = jsRuntimePaths();
    assertJsRuntimeFilesExist(paths);
    EXPECT_FALSE(dynxxJsLoadF("", false));
    ASSERT_TRUE(dynxxJsLoadF(paths.first.string(), false));
    EXPECT_TRUE(dynxxJsLoadF(paths.second.string(), false));
}

TEST(JS, DynxxJsLoadS) {
    const auto paths = jsRuntimePaths();
    assertJsRuntimeFilesExist(paths);
    const auto dynxxJsScript = readAll(paths.first);
    const auto bizJsScript = readAll(paths.second);
    ASSERT_FALSE(dynxxJsScript.empty());
    ASSERT_FALSE(bizJsScript.empty());
    EXPECT_FALSE(dynxxJsLoadS("", "empty.js", false));
    EXPECT_FALSE(dynxxJsLoadS("globalThis.a = 1;", "", false));
    ASSERT_TRUE(dynxxJsLoadS(dynxxJsScript, paths.first.string(), false));
    EXPECT_TRUE(dynxxJsLoadS(bizJsScript, paths.second.string(), false));
}

TEST(JS, DynxxJsLoadB) {
    EXPECT_FALSE(dynxxJsLoadB({}, false));
}

TEST(JS, DynxxJsCall) {
    const auto paths = jsRuntimePaths();
    assertJsRuntimeFilesExist(paths);
    ASSERT_TRUE(dynxxJsLoadF(paths.first.string(), false));
    ASSERT_TRUE(dynxxJsLoadF(paths.second.string(), false));

    const auto bizJsScript = readAll(paths.second);
    const auto funcs = extractJsCallableFunctions(bizJsScript);
    ASSERT_FALSE(funcs.empty());
    for (const auto &func : funcs) {
        std::cout << "[ JS CALL ] " << func << std::endl;
        const auto callResult = dynxxJsCall(func, "{}", false);
        EXPECT_TRUE(callResult.has_value()) << func;
    }

    // Expected: calling a missing JS function logs an error and returns nullopt.
    EXPECT_FALSE(dynxxJsCall("TestMethodNotExists_ForError", "{}", false).has_value());
    EXPECT_FALSE(dynxxJsCall("", "{}", false).has_value());
}

TEST(JS, DynxxJsSetMsgCallback) {
    EXPECT_NO_THROW(dynxxJsSetMsgCallback([](const char *msg) -> const char * { return msg; }));
}
