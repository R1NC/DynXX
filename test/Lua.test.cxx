#include <gtest/gtest.h>
#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string_view>
#include <chrono>
#include <DynXX/CXX/Lua.hxx>
#include <DynXX/CXX/Types.hxx>
#include "TestUtil.hxx"

class DynXXLuaTestSuite : public ::testing::Test {};

namespace {
    using LuaRuntimePaths = std::array<std::filesystem::path, 3>;
    using LuaCallEntry = std::pair<std::string_view, std::string_view>;

    const LuaRuntimePaths kLuaRuntimePaths = []() {
        return LuaRuntimePaths{
            DynXX::TestUtil::resolveLuaRuntimePath("json.lua"),
            DynXX::TestUtil::resolveLuaRuntimePath("DynXX.lua"),
            DynXX::TestUtil::resolveLuaRuntimePath("biz.lua")
        };
    }();

    void assertLuaRuntimeFilesExist(const LuaRuntimePaths &paths) {
        for (const auto &path : paths) {
            ASSERT_TRUE(std::filesystem::exists(path));
        }
    }

    std::string readAll(const std::filesystem::path &file) {
        std::ifstream in(file, std::ios::binary);
        std::stringstream ss;
        ss << in.rdbuf();
        return ss.str();
    }

    constexpr std::array<LuaCallEntry, 7> kLuaCallEntries{{
        {"TestNetHttpRequest", "https://rinc.xyz"},
        {"TestDeviceInfo", "{}"},
        {"TestCoding", "{}"},
        {"TestCrypto", "DynXX"},
        {"TestKV", "{}"},
        {"TestSQLite", "{}"},
        {"TestCoroutine", "https://rinc.xyz"}
    }};

}

TEST_F(DynXXLuaTestSuite, LoadF) {
    const auto invalidJsPath = DynXX::TestUtil::resolveJsRuntimePath("biz.js");
    assertLuaRuntimeFilesExist(kLuaRuntimePaths);
    ASSERT_TRUE(std::filesystem::exists(invalidJsPath));
    EXPECT_FALSE(dynxxLuaLoadF(""));
    for (const auto &path : kLuaRuntimePaths) {
        EXPECT_TRUE(dynxxLuaLoadF(path.string()));
    }
    EXPECT_FALSE(dynxxLuaLoadF(invalidJsPath.string()));
}

TEST_F(DynXXLuaTestSuite, LoadS) {
    const auto invalidJsPath = DynXX::TestUtil::resolveJsRuntimePath("biz.js");
    assertLuaRuntimeFilesExist(kLuaRuntimePaths);
    ASSERT_TRUE(std::filesystem::exists(invalidJsPath));
    const auto jsScript = readAll(invalidJsPath);
    ASSERT_FALSE(jsScript.empty());
    EXPECT_FALSE(dynxxLuaLoadS(""));
    for (const auto &path : kLuaRuntimePaths) {
        const auto luaScript = readAll(path);
        ASSERT_FALSE(luaScript.empty());
        EXPECT_TRUE(dynxxLuaLoadS(luaScript));
    }
    EXPECT_FALSE(dynxxLuaLoadS(jsScript));
}

TEST_F(DynXXLuaTestSuite, Call) {
    assertLuaRuntimeFilesExist(kLuaRuntimePaths);
    for (const auto &path : kLuaRuntimePaths) {
        ASSERT_TRUE(dynxxLuaLoadF(path.string()));
    }
    for (const auto &[func, params] : kLuaCallEntries) {
        std::cout << "[ LUA CALL ] " << func << '\n';
        const auto callResult = dynxxLuaCall(func, params);
        EXPECT_TRUE(callResult.has_value()) << func;
    }
    EXPECT_FALSE(dynxxLuaCall("TestMethodNotExists_ForError", "https://rinc.xyz").has_value());
    EXPECT_FALSE(dynxxLuaCall("", "{}").has_value());
}

TEST_F(DynXXLuaTestSuite, TimerAsyncWaitOneMinute) {
    assertLuaRuntimeFilesExist(kLuaRuntimePaths);
    for (const auto &path : kLuaRuntimePaths) {
        ASSERT_TRUE(dynxxLuaLoadF(path.string()));
    }
    const auto callResult = dynxxLuaCall("TestTimer", "{}");
    ASSERT_TRUE(callResult.has_value());
    EXPECT_TRUE(DynXX::TestUtil::waitByTimerTask(
        std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::minutes(1)),
        std::chrono::seconds(70)
    ));
}
