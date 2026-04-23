#include <gtest/gtest.h>
#include <array>
#include <type_traits>
#include <vector>
#include <DynXX/CXX/Memory.hxx>

class DynXXMemoryTestSuite : public ::testing::Test {};

TEST_F(DynXXMemoryTestSuite, DupCStr) {
    const auto *copied = dupCStr("abc", 3);
    ASSERT_NE(copied, nullptr);
    const char *toFree = copied;
    freeX(toFree);
    EXPECT_EQ(toFree, nullptr);
}

TEST_F(DynXXMemoryTestSuite, DupCStrNullptr) {
    EXPECT_EQ(dupCStr(nullptr, 0), nullptr);
}

TEST_F(DynXXMemoryTestSuite, DupStr) {
    const auto *copied = dupStr("dynxx");
    ASSERT_NE(copied, nullptr);
    const char *toFree = copied;
    freeX(toFree);
    EXPECT_EQ(toFree, nullptr);
}

TEST_F(DynXXMemoryTestSuite, MemcpyX) {
    std::array<int, 3> src{1, 2, 3};
    std::array<int, 3> dst{0, 0, 0};
    memcpyX(src.data(), dst.data(), src.size());
    EXPECT_EQ(dst, src);
}

TEST_F(DynXXMemoryTestSuite, MemcpyXZeroCount) {
    std::array<int, 3> src{1, 2, 3};
    std::array<int, 3> dst{9, 9, 9};
    memcpyX(src.data(), dst.data(), 0);
    EXPECT_EQ(dst, (std::array<int, 3>{9, 9, 9}));
}

TEST_F(DynXXMemoryTestSuite, MemcpyXNullptr) {
    std::array<int, 3> src{1, 2, 3};
    std::array<int, 3> dst{9, 9, 9};
    memcpyX(static_cast<const int *>(nullptr), dst.data(), src.size());
    memcpyX(src.data(), static_cast<int *>(nullptr), src.size());
    EXPECT_EQ(dst, (std::array<int, 3>{9, 9, 9}));
}

TEST_F(DynXXMemoryTestSuite, MallocXCharacter) {
    auto *ptr = mallocX<char>(8);
    ASSERT_NE(ptr, nullptr);
    freeX(ptr);
    EXPECT_EQ(ptr, nullptr);
}

TEST_F(DynXXMemoryTestSuite, MallocXCharacterZeroCount) {
    auto *ptr = mallocX<char>(0);
    ASSERT_NE(ptr, nullptr);
    freeX(ptr);
    EXPECT_EQ(ptr, nullptr);
}

TEST_F(DynXXMemoryTestSuite, MallocXNonCharacter) {
    auto *ptr = mallocX<int>(4);
    ASSERT_NE(ptr, nullptr);
    freeX(ptr);
    EXPECT_EQ(ptr, nullptr);
}

TEST_F(DynXXMemoryTestSuite, MallocXNonCharacterZeroCount) {
    auto *ptr = mallocX<int>(0);
    if (ptr != nullptr) {
        freeX(ptr);
    }
    EXPECT_EQ(ptr, nullptr);
}

TEST_F(DynXXMemoryTestSuite, FreeXNonConstNonVoid) {
    auto *ptr = mallocX<int>(1);
    ASSERT_NE(ptr, nullptr);
    freeX(ptr);
    EXPECT_EQ(ptr, nullptr);
}

TEST_F(DynXXMemoryTestSuite, FreeXNonConstNonVoidNullptr) {
    int *ptr = nullptr;
    freeX(ptr);
    EXPECT_EQ(ptr, nullptr);
}

TEST_F(DynXXMemoryTestSuite, FreeXNonConstVoid) {
    auto *raw = std::malloc(8);
    ASSERT_NE(raw, nullptr);
    void *ptr = raw;
    freeX(ptr);
    EXPECT_EQ(ptr, nullptr);
}

TEST_F(DynXXMemoryTestSuite, FreeXNonConstVoidNullptr) {
    void *ptr = nullptr;
    freeX(ptr);
    EXPECT_EQ(ptr, nullptr);
}

TEST_F(DynXXMemoryTestSuite, FreeXConstNonVoid) {
    auto *raw = mallocX<int>(1);
    ASSERT_NE(raw, nullptr);
    const int *ptr = raw;
    freeX(ptr);
    EXPECT_EQ(ptr, nullptr);
}

TEST_F(DynXXMemoryTestSuite, FreeXConstNonVoidNullptr) {
    const int *ptr = nullptr;
    freeX(ptr);
    EXPECT_EQ(ptr, nullptr);
}

TEST_F(DynXXMemoryTestSuite, FreeXConstVoid) {
    auto *raw = std::malloc(8);
    ASSERT_NE(raw, nullptr);
    const void *ptr = raw;
    freeX(ptr);
    EXPECT_EQ(ptr, nullptr);
}

TEST_F(DynXXMemoryTestSuite, FreeXConstVoidNullptr) {
    const void *ptr = nullptr;
    freeX(ptr);
    EXPECT_EQ(ptr, nullptr);
}

TEST_F(DynXXMemoryTestSuite, CopyRange) {
    std::vector<int> src{1, 2, 3, 4};
    std::vector<int> dst;
    copyRange(src, dst, 3);
    EXPECT_EQ(dst, (std::vector<int>{1, 2, 3}));
}

TEST_F(DynXXMemoryTestSuite, CopyRangeZeroLen) {
    std::vector<int> src{1, 2, 3, 4};
    std::vector<int> dst{7, 8};
    copyRange(src, dst, 0);
    EXPECT_EQ(dst, (std::vector<int>{7, 8}));
}

TEST_F(DynXXMemoryTestSuite, FreeDeleter) {
    void *raw = std::malloc(8);
    ASSERT_NE(raw, nullptr);
    EXPECT_NO_FATAL_FAILURE(FreeDeleter{}(raw));
}

TEST_F(DynXXMemoryTestSuite, FreeDeleterNullptr) {
    void *raw = nullptr;
    EXPECT_NO_FATAL_FAILURE(FreeDeleter{}(raw));
}

TEST_F(DynXXMemoryTestSuite, AutoFreePtr) {
    AutoFreePtr ptr(std::malloc(8));
    EXPECT_NE(ptr.get(), nullptr);
}

TEST_F(DynXXMemoryTestSuite, StaticOnly) {
    EXPECT_FALSE(std::is_constructible_v<DynXXStaticOnly>);
}

TEST_F(DynXXMemoryTestSuite, StackOnly) {
    EXPECT_FALSE(std::is_destructible_v<DynXXStackOnly>);
}

TEST_F(DynXXMemoryTestSuite, HeapOnly) {
    EXPECT_FALSE(std::is_constructible_v<DynXXHeapOnly>);
}



