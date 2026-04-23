#include <gtest/gtest.h>
#include <algorithm>
#include <DynXX/CXX/KV.hxx>

class DynXXKVTestSuite : public ::testing::Test {};

TEST_F(DynXXKVTestSuite, Open) {
    const auto conn = dynxxKVOpen("kv_open");
    ASSERT_NE(conn, 0U);
    dynxxKVClose(conn);
}

TEST_F(DynXXKVTestSuite, Open_EmptyId) {
    EXPECT_EQ(dynxxKVOpen(""), 0U);
}

TEST_F(DynXXKVTestSuite, ReadString) {
    const auto conn = dynxxKVOpen("kv_read_string");
    ASSERT_NE(conn, 0U);
    ASSERT_TRUE(dynxxKVWriteString(conn, "k", "v"));
    EXPECT_EQ(dynxxKVReadString(conn, "k").value_or(""), "v");
    EXPECT_FALSE(dynxxKVReadString(conn, "not-exist").has_value());
    dynxxKVClose(conn);
}

TEST_F(DynXXKVTestSuite, WriteString) {
    const auto conn = dynxxKVOpen("kv_write_string");
    ASSERT_NE(conn, 0U);
    EXPECT_TRUE(dynxxKVWriteString(conn, "k", "v"));
    dynxxKVClose(conn);
}

TEST_F(DynXXKVTestSuite, ReadInteger) {
    const auto conn = dynxxKVOpen("kv_read_integer");
    ASSERT_NE(conn, 0U);
    ASSERT_TRUE(dynxxKVWriteInteger(conn, "n", 7));
    EXPECT_EQ(dynxxKVReadInteger(conn, "n").value_or(0), 7);
    EXPECT_FALSE(dynxxKVReadInteger(conn, "not-exist").has_value());
    dynxxKVClose(conn);
}

TEST_F(DynXXKVTestSuite, WriteInteger) {
    const auto conn = dynxxKVOpen("kv_write_integer");
    ASSERT_NE(conn, 0U);
    EXPECT_TRUE(dynxxKVWriteInteger(conn, "n", 7));
    dynxxKVClose(conn);
}

TEST_F(DynXXKVTestSuite, ReadFloat) {
    const auto conn = dynxxKVOpen("kv_read_float");
    ASSERT_NE(conn, 0U);
    ASSERT_TRUE(dynxxKVWriteFloat(conn, "f", 3.14));
    EXPECT_NEAR(dynxxKVReadFloat(conn, "f").value_or(0.0), 3.14, 1e-9);
    EXPECT_FALSE(dynxxKVReadFloat(conn, "not-exist").has_value());
    dynxxKVClose(conn);
}

TEST_F(DynXXKVTestSuite, WriteFloat) {
    const auto conn = dynxxKVOpen("kv_write_float");
    ASSERT_NE(conn, 0U);
    EXPECT_TRUE(dynxxKVWriteFloat(conn, "f", 3.14));
    dynxxKVClose(conn);
}

TEST_F(DynXXKVTestSuite, AllKeys) {
    const auto conn = dynxxKVOpen("kv_all_keys");
    ASSERT_NE(conn, 0U);
    ASSERT_TRUE(dynxxKVWriteString(conn, "k1", "v1"));
    const auto keys = dynxxKVAllKeys(conn);
    EXPECT_NE(std::find(keys.begin(), keys.end(), "k1"), keys.end());
    dynxxKVClose(conn);
}

TEST_F(DynXXKVTestSuite, Contains) {
    const auto conn = dynxxKVOpen("kv_contains");
    ASSERT_NE(conn, 0U);
    ASSERT_TRUE(dynxxKVWriteString(conn, "k", "v"));
    EXPECT_TRUE(dynxxKVContains(conn, "k"));
    EXPECT_FALSE(dynxxKVContains(conn, "not-exist"));
    EXPECT_FALSE(dynxxKVContains(conn, ""));
    dynxxKVClose(conn);
}

TEST_F(DynXXKVTestSuite, Remove) {
    const auto conn = dynxxKVOpen("kv_remove");
    ASSERT_NE(conn, 0U);
    ASSERT_TRUE(dynxxKVWriteString(conn, "k", "v"));
    EXPECT_TRUE(dynxxKVRemove(conn, "k"));
    EXPECT_FALSE(dynxxKVContains(conn, "k"));
    EXPECT_FALSE(dynxxKVRemove(conn, "not-exist"));
    EXPECT_FALSE(dynxxKVRemove(conn, ""));
    EXPECT_FALSE(dynxxKVWriteString(conn, "", "v"));
    EXPECT_FALSE(dynxxKVWriteInteger(conn, "", 7));
    EXPECT_FALSE(dynxxKVWriteFloat(conn, "", 3.14));
    EXPECT_FALSE(dynxxKVReadString(conn, "").has_value());
    EXPECT_FALSE(dynxxKVReadInteger(conn, "").has_value());
    EXPECT_FALSE(dynxxKVReadFloat(conn, "").has_value());
    dynxxKVClose(conn);
}

TEST_F(DynXXKVTestSuite, Clear) {
    const auto conn = dynxxKVOpen("kv_clear");
    ASSERT_NE(conn, 0U);
    ASSERT_TRUE(dynxxKVWriteString(conn, "k", "v"));
    dynxxKVClear(conn);
    EXPECT_FALSE(dynxxKVContains(conn, "k"));
    dynxxKVClose(conn);
}

TEST_F(DynXXKVTestSuite, Close) {
    const auto conn = dynxxKVOpen("kv_close");
    ASSERT_NE(conn, 0U);
    EXPECT_NO_THROW(dynxxKVClose(conn));
}

struct KvInvalidParamCase {
    DynXXKVConnHandle conn;
    std::string key;
};

TEST_F(DynXXKVTestSuite, InvalidParam_ReadWriteShouldFailFast) {
    for (const auto &param : {
        KvInvalidParamCase{0, "k"},
        KvInvalidParamCase{0, ""},
        KvInvalidParamCase{1, ""}
    }) {
        EXPECT_FALSE(dynxxKVReadString(param.conn, param.key).has_value());
        EXPECT_FALSE(dynxxKVWriteString(param.conn, param.key, "v"));
        EXPECT_FALSE(dynxxKVReadInteger(param.conn, param.key).has_value());
        EXPECT_FALSE(dynxxKVWriteInteger(param.conn, param.key, 1));
        EXPECT_FALSE(dynxxKVReadFloat(param.conn, param.key).has_value());
        EXPECT_FALSE(dynxxKVWriteFloat(param.conn, param.key, 1.0));
        EXPECT_FALSE(dynxxKVContains(param.conn, param.key));
        EXPECT_FALSE(dynxxKVRemove(param.conn, param.key));
    }
}



