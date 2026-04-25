#include <gtest/gtest.h>
#include <array>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <DynXX/CXX/DynXX.hxx>
#include <DynXXTest.hxx>

#include "../src/core/util/TimeUtil.hxx"
#include "TestUtil.hxx"

class DynXXTestSuite : public ::testing::Test {};

namespace {
    void initGoogleTestOnce() {
        static bool initialized = false;
        if (initialized) {
            return;
        }
        int argc = 1;
        std::string arg0 = "DynXXCxxTests";
        std::array<char *, 2> argv{arg0.data(), nullptr};
        ::testing::InitGoogleTest(&argc, argv.data());
        initialized = true;
    }

    void setupRunOptionsAndEnvironment();
    std::string resolveFilter(std::string_view explicitFilter);

    int runByFilter(std::string_view filter) {
        initGoogleTestOnce();
        setupRunOptionsAndEnvironment();
        ::testing::GTEST_FLAG(filter) = resolveFilter(filter);
        return RUN_ALL_TESTS();
    }

    std::string resolveFilter(std::string_view explicitFilter) {
        if (!explicitFilter.empty()) {
            return std::string(explicitFilter);
        }
        if (const auto envFilter = DynXX::TestUtil::envValue("DYNXX_GTEST_FILTER"); !envFilter.empty()) {
            return envFilter;
        }
        if (!::testing::GTEST_FLAG(filter).empty()) {
            return ::testing::GTEST_FLAG(filter);
        }
        return "*";
    }

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
            std::cout << "[==========] Running " << unitTest.test_to_run_count() << " tests" << '\n';
        }

        void OnTestStart(const ::testing::TestInfo &testInfo) override {
            startNs_ = DynXX::Core::Util::Time::nowInNanoSecs();
            std::cout << "[ RUN      ] " << testInfo.test_suite_name() << "." << testInfo.name() << '\n';
        }

        void OnTestEnd(const ::testing::TestInfo &testInfo) override {
            const auto elapsed = DynXX::Core::Util::Time::nowInNanoSecs() - startNs_;
            const char *status = testInfo.result()->Passed() ? "       OK " : "  FAILED  ";
            std::cout << "[" << status << "] " << testInfo.test_suite_name() << "." << testInfo.name()
                      << " (" << elapsed << " ns)" << '\n';
        }

        void OnTestProgramEnd(const ::testing::UnitTest &unitTest) override {
            std::cout << "[==========] "
                      << unitTest.successful_test_count() << " passed, "
                      << unitTest.failed_test_count() << " failed"
                      << '\n';
        }

    private:
        uint64_t startNs_{0};
    };

    class DynXXGlobalEnvironment final : public ::testing::Environment {
    public:
        void SetUp() override {
            std::filesystem::create_directories(dynxxTestRoot());
            if (!initEngine()) {
                std::cout << "TestEnv init failed" << '\n';
                throw std::runtime_error("dynxxInit failed in global SetUp");
            }
        }

        void TearDown() override {
            releaseEngine();
        }
    };

    void setupRunOptionsAndEnvironment() {
        if (const auto envOutput = DynXX::TestUtil::envValue("DYNXX_GTEST_OUTPUT"); !envOutput.empty()) {
            ::testing::GTEST_FLAG(output) = envOutput;
        } else if (::testing::GTEST_FLAG(output).empty()) {
            ::testing::GTEST_FLAG(output) = "xml:dynxx-gtest-report.xml";
        }

        ::testing::GTEST_FLAG(print_time) = true;
        ::testing::GTEST_FLAG(throw_on_failure) = false;

        static bool listenerInstalled = false;
        if (!listenerInstalled) {
            ::testing::UnitTest::GetInstance()->listeners().Append(new DynXXCoutListener());
            listenerInstalled = true;
        }

        static bool globalEnvAdded = false;
        if (!globalEnvAdded) {
            ::testing::AddGlobalTestEnvironment(new DynXXGlobalEnvironment());
            globalEnvAdded = true;
        }
    }
}

namespace DynXX::Test {
    namespace {
        struct TestCatalog {
            std::vector<std::string> suits;
            std::unordered_map<std::string, std::vector<std::string>> testsBySuit;
        };

        const TestCatalog &getCachedCatalog() {
            initGoogleTestOnce();
            static const TestCatalog cachedCatalog = [] {
                TestCatalog catalog;
                const auto *unitTest = ::testing::UnitTest::GetInstance();
                catalog.suits.reserve(static_cast<std::size_t>(unitTest->total_test_suite_count()));
                catalog.testsBySuit.reserve(static_cast<std::size_t>(unitTest->total_test_suite_count()));
                for (int suiteIndex = 0; suiteIndex < unitTest->total_test_suite_count(); ++suiteIndex) {
                    const auto *testSuite = unitTest->GetTestSuite(suiteIndex);
                    std::string suiteName = testSuite->name();
                    catalog.suits.push_back(suiteName);
                    auto &tests = catalog.testsBySuit[suiteName];
                    tests.reserve(static_cast<std::size_t>(testSuite->total_test_count()));
                    for (int caseIndex = 0; caseIndex < testSuite->total_test_count(); ++caseIndex) {
                        const auto *testInfo = testSuite->GetTestInfo(caseIndex);
                        tests.emplace_back(testInfo->name());
                    }
                }
                return catalog;
            }();
            return cachedCatalog;
        }
    }

    std::vector<std::string> listSuits() {
        return getCachedCatalog().suits;
    }

    std::vector<std::string> listTests(std::string_view suit) {
        if (suit.empty()) {
            return {};
        }
        const auto &catalog = getCachedCatalog();
        const auto it = catalog.testsBySuit.find(std::string(suit));
        if (it == catalog.testsBySuit.end()) {
            return {};
        }
        return it->second;
    }

    int runSuit(std::string_view suit) {
        if (suit.empty()) {
            return runAll();
        }
        return runByFilter(std::string(suit) + ".*");
    }

    int runTest(std::string_view suit, std::string_view test) {
        if (suit.empty() || test.empty()) {
            return runAll();
        }
        return runByFilter(std::string(suit) + "." + std::string(test));
    }

    int runAll() {
        return runByFilter("");
    }
}

TEST_F(DynXXTestSuite, GetVersion) {
    const auto version = dynxxGetVersion();
    EXPECT_FALSE(version.empty());
    EXPECT_NE(version.find('.'), std::string::npos);
}

TEST_F(DynXXTestSuite, RootPath) {
    const auto path = dynxxRootPath();
    EXPECT_TRUE(path.has_value());
    EXPECT_FALSE(path.value_or("").empty());
}

