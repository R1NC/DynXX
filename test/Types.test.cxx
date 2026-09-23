#include <gtest/gtest.h>

#include <string>

#include <DynXX/CXX/Types.hxx>

/**
 * `str2int32` / `str2float32` / `str2float128` are public helpers used to coerce
 * script-supplied text into numbers, falling back to the `Min*` sentinel when the text
 * cannot be represented. Nothing called the 32-bit integer and the float variants before,
 * so they were dead weight in the coverage report even though they are part of the
 * published header.
 *
 * Inputs are kept to forms where both backends (`std::from_chars` and the `std::sto*`
 * exceptions) agree, so the expectations hold on every toolchain.
 */
class DynXXTypesTestSuite : public ::testing::Test {};

TEST_F(DynXXTypesTestSuite, Str2Int32) {
    EXPECT_EQ(str2int32("42"), 42);
    EXPECT_EQ(str2int32("-7"), -7);

    // Empty text never reaches the parser.
    EXPECT_EQ(str2int32(""), MinInt32);
    EXPECT_EQ(str2int32("dynxx"), MinInt32);
    EXPECT_EQ(str2int32("999999999999"), MinInt32);  // beyond int32
    EXPECT_EQ(str2int32("dynxx", 7), 7);             // explicit fallback
}

TEST_F(DynXXTypesTestSuite, Str2Int64) {
    EXPECT_EQ(str2int64("9007199254740993"), 9007199254740993LL);
    EXPECT_EQ(str2int64(""), MinInt64);
    EXPECT_EQ(str2int64("dynxx"), MinInt64);
    EXPECT_EQ(str2int64("dynxx", 7), 7);
}

TEST_F(DynXXTypesTestSuite, Str2Float32) {
    EXPECT_FLOAT_EQ(str2float32("1.5"), 1.5F);
    EXPECT_FLOAT_EQ(str2float32("-0.25"), -0.25F);

    EXPECT_EQ(str2float32(""), MinFloat32);
    EXPECT_EQ(str2float32("dynxx"), MinFloat32);
    EXPECT_EQ(str2float32("1e999"), MinFloat32);  // beyond float
    EXPECT_FLOAT_EQ(str2float32("dynxx", 2.5F), 2.5F);
}

TEST_F(DynXXTypesTestSuite, Str2Float64) {
    EXPECT_DOUBLE_EQ(str2float64("1.5"), 1.5);
    EXPECT_EQ(str2float64(""), MinFloat64);
    EXPECT_EQ(str2float64("dynxx"), MinFloat64);
    EXPECT_DOUBLE_EQ(str2float64("dynxx", 2.5), 2.5);
}

TEST_F(DynXXTypesTestSuite, Str2Float128) {
    EXPECT_NEAR(str2float128("2.5"), 2.5L, 1e-12L);

    EXPECT_EQ(str2float128(""), MinFloat128);
    EXPECT_EQ(str2float128("dynxx"), MinFloat128);
    EXPECT_NEAR(str2float128("dynxx", 4.5L), 4.5L, 1e-12L);
}
