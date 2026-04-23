#include <gtest/gtest.h>
#include <DynXX/CXX/Crypto.hxx>

class DynXXCryptoTestSuite : public ::testing::Test {};

TEST_F(DynXXCryptoTestSuite, Rand) {
    constexpr auto kRandSize = 32;
    constexpr auto kRounds = 6;

    auto previous = dynxxCryptoRand(kRandSize);
    ASSERT_EQ(previous.size(), kRandSize);

    bool hasDifference = false;
    for (int i = 0; i < kRounds; ++i) {
        auto current = dynxxCryptoRand(kRandSize);
        ASSERT_EQ(current.size(), kRandSize);
        if (current != previous) {
            hasDifference = true;
        }
        previous = std::move(current);
    }

    EXPECT_TRUE(hasDifference);
}



