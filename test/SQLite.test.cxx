#include <gtest/gtest.h>
#include <DynXX/CXX/SQLite.hxx>

namespace {
    constexpr auto kCreateTextTableSql = "CREATE TABLE IF NOT EXISTS t (v TEXT);";
    constexpr auto kCreateIntegerTableSql = "CREATE TABLE IF NOT EXISTS t (v INTEGER);";
    constexpr auto kCreateRealTableSql = "CREATE TABLE IF NOT EXISTS t (v REAL);";
    constexpr auto kDeleteAllRowsSql = "DELETE FROM t;";
    constexpr auto kInsertTextRowSql = "INSERT INTO t (v) VALUES ('x');";
    constexpr auto kInsertIntegerRowSql = "INSERT INTO t (v) VALUES (7);";
    constexpr auto kInsertRealRowSql = "INSERT INTO t (v) VALUES (3.14);";
    constexpr auto kSelectSingleRowSql = "SELECT v FROM t LIMIT 1;";
    constexpr auto kSelectAllRowsSql = "SELECT v FROM t;";
}

TEST(SQLite, DynxxSQLiteOpen) {
    const auto conn = dynxxSQLiteOpen("sqlite_open");
    ASSERT_NE(conn, 0U);
    dynxxSQLiteClose(conn);
}

TEST(SQLite, DynxxSQLiteExecute) {
    const auto conn = dynxxSQLiteOpen("sqlite_execute");
    ASSERT_NE(conn, 0U);
    EXPECT_TRUE(dynxxSQLiteExecute(conn, kCreateTextTableSql));
    dynxxSQLiteClose(conn);
}

TEST(SQLite, DynxxSQLiteQueryDo) {
    const auto conn = dynxxSQLiteOpen("sqlite_query_do");
    ASSERT_NE(conn, 0U);
    ASSERT_TRUE(dynxxSQLiteExecute(conn, kCreateTextTableSql));
    ASSERT_TRUE(dynxxSQLiteExecute(conn, kInsertTextRowSql));
    const auto qr = dynxxSQLiteQueryDo(conn, kSelectSingleRowSql);
    EXPECT_NE(qr, 0U);
    dynxxSQLiteQueryDrop(qr);
    dynxxSQLiteClose(conn);
}

TEST(SQLite, DynxxSQLiteQueryReadRow) {
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

TEST(SQLite, DynxxSQLiteQueryReadColumnText) {
    const auto conn = dynxxSQLiteOpen("sqlite_col_text");
    ASSERT_NE(conn, 0U);
    ASSERT_TRUE(dynxxSQLiteExecute(conn, kCreateTextTableSql));
    ASSERT_TRUE(dynxxSQLiteExecute(conn, kDeleteAllRowsSql));
    ASSERT_TRUE(dynxxSQLiteExecute(conn, kInsertTextRowSql));
    const auto qr = dynxxSQLiteQueryDo(conn, kSelectSingleRowSql);
    ASSERT_NE(qr, 0U);
    ASSERT_TRUE(dynxxSQLiteQueryReadRow(qr));
    EXPECT_EQ(dynxxSQLiteQueryReadColumnText(qr, "v").value_or(""), "x");
    dynxxSQLiteQueryDrop(qr);
    dynxxSQLiteClose(conn);
}

TEST(SQLite, DynxxSQLiteQueryReadColumnInteger) {
    const auto conn = dynxxSQLiteOpen("sqlite_col_int");
    ASSERT_NE(conn, 0U);
    ASSERT_TRUE(dynxxSQLiteExecute(conn, kCreateIntegerTableSql));
    ASSERT_TRUE(dynxxSQLiteExecute(conn, kDeleteAllRowsSql));
    ASSERT_TRUE(dynxxSQLiteExecute(conn, kInsertIntegerRowSql));
    const auto qr = dynxxSQLiteQueryDo(conn, kSelectSingleRowSql);
    ASSERT_NE(qr, 0U);
    ASSERT_TRUE(dynxxSQLiteQueryReadRow(qr));
    EXPECT_EQ(dynxxSQLiteQueryReadColumnInteger(qr, "v").value_or(0), 7);
    dynxxSQLiteQueryDrop(qr);
    dynxxSQLiteClose(conn);
}

TEST(SQLite, DynxxSQLiteQueryReadColumnFloat) {
    const auto conn = dynxxSQLiteOpen("sqlite_col_float");
    ASSERT_NE(conn, 0U);
    ASSERT_TRUE(dynxxSQLiteExecute(conn, kCreateRealTableSql));
    ASSERT_TRUE(dynxxSQLiteExecute(conn, kDeleteAllRowsSql));
    ASSERT_TRUE(dynxxSQLiteExecute(conn, kInsertRealRowSql));
    const auto qr = dynxxSQLiteQueryDo(conn, kSelectSingleRowSql);
    ASSERT_NE(qr, 0U);
    ASSERT_TRUE(dynxxSQLiteQueryReadRow(qr));
    EXPECT_NEAR(dynxxSQLiteQueryReadColumnFloat(qr, "v").value_or(0.0), 3.14, 1e-9);
    dynxxSQLiteQueryDrop(qr);
    dynxxSQLiteClose(conn);
}

TEST(SQLite, DynxxSQLiteQueryDrop) {
    EXPECT_NO_THROW(dynxxSQLiteQueryDrop(0));
}

TEST(SQLite, DynxxSQLiteClose) {
    EXPECT_NO_THROW(dynxxSQLiteClose(0));
}
