#include <gtest/gtest.h>
#include <filesystem>
#include <DynXX/CXX/Net.hxx>

#include "TestUtil.hxx"

class DynXXNetTestSuite : public ::testing::Test {};

namespace {
    constexpr const char *cNetTestUrl = "https://rinc.xyz/index.html";
    constexpr const char *cNetPostEchoUrl = "https://httpbin.org/post";
    constexpr auto HTTP_OK = 200;

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
    rsp.code = HTTP_OK;
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
    if (rsp.code != HTTP_OK) {
        GTEST_SKIP();
    }
    EXPECT_FALSE(rsp.data.empty());
}

TEST_F(DynXXNetTestSuite, HttpRequestWithDictParams) {
    const DictAny params{{"a", 1Z}};
    const auto rsp = dynxxNetHttpRequest(cNetTestUrl, DynXXHttpMethodX::Get, params);
    if (rsp.code != HTTP_OK) {
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
    if (rsp.code != HTTP_OK) {
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

TEST_F(DynXXNetTestSuite, RequestInvalidParamStringParamsOverloadShouldFastFail) {
    // Hard-invalid inputs should be rejected by DynXX net wrapper before hitting transport.
    for (const auto &param : {
        NetRequestInvalidParamCase{.url="", .method=DynXXHttpMethodX::Get, .params="a=1"},
        NetRequestInvalidParamCase{.url="", .method=DynXXHttpMethodX::Post, .params="a=1"},
        NetRequestInvalidParamCase{.url="ht!tp://example.com", .method=DynXXHttpMethodX::Get, .params="a=1"},
        NetRequestInvalidParamCase{.url="http://", .method=DynXXHttpMethodX::Get, .params="a=1"}
    }) {
        const auto rsp = dynxxNetHttpRequest(param.url, param.method, param.params);
        EXPECT_EQ(rsp.code, 0);
        EXPECT_TRUE(rsp.data.empty());
        EXPECT_TRUE(rsp.headers.empty());
    }
}

TEST_F(DynXXNetTestSuite, RequestInvalidParamDictParamsOverloadShouldFastFail) {
    for (const auto &param : {
        NetRequestInvalidParamCase{.url="", .method=DynXXHttpMethodX::Get, .params="a=1"},
        NetRequestInvalidParamCase{.url="", .method=DynXXHttpMethodX::Post, .params="a=1"},
        NetRequestInvalidParamCase{.url="ht!tp://example.com", .method=DynXXHttpMethodX::Get, .params="a=1"},
        NetRequestInvalidParamCase{.url="http://", .method=DynXXHttpMethodX::Get, .params="a=1"}
    }) {
        const DictAny dictParams{{"a", 1Z}};
        const auto rsp = dynxxNetHttpRequest(param.url, param.method, dictParams);
        EXPECT_EQ(rsp.code, 0);
        EXPECT_TRUE(rsp.data.empty());
        EXPECT_TRUE(rsp.headers.empty());
    }
}

TEST_F(DynXXNetTestSuite, RequestInvalidQueryFormatStringParamsOverloadShouldBeHandledGracefully) {
    // Query encoding issues may be tolerated by underlying URL/curl stack; assert graceful handling, not forced fast-fail.
    for (const auto &param : {
        NetRequestInvalidParamCase{.url="https://example.com/?a=%", .method=DynXXHttpMethodX::Get, .params=""},
        NetRequestInvalidParamCase{.url="https://example.com/?a=%2", .method=DynXXHttpMethodX::Get, .params=""}
    }) {
        const auto rsp = dynxxNetHttpRequest(param.url, param.method, param.params);
        EXPECT_GE(rsp.code, 0);
        if (rsp.code == 0) {
            EXPECT_TRUE(rsp.data.empty());
            EXPECT_TRUE(rsp.headers.empty());
        }
    }
}

TEST_F(DynXXNetTestSuite, RequestInvalidQueryFormatDictParamsOverloadShouldBeHandledGracefully) {
    for (const auto &param : {
        NetRequestInvalidParamCase{.url="https://example.com/?a=%", .method=DynXXHttpMethodX::Get, .params=""},
        NetRequestInvalidParamCase{.url="https://example.com/?a=%2", .method=DynXXHttpMethodX::Get, .params=""}
    }) {
        const DictAny dictParams{{"a", 1Z}};
        const auto rsp = dynxxNetHttpRequest(param.url, param.method, dictParams);
        EXPECT_GE(rsp.code, 0);
        if (rsp.code == 0) {
            EXPECT_TRUE(rsp.data.empty());
            EXPECT_TRUE(rsp.headers.empty());
        }
    }
}

TEST_F(DynXXNetTestSuite, DownloadInvalidParamShouldReturnFalse) {
    // Hard-invalid inputs should fail fast at API boundary.
    for (const auto &param : {
        NetDownloadInvalidParamCase{.url="", .filePath="x.tmp"},
        NetDownloadInvalidParamCase{.url="https://rinc.xyz/index.html", .filePath=""},
        NetDownloadInvalidParamCase{.url="", .filePath=""},
        NetDownloadInvalidParamCase{.url="ht!tp://example.com", .filePath="x.tmp"},
        NetDownloadInvalidParamCase{.url="http://", .filePath="x.tmp"}
    }) {
        EXPECT_FALSE(dynxxNetHttpDownload(param.url, param.filePath));
    }
}

TEST_F(DynXXNetTestSuite, DownloadInvalidQueryFormatShouldBeHandledGracefully) {
    // Keep this as tolerant behavior check because lower layers may normalize/recover malformed query escapes.
    for (const auto &param : {
        NetDownloadInvalidParamCase{.url="https://example.com/?a=%", .filePath="dynxx_invalid_query_1.tmp"},
        NetDownloadInvalidParamCase{.url="https://example.com/?a=%2", .filePath="dynxx_invalid_query_2.tmp"}
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


