#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <DynXX/CXX/Lua.hxx>
#include "TestUtil.hxx"

namespace {
    using LuaRuntimePaths = std::pair<std::filesystem::path, std::filesystem::path>;

    std::filesystem::path luaRuntimePath(std::string_view fileName) {
        return DynXX::TestUtil::resolveRepoRootPath() / "scripts" / "Lua" / fileName;
    }

    LuaRuntimePaths luaRuntimePaths() {
        return {luaRuntimePath("DynXX.lua"), luaRuntimePath("biz.lua")};
    }

    void assertLuaRuntimeFilesExist(const LuaRuntimePaths &paths) {
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

TEST(Lua, DynxxLuaLoadF) {
    const auto paths = luaRuntimePaths();
    assertLuaRuntimeFilesExist(paths);
    ASSERT_TRUE(dynxxLuaLoadF(paths.first.string()));
    EXPECT_TRUE(dynxxLuaLoadF(paths.second.string()));
}

TEST(Lua, DynxxLuaLoadS) {
    const auto paths = luaRuntimePaths();
    assertLuaRuntimeFilesExist(paths);
    const auto dynxxLuaScript = readAll(paths.first);
    const auto bizLuaScript = readAll(paths.second);
    ASSERT_FALSE(dynxxLuaScript.empty());
    ASSERT_FALSE(bizLuaScript.empty());
    ASSERT_TRUE(dynxxLuaLoadS(dynxxLuaScript));
    EXPECT_TRUE(dynxxLuaLoadS(bizLuaScript));
}

TEST(Lua, DynxxLuaCall) {
    const auto paths = luaRuntimePaths();
    assertLuaRuntimeFilesExist(paths);
    ASSERT_TRUE(dynxxLuaLoadF(paths.first.string()));
    ASSERT_TRUE(dynxxLuaLoadF(paths.second.string()));
    const auto callResult = dynxxLuaCall("TestCoding", "{}");
    EXPECT_TRUE(callResult.has_value());
}
