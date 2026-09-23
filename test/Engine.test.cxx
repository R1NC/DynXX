#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>
#include <utility>

#include <DynXX/CXX/DynXX.hxx>
#include <DynXX/CXX/Json.hxx>
#include <DynXX/CXX/KV.hxx>
#include <DynXX/CXX/Net.hxx>
#include <DynXX/CXX/SQLite.hxx>
#include <DynXX/CXX/Zip.hxx>

#include "TestUtil.hxx"

/**
 * The engine is a process-wide singleton that the global test environment sets up once, so its
 * released state was unreachable for every suite: `dynxxInit` -> `dynxxRelease` -> `dynxxInit`
 * never happened in-process and all the "not initialized" guards of the facade stayed dead.
 * This suite owns that round trip and pins down what has to come back afterwards.
 *
 * Releasing the singleton mid-run is intrusive, so two safety nets are installed: a scope guard
 * for ordinary assertion failures and a listener for the tests that abort outright.
 */
namespace {
    constexpr const char *ENGINE_KV_ID = "dynxx_ut_engine_kv";
    constexpr const char *ENGINE_DB_ID = "dynxx_ut_engine_db";
    constexpr const char *ENGINE_UNREACHABLE_URL = "http://127.0.0.1:1/";
    constexpr size_t ENGINE_HTTP_TIMEOUT = 1000;

    std::string engineTempPath(const char *name) {
        return (DynXX::TestUtil::resolveTempPath() / name).string();
    }

    // A failing assertion must not leave the singleton released for the rest of the run.
    // `dynxxInit` is a no-op while the engine is already initialized.
    class EngineReinitGuard final {
    public:
        explicit EngineReinitGuard(std::string root) : root_(std::move(root)) {}

        ~EngineReinitGuard() {
            dynxxInit(root_);
        }

        EngineReinitGuard(const EngineReinitGuard &) = delete;
        EngineReinitGuard &operator=(const EngineReinitGuard &) = delete;
        EngineReinitGuard(EngineReinitGuard &&) = delete;
        EngineReinitGuard &operator=(EngineReinitGuard &&) = delete;

    private:
        std::string root_;
    };

    // A structured exception skips the stack unwinding that would run the scope guard, so the
    // engine is also restored between tests by the framework itself.
    class EngineRecoveryListener final : public ::testing::EmptyTestEventListener {
    public:
        void OnTestEnd(const ::testing::TestInfo &) override {
            if (const auto root = dynxxRootPath(); root.has_value()) {
                root_ = *root;
                return;
            }
            if (!root_.empty()) {
                dynxxInit(root_);
            }
        }

    private:
        std::string root_;
    };
}

class DynXXEngineTestSuite : public ::testing::Test {
protected:
    static void SetUpTestSuite() {
        ::testing::UnitTest::GetInstance()->listeners().Append(new EngineRecoveryListener());
    }

    static std::string requireRootPath() {
        const auto rootPath = dynxxRootPath();
        EXPECT_TRUE(rootPath.has_value());
        return rootPath.value_or("");
    }
};

TEST_F(DynXXEngineTestSuite, InitIsIdempotentAndEmptyIdIsRejected) {
    const auto rootPath = dynxxRootPath();
    ASSERT_TRUE(rootPath.has_value());
    ASSERT_FALSE(rootPath->empty());

    // Initializing an already initialized engine is a no-op that still reports success.
    EXPECT_TRUE(dynxxInit(*rootPath));

    // An empty id is rejected before the store is touched.
    EXPECT_EQ(dynxxSQLiteOpen(""), 0U);

    // The engine is still usable after the redundant initialization.
    const auto sqlConn = dynxxSQLiteOpen("dynxx_ut_engine_idempotent");
    ASSERT_NE(sqlConn, 0U);
    dynxxSQLiteClose(sqlConn);
}

TEST_F(DynXXEngineTestSuite, ReleasedEngineShouldDegradeStoresAndCaches) {
    const auto rootPath = requireRootPath();
    ASSERT_FALSE(rootPath.empty());
    const EngineReinitGuard reinitGuard{rootPath};

    // Open both stores so that the release has something to close.
    const auto sqlConn = dynxxSQLiteOpen(ENGINE_DB_ID);
    ASSERT_NE(sqlConn, 0U);
    const auto kvConn = dynxxKVOpen(ENGINE_KV_ID);
    ASSERT_NE(kvConn, 0U);

    dynxxRelease();
    // A second release finds nothing left to tear down.
    dynxxRelease();

    // With the engine released, no entry point may dereference a null store. The handles
    // below are deliberately non-zero so the store-null operand is the one being taken.
    RecordProperty("step", "rootPath");
    EXPECT_FALSE(dynxxRootPath().has_value());

    RecordProperty("step", "jsonDecoder");
    EXPECT_EQ(dynxxJsonDecoderInit(R"({"k":"v"})"), 0U);
    // Only `Release` may be driven with a stale handle here: the readers guard on
    // `decoder == 0` alone and would dereference the missing cache instead.
    dynxxJsonDecoderRelease(1U);

    RecordProperty("step", "sqlite");
    EXPECT_EQ(dynxxSQLiteOpen(ENGINE_DB_ID), 0U);
    EXPECT_EQ(dynxxSQLiteQueryDo(1U, "SELECT 1"), 0U);
    dynxxSQLiteQueryDrop(1U);
    dynxxSQLiteClose(1U);

    RecordProperty("step", "kv");
    EXPECT_EQ(dynxxKVOpen(ENGINE_KV_ID), 0U);
    dynxxKVClose(1U);

    RecordProperty("step", "zip");
    EXPECT_EQ(dynxxZZipInit(), 0U);
    EXPECT_EQ(dynxxZUnzipInit(), 0U);
    // As with the JSON readers, `Input` guards on the zero handle alone and would dereference
    // the missing cache, so only the release paths are driven with a stale handle.
    dynxxZZipRelease(1U);
    dynxxZUnzipRelease(1U);

    // Re-initializing must hand every store and cache back.
    RecordProperty("step", "reinit");
    EXPECT_TRUE(dynxxInit(rootPath));
    EXPECT_EQ(dynxxRootPath().value_or(""), rootPath);

    RecordProperty("step", "jsonRestored");
    const auto jsonDecoder = dynxxJsonDecoderInit(R"({"s":"v"})");
    ASSERT_NE(jsonDecoder, 0U);
    const auto sNode = dynxxJsonDecoderReadNode(jsonDecoder, "s");
    ASSERT_NE(sNode, 0U);
    EXPECT_EQ(dynxxJsonDecoderReadString(jsonDecoder, sNode).value_or(""), "v");
    dynxxJsonDecoderRelease(jsonDecoder);

    // Reopening the same id proves the store came back rather than only its handle table.
    RecordProperty("step", "kvRestored");
    const auto kvConnRestored = dynxxKVOpen(ENGINE_KV_ID);
    ASSERT_NE(kvConnRestored, 0U);
    EXPECT_TRUE(dynxxKVWriteString(kvConnRestored, "k", "v"));
    EXPECT_EQ(dynxxKVReadString(kvConnRestored, "k").value_or(""), "v");
    dynxxKVClose(kvConnRestored);

    RecordProperty("step", "sqliteRestored");
    const auto sqlConnRestored = dynxxSQLiteOpen(ENGINE_DB_ID);
    ASSERT_NE(sqlConnRestored, 0U);
    dynxxSQLiteClose(sqlConnRestored);

    RecordProperty("step", "zipRestored");
    const auto zip = dynxxZZipInit();
    ASSERT_NE(zip, 0U);
    dynxxZZipRelease(zip);
}

TEST_F(DynXXEngineTestSuite, ReleasedEngineShouldDegradeHttp) {
    const auto rootPath = requireRootPath();
    ASSERT_FALSE(rootPath.empty());
    const EngineReinitGuard reinitGuard{rootPath};

    dynxxRelease();
    ASSERT_FALSE(dynxxRootPath().has_value());

    // A relative cert path can not be resolved without an engine root.
    RecordProperty("step", "certPath");
    dynxxNetHttpSetCertPath("certs/dynxx_ut_engine_ca.pem");

    RecordProperty("step", "download");
    EXPECT_FALSE(dynxxNetHttpDownload(ENGINE_UNREACHABLE_URL,
                                      engineTempPath("dynxx_ut_engine.tmp"),
                                      ENGINE_HTTP_TIMEOUT));

    EXPECT_TRUE(dynxxInit(rootPath));
    dynxxNetHttpSetCertPath("");
}

TEST_F(DynXXEngineTestSuite, FailedInitShouldLeaveEngineReleased) {
    const auto rootPath = requireRootPath();
    ASSERT_FALSE(rootPath.empty());
    const EngineReinitGuard reinitGuard{rootPath};

    dynxxRelease();
    ASSERT_FALSE(dynxxRootPath().has_value());

    RecordProperty("step", "emptyRoot");
    EXPECT_FALSE(dynxxInit(""));

    // `create_directories` fails when a path component is an existing file.
    RecordProperty("step", "uncreatableRoot");
    const auto blockerPath = engineTempPath("dynxx_ut_engine_blocker");
    {
        std::ofstream blockerFile(blockerPath);
        blockerFile << "dynxx";
    }
    ASSERT_TRUE(std::filesystem::exists(blockerPath));
    EXPECT_FALSE(dynxxInit((std::filesystem::path(blockerPath) / "nested").string()));

    // `dynxxInit` builds the caches before it validates the root, so the engine is released
    // once more before the caches are asserted to be gone.
    RecordProperty("step", "releaseAfterFailedInit");
    dynxxRelease();
    EXPECT_FALSE(dynxxRootPath().has_value());
    EXPECT_EQ(dynxxJsonDecoderInit(R"({"k":"v"})"), 0U);

    RecordProperty("step", "reinit");
    EXPECT_TRUE(dynxxInit(rootPath));
    EXPECT_EQ(dynxxRootPath().value_or(""), rootPath);

    std::error_code ec;
    std::filesystem::remove(blockerPath, ec);
}
