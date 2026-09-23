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

#if !defined(DYNXX_USE_STD_FORMAT)
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

TEST_F(DynXXLogTestSuite, SetLevelAllLevels) {
    // Every valid level must be accepted, including the ones that only the
    // file logger knows how to translate.
    for (const auto level : {
        DynXXLogLevelX::Debug,
        DynXXLogLevelX::Info,
        DynXXLogLevelX::Warn,
        DynXXLogLevelX::Error,
        DynXXLogLevelX::Fatal,
        DynXXLogLevelX::None
    }) {
        EXPECT_NO_THROW(dynxxLogSetLevel(level));
    }
    dynxxLogSetLevel(DynXXLogLevelX::Debug);
}

TEST_F(DynXXLogTestSuite, SetLevelOutOfRangeShouldBeIgnored) {
    dynxxLogSetLevel(DynXXLogLevelX::Debug);
    EXPECT_NO_THROW(dynxxLogSetLevel(static_cast<DynXXLogLevelX>(0)));
    EXPECT_NO_THROW(dynxxLogSetLevel(static_cast<DynXXLogLevelX>(255)));
    dynxxLogSetLevel(DynXXLogLevelX::Debug);
}

TEST_F(DynXXLogTestSuite, PrintAllLevels) {
    for (const auto level : {
        DynXXLogLevelX::Debug,
        DynXXLogLevelX::Info,
        DynXXLogLevelX::Warn,
        DynXXLogLevelX::Error,
        DynXXLogLevelX::Fatal,
        DynXXLogLevelX::None
    }) {
        EXPECT_NO_THROW(dynxxLogPrint(level, "dynxx-log-level"));
    }
}


