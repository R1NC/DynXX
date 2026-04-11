#include <gtest/gtest.h>
#include <algorithm>
#include <DynXX/CXX/KV.hxx>

TEST(KV, DynxxKVOpen) {
    const auto conn = dynxxKVOpen("kv_open");
    ASSERT_NE(conn, 0U);
    dynxxKVClose(conn);
}

TEST(KV, DynxxKVReadString) {
    const auto conn = dynxxKVOpen("kv_read_string");
    ASSERT_NE(conn, 0U);
    ASSERT_TRUE(dynxxKVWriteString(conn, "k", "v"));
    EXPECT_EQ(dynxxKVReadString(conn, "k").value_or(""), "v");
    dynxxKVClose(conn);
}

TEST(KV, DynxxKVWriteString) {
    const auto conn = dynxxKVOpen("kv_write_string");
    ASSERT_NE(conn, 0U);
    EXPECT_TRUE(dynxxKVWriteString(conn, "k", "v"));
    dynxxKVClose(conn);
}

TEST(KV, DynxxKVReadInteger) {
    const auto conn = dynxxKVOpen("kv_read_integer");
    ASSERT_NE(conn, 0U);
    ASSERT_TRUE(dynxxKVWriteInteger(conn, "n", 7));
    EXPECT_EQ(dynxxKVReadInteger(conn, "n").value_or(0), 7);
    dynxxKVClose(conn);
}

TEST(KV, DynxxKVWriteInteger) {
    const auto conn = dynxxKVOpen("kv_write_integer");
    ASSERT_NE(conn, 0U);
    EXPECT_TRUE(dynxxKVWriteInteger(conn, "n", 7));
    dynxxKVClose(conn);
}

TEST(KV, DynxxKVReadFloat) {
    const auto conn = dynxxKVOpen("kv_read_float");
    ASSERT_NE(conn, 0U);
    ASSERT_TRUE(dynxxKVWriteFloat(conn, "f", 3.14));
    EXPECT_NEAR(dynxxKVReadFloat(conn, "f").value_or(0.0), 3.14, 1e-9);
    dynxxKVClose(conn);
}

TEST(KV, DynxxKVWriteFloat) {
    const auto conn = dynxxKVOpen("kv_write_float");
    ASSERT_NE(conn, 0U);
    EXPECT_TRUE(dynxxKVWriteFloat(conn, "f", 3.14));
    dynxxKVClose(conn);
}

TEST(KV, DynxxKVAllKeys) {
    const auto conn = dynxxKVOpen("kv_all_keys");
    ASSERT_NE(conn, 0U);
    ASSERT_TRUE(dynxxKVWriteString(conn, "k1", "v1"));
    const auto keys = dynxxKVAllKeys(conn);
    EXPECT_NE(std::find(keys.begin(), keys.end(), "k1"), keys.end());
    dynxxKVClose(conn);
}

TEST(KV, DynxxKVContains) {
    const auto conn = dynxxKVOpen("kv_contains");
    ASSERT_NE(conn, 0U);
    ASSERT_TRUE(dynxxKVWriteString(conn, "k", "v"));
    EXPECT_TRUE(dynxxKVContains(conn, "k"));
    dynxxKVClose(conn);
}

TEST(KV, DynxxKVRemove) {
    const auto conn = dynxxKVOpen("kv_remove");
    ASSERT_NE(conn, 0U);
    ASSERT_TRUE(dynxxKVWriteString(conn, "k", "v"));
    EXPECT_TRUE(dynxxKVRemove(conn, "k"));
    EXPECT_FALSE(dynxxKVContains(conn, "k"));
    dynxxKVClose(conn);
}

TEST(KV, DynxxKVClear) {
    const auto conn = dynxxKVOpen("kv_clear");
    ASSERT_NE(conn, 0U);
    ASSERT_TRUE(dynxxKVWriteString(conn, "k", "v"));
    dynxxKVClear(conn);
    EXPECT_FALSE(dynxxKVContains(conn, "k"));
    dynxxKVClose(conn);
}

TEST(KV, DynxxKVClose) {
    const auto conn = dynxxKVOpen("kv_close");
    ASSERT_NE(conn, 0U);
    EXPECT_NO_THROW(dynxxKVClose(conn));
}
