#include <gtest/gtest.h>
#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <string_view>
#include <unordered_set>
#include <vector>
#include <DynXX/CXX/Lua.hxx>
#include <DynXX/CXX/Types.hxx>
#include "TestUtil.hxx"

class DynXXLuaTestSuite : public ::testing::Test {};

namespace {
    using LuaRuntimePaths = std::array<std::filesystem::path, 3>;
#if defined(__cpp_lib_generic_unordered_lookup)
    using LuaExcludedSet = std::unordered_set<std::string, TransparentStringHash, std::equal_to<>>;
#else
    using LuaExcludedSet = std::unordered_set<std::string>;
#endif

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

    std::vector<std::string> extractLuaTestFunctions(std::string_view script) {
        static const std::regex kPattern(R"((?:^|\n)\s*function\s+(Test[A-Za-z0-9_]*)\s*\()");
        static const LuaExcludedSet kExcluded{
            "TestTimer"
        };
        const std::string scriptText{script};
        std::vector<std::string> funcs;
        for (auto it = std::sregex_iterator(scriptText.begin(), scriptText.end(), kPattern);
             it != std::sregex_iterator();
             ++it) {
            const auto func = (*it)[1].str();
            if (kExcluded.contains(func)) {
                continue;
            }
            funcs.push_back(func);
        }
        return funcs;
    }

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
    const auto bizLuaScript = readAll(kLuaRuntimePaths[2]);
    const auto funcs = extractLuaTestFunctions(bizLuaScript);
    ASSERT_FALSE(funcs.empty());
    for (const auto &func : funcs) {
        std::cout << "[ LUA CALL ] " << func << '\n';
        const auto callResult = dynxxLuaCall(func, "https://rinc.xyz");
        EXPECT_TRUE(callResult.has_value()) << func;
    }
    EXPECT_FALSE(dynxxLuaCall("TestMethodNotExists_ForError", "https://rinc.xyz").has_value());
    EXPECT_FALSE(dynxxLuaCall("", "{}").has_value());
}



