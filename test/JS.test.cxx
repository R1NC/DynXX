#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <sstream>
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
    EXPECT_FALSE(dynxxJsCall("", "{}", false).has_value());
    const auto callResult = dynxxJsCall("testPromise", "{}", false);
    EXPECT_TRUE(callResult.has_value());
}

TEST(JS, DynxxJsSetMsgCallback) {
    EXPECT_NO_THROW(dynxxJsSetMsgCallback([](const char *msg) -> const char * { return msg; }));
}
