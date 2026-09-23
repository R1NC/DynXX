#include <gtest/gtest.h>

#include <cstdlib>
#include <cstring>
#include <span>
#include <string>
#include <vector>

#include "../src/core/util/TypeUtil.hxx"

/**
 * `TypeUtil.hxx` is internal plumbing used only by the C API layer: it hands out
 * `std::malloc`-allocated arrays whose ownership transfers to the caller. It used to be
 * covered only indirectly through the C entry points, so these tests pin the contract
 * directly -- element contents, truncation, NUL termination and how empty inputs behave.
 */
class DynXXTypeUtilTestSuite : public ::testing::Test {};

namespace {
    using DynXX::Core::Util::Type::dupBytes;
    using DynXX::Core::Util::Type::dupCharsArray;

    // Releases a malloc'd pointee regardless of its cv-qualification.
    void freeRaw(const void *ptr) {
        std::free(const_cast<void *>(ptr));
    }
}

TEST_F(DynXXTypeUtilTestSuite, DupBytesCopiesContents) {
    const std::vector<byte> raw{'D', 'y', 'n', 'X', 'X'};

    const auto *copied = dupBytes(raw);
    ASSERT_NE(copied, nullptr);
    EXPECT_EQ(std::memcmp(copied, raw.data(), raw.size()), 0);
    freeRaw(copied);
}

TEST_F(DynXXTypeUtilTestSuite, DupBytesEmptyViewsReturnNullptr) {
    // A default-constructed span has a null `data()`, which is rejected up front.
    EXPECT_EQ(dupBytes({}), nullptr);

    // A span can also be empty while still pointing at a real buffer; neither the null
    // nor the zero-length case may hand out an allocation.
    const byte buffer[1]{0x2A};
    EXPECT_EQ(dupBytes(std::span<const byte>(buffer, 0)), nullptr);
}

TEST_F(DynXXTypeUtilTestSuite, DupCharsArrayCopiesEachElement) {
    const std::vector<std::string> src{"dynxx", "kv"};

    const char **arr = dupCharsArray(src, 64);
    ASSERT_NE(arr, nullptr);
    EXPECT_STREQ(arr[0], "dynxx");
    EXPECT_STREQ(arr[1], "kv");

    for (size_t i = 0; i < src.size(); ++i) {
        freeRaw(arr[i]);
    }
    freeRaw(arr);
}

TEST_F(DynXXTypeUtilTestSuite, DupCharsArrayElementsAreNulTerminated) {
    const std::vector<std::string> src{"dynxx"};

    const char **arr = dupCharsArray(src, 64);
    ASSERT_NE(arr, nullptr);
    ASSERT_EQ(std::strlen(arr[0]), 5U);
    EXPECT_EQ(arr[0][5], '\0');

    freeRaw(arr[0]);
    freeRaw(arr);
}

TEST_F(DynXXTypeUtilTestSuite, DupCharsArrayTruncatesToMaxLength) {
    constexpr size_t kMaxLen = 4;
    const std::vector<std::string> src{"0123456789", "abc", ""};

    const char **arr = dupCharsArray(src, kMaxLen);
    ASSERT_NE(arr, nullptr);
    EXPECT_STREQ(arr[0], "0123");
    EXPECT_STREQ(arr[1], "abc");
    EXPECT_STREQ(arr[2], "");

    for (size_t i = 0; i < src.size(); ++i) {
        freeRaw(arr[i]);
    }
    freeRaw(arr);
}

TEST_F(DynXXTypeUtilTestSuite, DupCharsArrayEmptyInputStillReturnsOwnedArray) {
    // `mallocX<char *>(0)` allocates one zeroed slot, so the result is never null:
    // callers cannot tell "empty" from the pointer alone and must use the out-parameter
    // (this is why `handleOutCharsArray` writes the length before returning).
    const char **arr = dupCharsArray({}, 64);
    ASSERT_NE(arr, nullptr);
    EXPECT_EQ(arr[0], nullptr);
    freeRaw(arr);
}
