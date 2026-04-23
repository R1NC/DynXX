#include <gtest/gtest.h>
#include <DynXX/CXX/SQLite.hxx>

#include "../src/core/store/SQLite.hxx"

class DynXXSQLiteTestSuite : public ::testing::Test {};

namespace {
    constexpr auto kCreateTextTableSql = "CREATE TABLE IF NOT EXISTS t (v TEXT);";
    constexpr auto kCreateIntegerTableSql = "CREATE TABLE IF NOT EXISTS t (v INTEGER);";
    constexpr auto kCreateRealTableSql = "CREATE TABLE IF NOT EXISTS t (v REAL);";
    constexpr auto kDropTableSql = "DROP TABLE IF EXISTS t;";
    constexpr auto kDeleteAllRowsSql = "DELETE FROM t;";
    constexpr auto kInsertTextRowSql = "INSERT INTO t (v) VALUES ('x');";
    constexpr auto kInsertIntegerRowSql = "INSERT INTO t (v) VALUES (7);";
    constexpr auto kInsertRealRowSql = "INSERT INTO t (v) VALUES (3.14);";
    constexpr auto kSelectSingleRowSql = "SELECT v FROM t LIMIT 1;";
    constexpr auto kSelectAllRowsSql = "SELECT v FROM t;";
}

TEST_F(DynXXSQLiteTestSuite, Open) {
    const auto conn = dynxxSQLiteOpen("sqlite_open");
    ASSERT_NE(conn, 0U);
    dynxxSQLiteClose(conn);
}

TEST_F(DynXXSQLiteTestSuite, Open_EmptyId) {
    EXPECT_EQ(dynxxSQLiteOpen(""), 0U);
}

TEST_F(DynXXSQLiteTestSuite, StoreOpen_EmptyFile) {
    DynXX::Core::Store::SQLite::SQLiteStore store;
    const auto conn = store.open("");
    EXPECT_TRUE(conn.expired());
}

TEST_F(DynXXSQLiteTestSuite, Execute) {
    const auto conn = dynxxSQLiteOpen("sqlite_execute");
    ASSERT_NE(conn, 0U);
    EXPECT_TRUE(dynxxSQLiteExecute(conn, kCreateTextTableSql));
    EXPECT_FALSE(dynxxSQLiteExecute(conn, ""));
    EXPECT_FALSE(dynxxSQLiteExecute(conn, "CREATE TABLE t ("));
    EXPECT_FALSE(dynxxSQLiteExecute(0, kCreateTextTableSql));
    dynxxSQLiteClose(conn);
}

TEST_F(DynXXSQLiteTestSuite, QueryDo) {
    const auto conn = dynxxSQLiteOpen("sqlite_query_do");
    ASSERT_NE(conn, 0U);
    ASSERT_TRUE(dynxxSQLiteExecute(conn, kCreateTextTableSql));
    ASSERT_TRUE(dynxxSQLiteExecute(conn, kInsertTextRowSql));
    const auto qr = dynxxSQLiteQueryDo(conn, kSelectSingleRowSql);
    EXPECT_NE(qr, 0U);
    EXPECT_EQ(dynxxSQLiteQueryDo(conn, ""), 0U);
    const auto badQr = dynxxSQLiteQueryDo(conn, "SELECT bad syntax FROM");
    ASSERT_NE(badQr, 0U);
    EXPECT_FALSE(dynxxSQLiteQueryReadRow(badQr));
    dynxxSQLiteQueryDrop(badQr);
    EXPECT_EQ(dynxxSQLiteQueryDo(0, kSelectSingleRowSql), 0U);
    dynxxSQLiteQueryDrop(qr);
    dynxxSQLiteClose(conn);
}

TEST_F(DynXXSQLiteTestSuite, QueryReadRow) {
    const auto conn = dynxxSQLiteOpen("sqlite_read_row");
    ASSERT_NE(conn, 0U);
    ASSERT_TRUE(dynxxSQLiteExecute(conn, kCreateTextTableSql));
    ASSERT_TRUE(dynxxSQLiteExecute(conn, kDeleteAllRowsSql));
    ASSERT_TRUE(dynxxSQLiteExecute(conn, kInsertTextRowSql));
    const auto qr = dynxxSQLiteQueryDo(conn, kSelectAllRowsSql);
    ASSERT_NE(qr, 0U);
    EXPECT_TRUE(dynxxSQLiteQueryReadRow(qr));
    EXPECT_FALSE(dynxxSQLiteQueryReadRow(0));
    dynxxSQLiteQueryDrop(qr);
    dynxxSQLiteClose(conn);
}

TEST_F(DynXXSQLiteTestSuite, QueryReadColumnText) {
    const auto conn = dynxxSQLiteOpen("sqlite_col_text");
    ASSERT_NE(conn, 0U);
    ASSERT_TRUE(dynxxSQLiteExecute(conn, kCreateTextTableSql));
    ASSERT_TRUE(dynxxSQLiteExecute(conn, kDeleteAllRowsSql));
    ASSERT_TRUE(dynxxSQLiteExecute(conn, kInsertTextRowSql));
    const auto qr = dynxxSQLiteQueryDo(conn, kSelectSingleRowSql);
    ASSERT_NE(qr, 0U);
    ASSERT_TRUE(dynxxSQLiteQueryReadRow(qr));
    EXPECT_EQ(dynxxSQLiteQueryReadColumnText(qr, "v").value_or(""), "x");
    EXPECT_FALSE(dynxxSQLiteQueryReadColumnText(qr, "not-exist").has_value());
    EXPECT_FALSE(dynxxSQLiteQueryReadColumnText(qr, "").has_value());
    EXPECT_FALSE(dynxxSQLiteQueryReadColumnText(0, "v").has_value());
    dynxxSQLiteQueryDrop(qr);
    dynxxSQLiteClose(conn);
}

TEST_F(DynXXSQLiteTestSuite, QueryReadColumnWithoutReadRow) {
    const auto conn = dynxxSQLiteOpen("sqlite_read_without_row");
    ASSERT_NE(conn, 0U);

    ASSERT_TRUE(dynxxSQLiteExecute(conn, kDropTableSql));
    ASSERT_TRUE(dynxxSQLiteExecute(conn, kCreateTextTableSql));
    ASSERT_TRUE(dynxxSQLiteExecute(conn, kDeleteAllRowsSql));
    ASSERT_TRUE(dynxxSQLiteExecute(conn, kInsertTextRowSql));
    const auto textQr = dynxxSQLiteQueryDo(conn, kSelectSingleRowSql);
    ASSERT_NE(textQr, 0U);
    EXPECT_FALSE(dynxxSQLiteQueryReadColumnText(textQr, "v").has_value());
    dynxxSQLiteQueryDrop(textQr);

    ASSERT_TRUE(dynxxSQLiteExecute(conn, kDropTableSql));
    ASSERT_TRUE(dynxxSQLiteExecute(conn, kCreateIntegerTableSql));
    ASSERT_TRUE(dynxxSQLiteExecute(conn, kDeleteAllRowsSql));
    ASSERT_TRUE(dynxxSQLiteExecute(conn, kInsertIntegerRowSql));
    const auto intQr = dynxxSQLiteQueryDo(conn, kSelectSingleRowSql);
    ASSERT_NE(intQr, 0U);
    EXPECT_FALSE(dynxxSQLiteQueryReadColumnInteger(intQr, "v").has_value());
    dynxxSQLiteQueryDrop(intQr);

    ASSERT_TRUE(dynxxSQLiteExecute(conn, kDropTableSql));
    ASSERT_TRUE(dynxxSQLiteExecute(conn, kCreateRealTableSql));
    ASSERT_TRUE(dynxxSQLiteExecute(conn, kDeleteAllRowsSql));
    ASSERT_TRUE(dynxxSQLiteExecute(conn, kInsertRealRowSql));
    const auto realQr = dynxxSQLiteQueryDo(conn, kSelectSingleRowSql);
    ASSERT_NE(realQr, 0U);
    EXPECT_FALSE(dynxxSQLiteQueryReadColumnFloat(realQr, "v").has_value());
    dynxxSQLiteQueryDrop(realQr);

    dynxxSQLiteClose(conn);
}

TEST_F(DynXXSQLiteTestSuite, QueryReadColumnInteger) {
    const auto conn = dynxxSQLiteOpen("sqlite_col_int");
    ASSERT_NE(conn, 0U);
    ASSERT_TRUE(dynxxSQLiteExecute(conn, kCreateIntegerTableSql));
    ASSERT_TRUE(dynxxSQLiteExecute(conn, kDeleteAllRowsSql));
    ASSERT_TRUE(dynxxSQLiteExecute(conn, kInsertIntegerRowSql));
    const auto qr = dynxxSQLiteQueryDo(conn, kSelectSingleRowSql);
    ASSERT_NE(qr, 0U);
    ASSERT_TRUE(dynxxSQLiteQueryReadRow(qr));
    EXPECT_EQ(dynxxSQLiteQueryReadColumnInteger(qr, "v").value_or(0), 7);
    EXPECT_FALSE(dynxxSQLiteQueryReadColumnInteger(qr, "not-exist").has_value());
    EXPECT_FALSE(dynxxSQLiteQueryReadColumnInteger(qr, "").has_value());
    EXPECT_FALSE(dynxxSQLiteQueryReadColumnInteger(0, "v").has_value());
    dynxxSQLiteQueryDrop(qr);
    dynxxSQLiteClose(conn);
}

TEST_F(DynXXSQLiteTestSuite, QueryReadColumnFloat) {
    const auto conn = dynxxSQLiteOpen("sqlite_col_float");
    ASSERT_NE(conn, 0U);
    ASSERT_TRUE(dynxxSQLiteExecute(conn, kCreateRealTableSql));
    ASSERT_TRUE(dynxxSQLiteExecute(conn, kDeleteAllRowsSql));
    ASSERT_TRUE(dynxxSQLiteExecute(conn, kInsertRealRowSql));
    const auto qr = dynxxSQLiteQueryDo(conn, kSelectSingleRowSql);
    ASSERT_NE(qr, 0U);
    ASSERT_TRUE(dynxxSQLiteQueryReadRow(qr));
    EXPECT_NEAR(dynxxSQLiteQueryReadColumnFloat(qr, "v").value_or(0.0), 3.14, 1e-9);
    EXPECT_FALSE(dynxxSQLiteQueryReadColumnFloat(qr, "not-exist").has_value());
    EXPECT_FALSE(dynxxSQLiteQueryReadColumnFloat(qr, "").has_value());
    EXPECT_FALSE(dynxxSQLiteQueryReadColumnFloat(0, "v").has_value());
    dynxxSQLiteQueryDrop(qr);
    dynxxSQLiteClose(conn);
}

TEST_F(DynXXSQLiteTestSuite, QueryReadColumnWrongType) {
    const auto conn = dynxxSQLiteOpen("sqlite_col_wrong_type");
    ASSERT_NE(conn, 0U);

    ASSERT_TRUE(dynxxSQLiteExecute(conn, kDropTableSql));
    ASSERT_TRUE(dynxxSQLiteExecute(conn, kCreateTextTableSql));
    ASSERT_TRUE(dynxxSQLiteExecute(conn, kDeleteAllRowsSql));
    ASSERT_TRUE(dynxxSQLiteExecute(conn, kInsertTextRowSql));
    const auto textQr = dynxxSQLiteQueryDo(conn, kSelectSingleRowSql);
    ASSERT_NE(textQr, 0U);
    ASSERT_TRUE(dynxxSQLiteQueryReadRow(textQr));
    EXPECT_FALSE(dynxxSQLiteQueryReadColumnInteger(textQr, "v").has_value());
    EXPECT_FALSE(dynxxSQLiteQueryReadColumnFloat(textQr, "v").has_value());
    dynxxSQLiteQueryDrop(textQr);

    ASSERT_TRUE(dynxxSQLiteExecute(conn, kDropTableSql));
    ASSERT_TRUE(dynxxSQLiteExecute(conn, kCreateIntegerTableSql));
    ASSERT_TRUE(dynxxSQLiteExecute(conn, kDeleteAllRowsSql));
    ASSERT_TRUE(dynxxSQLiteExecute(conn, kInsertIntegerRowSql));
    const auto intQr = dynxxSQLiteQueryDo(conn, kSelectSingleRowSql);
    ASSERT_NE(intQr, 0U);
    ASSERT_TRUE(dynxxSQLiteQueryReadRow(intQr));
    EXPECT_FALSE(dynxxSQLiteQueryReadColumnText(intQr, "v").has_value());
    EXPECT_FALSE(dynxxSQLiteQueryReadColumnFloat(intQr, "v").has_value());
    dynxxSQLiteQueryDrop(intQr);

    ASSERT_TRUE(dynxxSQLiteExecute(conn, kDropTableSql));
    ASSERT_TRUE(dynxxSQLiteExecute(conn, kCreateRealTableSql));
    ASSERT_TRUE(dynxxSQLiteExecute(conn, kDeleteAllRowsSql));
    ASSERT_TRUE(dynxxSQLiteExecute(conn, kInsertRealRowSql));
    const auto realQr = dynxxSQLiteQueryDo(conn, kSelectSingleRowSql);
    ASSERT_NE(realQr, 0U);
    ASSERT_TRUE(dynxxSQLiteQueryReadRow(realQr));
    EXPECT_FALSE(dynxxSQLiteQueryReadColumnText(realQr, "v").has_value());
    EXPECT_FALSE(dynxxSQLiteQueryReadColumnInteger(realQr, "v").has_value());
    dynxxSQLiteQueryDrop(realQr);

    dynxxSQLiteClose(conn);
}

TEST_F(DynXXSQLiteTestSuite, QueryDrop) {
    EXPECT_NO_THROW(dynxxSQLiteQueryDrop(0));
}

TEST_F(DynXXSQLiteTestSuite, Close) {
    EXPECT_NO_THROW(dynxxSQLiteClose(0));
}



