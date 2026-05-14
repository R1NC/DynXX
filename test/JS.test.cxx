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
    const auto missingJsPath = DynXX::TestUtil::resolveTempPath() / "dynxx_missing_runtime.js";
    assertJsRuntimeFilesExist(paths);
    ASSERT_TRUE(std::filesystem::exists(invalidLuaPath));
    ASSERT_FALSE(std::filesystem::exists(missingJsPath));
    EXPECT_FALSE(dynxxJsLoadF("", false));
    EXPECT_FALSE(dynxxJsLoadF(missingJsPath.string(), false));
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

TEST_F(DynXXJSTestSuite, CallThrowingFunctionShouldReturnNullopt) {
    ASSERT_TRUE(dynxxJsLoadS("function TestThrowForError(){ throw new Error('DynXXThrow'); }",
                             "throw_error.js", false));
    EXPECT_FALSE(dynxxJsCall("TestThrowForError", "{}", false).has_value());
}

TEST_F(DynXXJSTestSuite, PromiseBranches) {
    const auto paths = jsRuntimePaths();
    assertJsRuntimeFilesExist(paths);
    ASSERT_TRUE(dynxxJsLoadF(paths.first.string(), false));
    ASSERT_TRUE(dynxxJsLoadF(paths.second.string(), false));

    const auto pureResolve = dynxxJsCall("TestPurePromiseResolve", "{}", true);
    ASSERT_TRUE(pureResolve.has_value());
    EXPECT_EQ(*pureResolve, "DynXX");

    const auto pureChain = dynxxJsCall("TestPurePromiseChain", "{}", true);
    ASSERT_TRUE(pureChain.has_value());
    EXPECT_EQ(*pureChain, "3");

    const auto pureAll = dynxxJsCall("TestPurePromiseAll", "{}", true);
    ASSERT_TRUE(pureAll.has_value());
    EXPECT_EQ(*pureAll, "A,B,C");

    const auto pureCatch = dynxxJsCall("TestPurePromiseCatch", "{}", true);
    ASSERT_TRUE(pureCatch.has_value());
    EXPECT_EQ(*pureCatch, "DynXX");

    const auto pureReject = dynxxJsCall("TestPurePromiseReject", "{}", true);
    ASSERT_TRUE(pureReject.has_value());
    EXPECT_TRUE(pureReject->empty());
}
