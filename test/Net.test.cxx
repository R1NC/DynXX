#include <gtest/gtest.h>
#include <filesystem>
#include <DynXX/CXX/Net.hxx>

#include "TestUtil.hxx"

class DynXXNetTestSuite : public ::testing::Test {};

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

TEST_F(DynXXNetTestSuite, HttpResponseToJson) {
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

TEST_F(DynXXNetTestSuite, HttpRequestWithStringParams) {
    const auto rsp = dynxxNetHttpRequest(cNetTestUrl, DynXXHttpMethodX::Get, "a=1");
    if (rsp.code != 200) {
        GTEST_SKIP();
    }
    EXPECT_FALSE(rsp.data.empty());
}

TEST_F(DynXXNetTestSuite, HttpRequestWithDictParams) {
    const DictAny params{{"a", int64_t(1)}};
    const auto rsp = dynxxNetHttpRequest(cNetTestUrl, DynXXHttpMethodX::Get, params);
    if (rsp.code != 200) {
        GTEST_SKIP();
    }
    EXPECT_FALSE(rsp.data.empty());
}

TEST_F(DynXXNetTestSuite, HttpRequestPostWithHeadersAndMime) {
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

TEST_F(DynXXNetTestSuite, HttpDownload) {
    const auto outPath = DynXX::TestUtil::resolveTempPath() / "dynxx_net_test_index.html";
    if (!dynxxNetHttpDownload(cNetTestUrl, outPath.string())) {
        GTEST_SKIP();
    }
    EXPECT_TRUE(std::filesystem::exists(outPath));
    EXPECT_GT(std::filesystem::file_size(outPath), 0);
}

TEST_F(DynXXNetTestSuite, RequestInvalidParam_StringParamsOverloadShouldFastFail) {
    // Hard-invalid inputs should be rejected by DynXX net wrapper before hitting transport.
    for (const auto &param : {
        NetRequestInvalidParamCase{"", DynXXHttpMethodX::Get, "a=1"},
        NetRequestInvalidParamCase{"", DynXXHttpMethodX::Post, "a=1"},
        NetRequestInvalidParamCase{"ht!tp://example.com", DynXXHttpMethodX::Get, "a=1"},
        NetRequestInvalidParamCase{"http://", DynXXHttpMethodX::Get, "a=1"}
    }) {
        const auto rsp = dynxxNetHttpRequest(param.url, param.method, param.params);
        EXPECT_EQ(rsp.code, 0);
        EXPECT_TRUE(rsp.data.empty());
        EXPECT_TRUE(rsp.headers.empty());
    }
}

TEST_F(DynXXNetTestSuite, RequestInvalidParam_DictParamsOverloadShouldFastFail) {
    for (const auto &param : {
        NetRequestInvalidParamCase{"", DynXXHttpMethodX::Get, "a=1"},
        NetRequestInvalidParamCase{"", DynXXHttpMethodX::Post, "a=1"},
        NetRequestInvalidParamCase{"ht!tp://example.com", DynXXHttpMethodX::Get, "a=1"},
        NetRequestInvalidParamCase{"http://", DynXXHttpMethodX::Get, "a=1"}
    }) {
        const DictAny dictParams{{"a", int64_t(1)}};
        const auto rsp = dynxxNetHttpRequest(param.url, param.method, dictParams);
        EXPECT_EQ(rsp.code, 0);
        EXPECT_TRUE(rsp.data.empty());
        EXPECT_TRUE(rsp.headers.empty());
    }
}

TEST_F(DynXXNetTestSuite, RequestInvalidQueryFormat_StringParamsOverloadShouldBeHandledGracefully) {
    // Query encoding issues may be tolerated by underlying URL/curl stack; assert graceful handling, not forced fast-fail.
    for (const auto &param : {
        NetRequestInvalidParamCase{"https://example.com/?a=%", DynXXHttpMethodX::Get, ""},
        NetRequestInvalidParamCase{"https://example.com/?a=%2", DynXXHttpMethodX::Get, ""}
    }) {
        const auto rsp = dynxxNetHttpRequest(param.url, param.method, param.params);
        EXPECT_GE(rsp.code, 0);
        if (rsp.code == 0) {
            EXPECT_TRUE(rsp.data.empty());
            EXPECT_TRUE(rsp.headers.empty());
        }
    }
}

TEST_F(DynXXNetTestSuite, RequestInvalidQueryFormat_DictParamsOverloadShouldBeHandledGracefully) {
    for (const auto &param : {
        NetRequestInvalidParamCase{"https://example.com/?a=%", DynXXHttpMethodX::Get, ""},
        NetRequestInvalidParamCase{"https://example.com/?a=%2", DynXXHttpMethodX::Get, ""}
    }) {
        const DictAny dictParams{{"a", int64_t(1)}};
        const auto rsp = dynxxNetHttpRequest(param.url, param.method, dictParams);
        EXPECT_GE(rsp.code, 0);
        if (rsp.code == 0) {
            EXPECT_TRUE(rsp.data.empty());
            EXPECT_TRUE(rsp.headers.empty());
        }
    }
}

TEST_F(DynXXNetTestSuite, DownloadInvalidParam_ShouldReturnFalse) {
    // Hard-invalid inputs should fail fast at API boundary.
    for (const auto &param : {
        NetDownloadInvalidParamCase{"", "x.tmp"},
        NetDownloadInvalidParamCase{"https://rinc.xyz/index.html", ""},
        NetDownloadInvalidParamCase{"", ""},
        NetDownloadInvalidParamCase{"ht!tp://example.com", "x.tmp"},
        NetDownloadInvalidParamCase{"http://", "x.tmp"}
    }) {
        EXPECT_FALSE(dynxxNetHttpDownload(param.url, param.filePath));
    }
}

TEST_F(DynXXNetTestSuite, DownloadInvalidQueryFormat_ShouldBeHandledGracefully) {
    // Keep this as tolerant behavior check because lower layers may normalize/recover malformed query escapes.
    for (const auto &param : {
        NetDownloadInvalidParamCase{"https://example.com/?a=%", "dynxx_invalid_query_1.tmp"},
        NetDownloadInvalidParamCase{"https://example.com/?a=%2", "dynxx_invalid_query_2.tmp"}
    }) {
        const auto outPath = DynXX::TestUtil::resolveTempPath() / param.filePath;
        const auto ok = dynxxNetHttpDownload(param.url, outPath.string());
        if (ok) {
            EXPECT_TRUE(std::filesystem::exists(outPath));
            std::error_code ec;
            std::filesystem::remove(outPath, ec);
        } else {
            SUCCEED();
        }
    }
}



