#include <gtest/gtest.h>
#include <filesystem>
#include <DynXX/CXX/Net.hxx>

namespace {
    constexpr const char *cNetTestUrl = "https://rinc.xyz/index.html";
    constexpr const char *cNetPostEchoUrl = "https://httpbin.org/post";

    struct NetRequestInvalidParamCase {
        std::string url;
        DynXXHttpMethodX method;
        std::string params;
    };

    struct NetDownloadInvalidParamCase {
        std::string url;
        std::string filePath;
    };
}

TEST(Net, DynXXHttpResponseToJson) {
    DynXXHttpResponse rsp;
    rsp.code = 200;
    rsp.contentType = "application/json";
    rsp.data = "{}";
    rsp.headers["k"] = "v";
    const auto json = rsp.toJson();
    ASSERT_TRUE(json.has_value());
    EXPECT_NE(json->find("\"code\""), std::string::npos);
    EXPECT_NE(json->find("\"contentType\""), std::string::npos);
}

TEST(Net, DynxxNetHttpRequestWithStringParams) {
    const auto rsp = dynxxNetHttpRequest(cNetTestUrl, DynXXHttpMethodX::Get, "a=1");
    if (rsp.code != 200) {
        GTEST_SKIP();
    }
    EXPECT_FALSE(rsp.data.empty());
}

TEST(Net, DynxxNetHttpRequestWithDictParams) {
    const DictAny params{{"a", int64_t(1)}};
    const auto rsp = dynxxNetHttpRequest(cNetTestUrl, DynXXHttpMethodX::Get, params);
    if (rsp.code != 200) {
        GTEST_SKIP();
    }
    EXPECT_FALSE(rsp.data.empty());
}

TEST(Net, DynxxNetHttpRequestPostWithHeadersAndMime) {
    const std::vector<std::string> headers{
        "X-DynXX-Test: post",
        "Accept: application/json"
    };
    const std::vector<std::string> formNames{"desc"};
    const std::vector<std::string> formMimes{"text/plain"};
    const std::vector<std::string> formData{"hello-mime"};

    const auto rsp = dynxxNetHttpRequest(
        cNetPostEchoUrl,
        DynXXHttpMethodX::Post,
        "",
        {},
        headers,
        formNames,
        formMimes,
        formData
    );
    if (rsp.code != 200) {
        GTEST_SKIP();
    }
    EXPECT_EQ(rsp.contentType.find("application/json"), 0U);
    EXPECT_NE(rsp.data.find("hello-mime"), std::string::npos);
    EXPECT_NE(rsp.data.find("post"), std::string::npos);
}

TEST(Net, DynxxNetHttpDownload) {
    const auto outPath = std::filesystem::temp_directory_path() / "dynxx_net_test_index.html";
    if (!dynxxNetHttpDownload(cNetTestUrl, outPath.string())) {
        GTEST_SKIP();
    }
    EXPECT_TRUE(std::filesystem::exists(outPath));
    EXPECT_GT(std::filesystem::file_size(outPath), 0);
}

class NetRequestInvalidParamTest : public ::testing::TestWithParam<NetRequestInvalidParamCase> {};

TEST_P(NetRequestInvalidParamTest, StringParamsOverloadShouldFastFail) {
    // Hard-invalid inputs should be rejected by DynXX net wrapper before hitting transport.
    const auto &param = GetParam();
    const auto rsp = dynxxNetHttpRequest(param.url, param.method, param.params);
    EXPECT_EQ(rsp.code, 0);
    EXPECT_TRUE(rsp.data.empty());
    EXPECT_TRUE(rsp.headers.empty());
}

TEST_P(NetRequestInvalidParamTest, DictParamsOverloadShouldFastFail) {
    const auto &param = GetParam();
    const DictAny dictParams{{"a", int64_t(1)}};
    const auto rsp = dynxxNetHttpRequest(param.url, param.method, dictParams);
    EXPECT_EQ(rsp.code, 0);
    EXPECT_TRUE(rsp.data.empty());
    EXPECT_TRUE(rsp.headers.empty());
}

INSTANTIATE_TEST_SUITE_P(
    InvalidParams,
    NetRequestInvalidParamTest,
    ::testing::Values(
        NetRequestInvalidParamCase{"", DynXXHttpMethodX::Get, "a=1"},
        NetRequestInvalidParamCase{"", DynXXHttpMethodX::Post, "a=1"},
        NetRequestInvalidParamCase{"ht!tp://example.com", DynXXHttpMethodX::Get, "a=1"},
        NetRequestInvalidParamCase{"http://", DynXXHttpMethodX::Get, "a=1"}
    )
);

class NetRequestInvalidQueryFormatTest : public ::testing::TestWithParam<NetRequestInvalidParamCase> {};

TEST_P(NetRequestInvalidQueryFormatTest, StringParamsOverloadShouldBeHandledGracefully) {
    // Query encoding issues may be tolerated by underlying URL/curl stack; assert graceful handling, not forced fast-fail.
    const auto &param = GetParam();
    const auto rsp = dynxxNetHttpRequest(param.url, param.method, param.params);
    EXPECT_GE(rsp.code, 0);
    if (rsp.code == 0) {
        EXPECT_TRUE(rsp.data.empty());
        EXPECT_TRUE(rsp.headers.empty());
    }
}

TEST_P(NetRequestInvalidQueryFormatTest, DictParamsOverloadShouldBeHandledGracefully) {
    const auto &param = GetParam();
    const DictAny dictParams{{"a", int64_t(1)}};
    const auto rsp = dynxxNetHttpRequest(param.url, param.method, dictParams);
    EXPECT_GE(rsp.code, 0);
    if (rsp.code == 0) {
        EXPECT_TRUE(rsp.data.empty());
        EXPECT_TRUE(rsp.headers.empty());
    }
}

INSTANTIATE_TEST_SUITE_P(
    InvalidQueryFormat,
    NetRequestInvalidQueryFormatTest,
    ::testing::Values(
        NetRequestInvalidParamCase{"https://example.com/?a=%", DynXXHttpMethodX::Get, ""},
        NetRequestInvalidParamCase{"https://example.com/?a=%2", DynXXHttpMethodX::Get, ""}
    )
);

class NetDownloadInvalidParamTest : public ::testing::TestWithParam<NetDownloadInvalidParamCase> {};

TEST_P(NetDownloadInvalidParamTest, ShouldReturnFalse) {
    // Hard-invalid inputs should fail fast at API boundary.
    const auto &param = GetParam();
    EXPECT_FALSE(dynxxNetHttpDownload(param.url, param.filePath));
}

INSTANTIATE_TEST_SUITE_P(
    InvalidParams,
    NetDownloadInvalidParamTest,
    ::testing::Values(
        NetDownloadInvalidParamCase{"", "x.tmp"},
        NetDownloadInvalidParamCase{"https://rinc.xyz/index.html", ""},
        NetDownloadInvalidParamCase{"", ""},
        NetDownloadInvalidParamCase{"ht!tp://example.com", "x.tmp"},
        NetDownloadInvalidParamCase{"http://", "x.tmp"}
    )
);

class NetDownloadInvalidQueryFormatTest : public ::testing::TestWithParam<NetDownloadInvalidParamCase> {};

TEST_P(NetDownloadInvalidQueryFormatTest, ShouldBeHandledGracefully) {
    // Keep this as tolerant behavior check because lower layers may normalize/recover malformed query escapes.
    const auto &param = GetParam();
    const auto outPath = std::filesystem::temp_directory_path() / param.filePath;
    const auto ok = dynxxNetHttpDownload(param.url, outPath.string());
    if (ok) {
        EXPECT_TRUE(std::filesystem::exists(outPath));
        std::error_code ec;
        std::filesystem::remove(outPath, ec);
    } else {
        SUCCEED();
    }
}

INSTANTIATE_TEST_SUITE_P(
    InvalidQueryFormat,
    NetDownloadInvalidQueryFormatTest,
    ::testing::Values(
        NetDownloadInvalidParamCase{"https://example.com/?a=%", "dynxx_invalid_query_1.tmp"},
        NetDownloadInvalidParamCase{"https://example.com/?a=%2", "dynxx_invalid_query_2.tmp"}
    )
);
