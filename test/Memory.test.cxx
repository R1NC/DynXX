#include <gtest/gtest.h>
#include <array>
#include <type_traits>
#include <vector>
#include <DynXX/CXX/Memory.hxx>

TEST(Memory, DupCStr) {
    const auto *copied = dupCStr("abc", 3);
    ASSERT_NE(copied, nullptr);
    const char *toFree = copied;
    freeX(toFree);
    EXPECT_EQ(toFree, nullptr);
}

TEST(Memory, DupStr) {
    const auto *copied = dupStr("dynxx");
    ASSERT_NE(copied, nullptr);
    const char *toFree = copied;
    freeX(toFree);
    EXPECT_EQ(toFree, nullptr);
}

TEST(Memory, MemcpyX) {
    std::array<int, 3> src{1, 2, 3};
    std::array<int, 3> dst{0, 0, 0};
    memcpyX(src.data(), dst.data(), src.size());
    EXPECT_EQ(dst, src);
}

TEST(Memory, MallocXCharacter) {
    auto *ptr = mallocX<char>(8);
    ASSERT_NE(ptr, nullptr);
    freeX(ptr);
    EXPECT_EQ(ptr, nullptr);
}

TEST(Memory, MallocXNonCharacter) {
    auto *ptr = mallocX<int>(4);
    ASSERT_NE(ptr, nullptr);
    freeX(ptr);
    EXPECT_EQ(ptr, nullptr);
}

TEST(Memory, FreeXNonConstNonVoid) {
    auto *ptr = mallocX<int>(1);
    ASSERT_NE(ptr, nullptr);
    freeX(ptr);
    EXPECT_EQ(ptr, nullptr);
}

TEST(Memory, FreeXNonConstVoid) {
    auto *raw = std::malloc(8);
    ASSERT_NE(raw, nullptr);
    void *ptr = raw;
    freeX(ptr);
    EXPECT_EQ(ptr, nullptr);
}

TEST(Memory, FreeXConstNonVoid) {
    auto *raw = mallocX<int>(1);
    ASSERT_NE(raw, nullptr);
    const int *ptr = raw;
    freeX(ptr);
    EXPECT_EQ(ptr, nullptr);
}

TEST(Memory, FreeXConstVoid) {
    auto *raw = std::malloc(8);
    ASSERT_NE(raw, nullptr);
    const void *ptr = raw;
    freeX(ptr);
    EXPECT_EQ(ptr, nullptr);
}

TEST(Memory, CopyRange) {
    std::vector<int> src{1, 2, 3, 4};
    std::vector<int> dst;
    copyRange(src, dst, 3);
    EXPECT_EQ(dst, (std::vector<int>{1, 2, 3}));
}

TEST(Memory, FreeDeleter) {
    void *raw = std::malloc(8);
    ASSERT_NE(raw, nullptr);
    EXPECT_NO_FATAL_FAILURE(FreeDeleter{}(raw));
}

TEST(Memory, AutoFreePtr) {
    AutoFreePtr ptr(std::malloc(8));
    EXPECT_NE(ptr.get(), nullptr);
}

TEST(Memory, DynXXStaticOnly) {
    EXPECT_FALSE(std::is_constructible_v<DynXXStaticOnly>);
}

TEST(Memory, DynXXStackOnly) {
    EXPECT_FALSE(std::is_destructible_v<DynXXStackOnly>);
}

TEST(Memory, DynXXHeapOnly) {
    EXPECT_FALSE(std::is_constructible_v<DynXXHeapOnly>);
}
