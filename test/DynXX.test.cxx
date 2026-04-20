#include <gtest/gtest.h>
#include <array>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>
#include <DynXX/CXX/DynXX.hxx>
#include <DynXXTest.hxx>

#include "../src/core/util/TimeUtil.hxx"
#include "TestUtil.hxx"

namespace {
    std::filesystem::path dynxxTestRoot() {
        return DynXX::TestUtil::resolveTempPath() / "dynxx_ut_gtest_root";
    }

    bool initEngine() {
        return dynxxInit(dynxxTestRoot().string());
    }

    void releaseEngine() {
        dynxxRelease();
    }

    class DynXXCoutListener final : public ::testing::EmptyTestEventListener {
    public:
        void OnTestProgramStart(const ::testing::UnitTest &unitTest) override {
            std::cout << "[==========] Running " << unitTest.test_to_run_count() << " tests" << std::endl;
        }

        void OnTestStart(const ::testing::TestInfo &testInfo) override {
            startNs_ = DynXX::Core::Util::Time::nowInNanoSecs();
            std::cout << "[ RUN      ] " << testInfo.test_suite_name() << "." << testInfo.name() << std::endl;
        }

        void OnTestEnd(const ::testing::TestInfo &testInfo) override {
            const auto elapsed = DynXX::Core::Util::Time::nowInNanoSecs() - startNs_;
            const char *status = testInfo.result()->Passed() ? "       OK " : "  FAILED  ";
            std::cout << "[" << status << "] " << testInfo.test_suite_name() << "." << testInfo.name()
                      << " (" << elapsed << " ns)" << std::endl;
        }

        void OnTestProgramEnd(const ::testing::UnitTest &unitTest) override {
            std::cout << "[==========] "
                      << unitTest.successful_test_count() << " passed, "
                      << unitTest.failed_test_count() << " failed"
                      << std::endl;
        }

    private:
        uint64_t startNs_{0};
    };

    class DynXXGlobalEnvironment final : public ::testing::Environment {
    public:
        void SetUp() override {
            std::filesystem::create_directories(dynxxTestRoot());
            if (!initEngine()) {
                std::cout << "@_@ init failed" << std::endl;
                throw std::runtime_error("dynxxInit failed in global SetUp");
            }
        }

        void TearDown() override {
            releaseEngine();
        }
    };
}

namespace DynXX::Test {
    int RunAll() {
        int argc = 1;
        std::string arg0 = "DynXXCxxTests";
        std::array<char *, 2> argv{arg0.data(), nullptr};
        ::testing::InitGoogleTest(&argc, argv.data());

        if (const auto envFilter = DynXX::TestUtil::envValue("DYNXX_GTEST_FILTER"); !envFilter.empty()) {
            ::testing::GTEST_FLAG(filter) = envFilter;
        } else if (::testing::GTEST_FLAG(filter).empty()) {
            ::testing::GTEST_FLAG(filter) = "*";
        }

        if (const auto envOutput = DynXX::TestUtil::envValue("DYNXX_GTEST_OUTPUT"); !envOutput.empty()) {
            ::testing::GTEST_FLAG(output) = envOutput;
        } else if (::testing::GTEST_FLAG(output).empty()) {
            ::testing::GTEST_FLAG(output) = "xml:dynxx-gtest-report.xml";
        }

        ::testing::GTEST_FLAG(print_time) = true;
        ::testing::GTEST_FLAG(throw_on_failure) = false;
        ::testing::UnitTest::GetInstance()->listeners().Append(new DynXXCoutListener());

        ::testing::AddGlobalTestEnvironment(new DynXXGlobalEnvironment());
        return RUN_ALL_TESTS();
    }
}

TEST(DynXX, DynxxGetVersion) {
    const auto version = dynxxGetVersion();
    EXPECT_FALSE(version.empty());
    EXPECT_NE(version.find('.'), std::string::npos);
}

TEST(DynXX, DynxxRootPath) {
    const auto path = dynxxRootPath();
    EXPECT_TRUE(path.has_value());
    EXPECT_FALSE(path.value_or("").empty());
}
