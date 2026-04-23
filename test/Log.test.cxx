#include <gtest/gtest.h>
#include <DynXX/CXX/Log.hxx>

class DynXXLogTestSuite : public ::testing::Test {};

TEST_F(DynXXLogTestSuite, SetLevel) {
    EXPECT_NO_THROW(dynxxLogSetLevel(DynXXLogLevelX::Debug));
}

TEST_F(DynXXLogTestSuite, SetCallback) {
    bool called = false;
    dynxxLogSetCallback([&called](int, const char *) {
        called = true;
    });
    dynxxLogPrint(DynXXLogLevelX::Error, "x");
    EXPECT_TRUE(called);
    dynxxLogSetCallback(nullptr);
}

TEST_F(DynXXLogTestSuite, Print) {
    EXPECT_NO_THROW(dynxxLogPrint(DynXXLogLevelX::Info, "dynxx-log"));
}

#if !defined(USE_STD_FORMAT)
TEST_F(DynXXLogTestSuite, FormatT) {
    EXPECT_EQ(dynxxLogFormatT("{}:{}:{}", "a", 1, 2.5), "a:1:2.5");
}
#endif

TEST_F(DynXXLogTestSuite, PrintF) {
    std::string msg;
    dynxxLogSetCallback([&msg](int, const char *content) {
        msg = content == nullptr ? "" : content;
    });
    dynxxLogPrintF(DynXXLogLevelX::Debug, "{}", "probe");
    EXPECT_NE(msg.find("probe"), std::string::npos);
    dynxxLogSetCallback(nullptr);
}



