#include <gtest/gtest.h>
#include <array>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string_view>
#include <DynXX/CXX/JS.hxx>
#include "TestUtil.hxx"

class DynXXJSTestSuite : public ::testing::Test {};

namespace {
    using JsRuntimePaths = std::pair<std::filesystem::path, std::filesystem::path>;
    using JsCallEntry = std::pair<std::string_view, std::string_view>;

    JsRuntimePaths jsRuntimePaths() {
        return {
            DynXX::TestUtil::resolveJsRuntimePath("DynXX.js"),
            DynXX::TestUtil::resolveJsRuntimePath("biz.js")
        };
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

    constexpr std::array<JsCallEntry, 14> kJsCallEntries{{
        {"TestDeviceInfo", "{}"},
        {"TestMicrotask", "{}"},
        {"TestNetHttpReqPro", "https://rinc.xyz"},
        {"TestPromise", "{}"},
        {"TestPromiseAll", "{}"},
        {"TestAwait", "{}"},
        {"TestKV", "{}"},
        {"TestSQLite", "{}"},
        {"TestCryptoBase64", "DynXX"},
        {"TestCryptoHash", "DynXX"},
        {"TestCryptoAes", "DynXX"},
        {"TestCryptoAesGcm", "DynXX"},
        {"TestZip", "{}"},
        {"TestCallPlatform", "{}"}
    }};

}

TEST_F(DynXXJSTestSuite, LoadF) {
    const auto paths = jsRuntimePaths();
    const auto invalidLuaPath = DynXX::TestUtil::resolveLuaRuntimePath("biz.lua");
    assertJsRuntimeFilesExist(paths);
    ASSERT_TRUE(std::filesystem::exists(invalidLuaPath));
    EXPECT_FALSE(dynxxJsLoadF("", false));
    ASSERT_TRUE(dynxxJsLoadF(paths.first.string(), false));
    EXPECT_TRUE(dynxxJsLoadF(paths.second.string(), false));
    EXPECT_FALSE(dynxxJsLoadF(invalidLuaPath.string(), false));
}

TEST_F(DynXXJSTestSuite, LoadS) {
    const auto paths = jsRuntimePaths();
    const auto invalidLuaPath = DynXX::TestUtil::resolveLuaRuntimePath("biz.lua");
    assertJsRuntimeFilesExist(paths);
    ASSERT_TRUE(std::filesystem::exists(invalidLuaPath));
    const auto dynxxJsScript = readAll(paths.first);
    const auto bizJsScript = readAll(paths.second);
    const auto luaScript = readAll(invalidLuaPath);
    ASSERT_FALSE(dynxxJsScript.empty());
    ASSERT_FALSE(bizJsScript.empty());
    ASSERT_FALSE(luaScript.empty());
    EXPECT_FALSE(dynxxJsLoadS("", "empty.js", false));
    EXPECT_FALSE(dynxxJsLoadS("globalThis.a = 1;", "", false));
    ASSERT_TRUE(dynxxJsLoadS(dynxxJsScript, paths.first.string(), false));
    EXPECT_TRUE(dynxxJsLoadS(bizJsScript, paths.second.string(), false));
    EXPECT_FALSE(dynxxJsLoadS(luaScript, invalidLuaPath.string(), false));
}

TEST_F(DynXXJSTestSuite, LoadB) {
    EXPECT_FALSE(dynxxJsLoadB({}, false));
    const Bytes nonEmptyBytes{0xFF, 0x00, 0x80};
    EXPECT_FALSE(dynxxJsLoadB(nonEmptyBytes, false));
}

TEST_F(DynXXJSTestSuite, Call) {
    const auto paths = jsRuntimePaths();
    assertJsRuntimeFilesExist(paths);
    ASSERT_TRUE(dynxxJsLoadF(paths.first.string(), false));
    ASSERT_TRUE(dynxxJsLoadF(paths.second.string(), false));

    for (const auto &[func, params] : kJsCallEntries) {
        std::cout << "[ JS CALL ] " << func << '\n';
        const auto callResult = dynxxJsCall(func, params, true);
        EXPECT_TRUE(callResult.has_value()) << func;
    }
    EXPECT_TRUE(DynXX::TestUtil::waitByTimerTask(
        std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::minutes(1)),
        std::chrono::seconds(70)
    ));

    // Expected: calling a missing JS function logs an error and returns nullopt.
    EXPECT_FALSE(dynxxJsCall("TestMethodNotExists_ForError", "{}", false).has_value());
    EXPECT_FALSE(dynxxJsCall("", "{}", false).has_value());
}

TEST_F(DynXXJSTestSuite, SetMsgCallback) {
    EXPECT_NO_THROW(dynxxJsSetMsgCallback([](const char *msg) -> const char * { return msg; }));
}


