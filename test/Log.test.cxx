#include <gtest/gtest.h>
#include <DynXX/CXX/Log.hxx>

TEST(Log, DynxxLogSetLevel) {
    EXPECT_NO_THROW(dynxxLogSetLevel(DynXXLogLevelX::Debug));
}

TEST(Log, DynxxLogSetCallback) {
    bool called = false;
    dynxxLogSetCallback([&called](int, const char *) {
        called = true;
    });
    dynxxLogPrint(DynXXLogLevelX::Error, "x");
    EXPECT_TRUE(called);
    dynxxLogSetCallback(nullptr);
}

TEST(Log, DynxxLogPrint) {
    EXPECT_NO_THROW(dynxxLogPrint(DynXXLogLevelX::Info, "dynxx-log"));
}

#if !defined(USE_STD_FORMAT)
TEST(Log, DynxxLogFormatT) {
    EXPECT_EQ(dynxxLogFormatT("{}:{}:{}", "a", 1, 2.5), "a:1:2.5");
}
#endif

TEST(Log, DynxxLogPrintF) {
    std::string msg;
    dynxxLogSetCallback([&msg](int, const char *content) {
        msg = content == nullptr ? "" : content;
    });
    dynxxLogPrintF(DynXXLogLevelX::Debug, "{}", "probe");
    EXPECT_NE(msg.find("probe"), std::string::npos);
    dynxxLogSetCallback(nullptr);
}
