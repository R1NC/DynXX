#include <gtest/gtest.h>
#include <array>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <DynXX/CXX/Lua.hxx>
#include "TestUtil.hxx"

namespace {
    using LuaRuntimePaths = std::array<std::filesystem::path, 3>;

    const LuaRuntimePaths kLuaRuntimePaths = []() {
        const auto luaDir = DynXX::TestUtil::resolveRepoRootPath() / "scripts" / "Lua";
        return LuaRuntimePaths{
            luaDir / "json.lua", 
            luaDir / "DynXX.lua", 
            luaDir / "biz.lua"
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
}

TEST(Lua, DynxxLuaLoadF) {
    assertLuaRuntimeFilesExist(kLuaRuntimePaths);
    for (const auto &path : kLuaRuntimePaths) {
        EXPECT_TRUE(dynxxLuaLoadF(path.string()));
    }
}

TEST(Lua, DynxxLuaLoadS) {
    assertLuaRuntimeFilesExist(kLuaRuntimePaths);
    for (const auto &path : kLuaRuntimePaths) {
        const auto luaScript = readAll(path);
        ASSERT_FALSE(luaScript.empty());
        EXPECT_TRUE(dynxxLuaLoadS(luaScript));
    }
}

TEST(Lua, DynxxLuaCall) {
    assertLuaRuntimeFilesExist(kLuaRuntimePaths);
    for (const auto &path : kLuaRuntimePaths) {
        ASSERT_TRUE(dynxxLuaLoadF(path.string()));
    }
    const auto callResult = dynxxLuaCall("TestCoding", "{}");
    EXPECT_TRUE(callResult.has_value());
}
