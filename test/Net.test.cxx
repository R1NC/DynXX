#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <DynXX/C/Net.h>
#include <DynXX/CXX/Net.hxx>

#include "TestUtil.hxx"

class DynXXNetTestSuite : public ::testing::Test {
protected:
    void SetUp() override {
        dynxxNetHttpSetCertPath("");
        dynxxNetHttpSetProxy({});
        dynxxNetHttpSetDnsConfigs({});
    }
};

namespace {
    constexpr const char *cNetTestUrl = "https://rinc.xyz/index.html";
    // httpbin.org is chronically unstable (frequent 503s); postman-echo.com is the stable echo backend
    constexpr const char *cNetPostEchoUrl = "https://postman-echo.com/post";
    constexpr const char *cNetGetEchoUrl = "https://postman-echo.com/get?x=1";
    constexpr const char *cNetPutEchoUrl = "https://postman-echo.com/put";
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

    DynXXHttpResponse netRequestWithTimeout(std::string_view url, size_t timeout) {
        return dynxxNetHttpRequest(url, DynXXHttpMethodX::Get, "a=1", {}, {}, {}, {}, {}, nullptr, 0, timeout);
    }

    // echo backends differ in JSON spacing (httpbin: `"a": "1"`, postman-echo: `"a":"1"`); accept both
    bool hasJsonField(std::string_view data, std::string_view name, std::string_view value) {
        const std::string spaced = std::string("\"") + std::string(name) + "\": \"" + std::string(value) + "\"";
        const std::string compact = std::string("\"") + std::string(name) + "\":\"" + std::string(value) + "\"";
        return data.find(spaced) != std::string::npos || data.find(compact) != std::string::npos;
    }
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

TEST_F(DynXXNetTestSuite, HttpRequestGetWithExistingQueryShouldAppendParams) {
    const auto rsp = dynxxNetHttpRequest(cNetGetEchoUrl, DynXXHttpMethodX::Get, "a=2");
    if (rsp.code != HTTP_OK) {
        GTEST_SKIP();
    }
    EXPECT_EQ(rsp.contentType.find("application/json"), 0U);
    EXPECT_TRUE(hasJsonField(rsp.data, "x", "1"));
    EXPECT_TRUE(hasJsonField(rsp.data, "a", "2"));
}

TEST_F(DynXXNetTestSuite, HttpRequestPostWithUrlEncodedParams) {
    const auto rsp = dynxxNetHttpRequest(cNetPostEchoUrl, DynXXHttpMethodX::Post, "a=1&b=2");
    if (rsp.code != HTTP_OK) {
        GTEST_SKIP();
    }
    EXPECT_EQ(rsp.contentType.find("application/json"), 0U);
    EXPECT_TRUE(hasJsonField(rsp.data, "a", "1"));
    EXPECT_TRUE(hasJsonField(rsp.data, "b", "2"));
}

TEST_F(DynXXNetTestSuite, HttpRequestPostWithRawBody) {
    const Bytes rawBody{'r', 'a', 'w', '-', 'b', 'o', 'd', 'y'};
    const std::vector<std::string> headers{
        "Content-Type: text/plain"
    };
    const auto rsp = dynxxNetHttpRequest(
        cNetPostEchoUrl,
        DynXXHttpMethodX::Post,
        "",
        rawBody,
        headers
    );
    if (rsp.code != HTTP_OK) {
        GTEST_SKIP();
    }
    EXPECT_EQ(rsp.contentType.find("application/json"), 0U);
    EXPECT_NE(rsp.data.find("raw-body"), std::string::npos);
}

TEST_F(DynXXNetTestSuite, HttpRequestUploadFileShouldSucceed) {
    const auto uploadPath = DynXX::TestUtil::resolveTempPath() / "dynxx_net_upload.txt";
    std::ofstream out(uploadPath, std::ios::binary);
    ASSERT_TRUE(out.is_open());
    out << "upload-body";
    out.close();

    auto *file = std::fopen(uploadPath.string().c_str(), "rb");
    ASSERT_NE(file, nullptr);

    const auto rsp = dynxxNetHttpRequest(
        cNetPutEchoUrl,
        DynXXHttpMethodX::Put,
        "",
        {},
        {},
        {},
        {},
        {},
        file,
        std::filesystem::file_size(uploadPath)
    );
    std::fclose(file);

    if (rsp.code != HTTP_OK) {
        GTEST_SKIP();
    }
    EXPECT_EQ(rsp.contentType.find("application/json"), 0U);
    EXPECT_NE(rsp.data.find("upload-body"), std::string::npos);
}

TEST_F(DynXXNetTestSuite, DownloadInvalidOpenPathShouldReturnFalse) {
    const auto invalidDir = DynXX::TestUtil::resolveTempPath() / "dynxx_net_missing_parent";
    std::error_code ec;
    std::filesystem::remove_all(invalidDir, ec);
    const auto invalidFile = invalidDir / "index.html";
    EXPECT_FALSE(dynxxNetHttpDownload(cNetTestUrl, invalidFile.string()));
}

TEST_F(DynXXNetTestSuite, HttpSetCertPathInvalidFileShouldFailHttpsRequest) {
    dynxxNetHttpSetCertPath((DynXX::TestUtil::resolveTempPath() / "dynxx_no_such_ca.pem").string());
    const auto rsp = netRequestWithTimeout(cNetTestUrl, 1000);
    EXPECT_EQ(rsp.code, 0);
    EXPECT_TRUE(rsp.data.empty());
}

TEST_F(DynXXNetTestSuite, HttpSetCertPathEmptyShouldRestoreRequest) {
    dynxxNetHttpSetCertPath("");
    const auto rsp = netRequestWithTimeout(cNetTestUrl, 5000);
    if (rsp.code != HTTP_OK) {
        GTEST_SKIP();
    }
    EXPECT_FALSE(rsp.data.empty());
}

TEST_F(DynXXNetTestSuite, HttpSetProxyUnreachableShouldFailRequest) {
    dynxxNetHttpSetProxy({.host = "127.0.0.1", .port = 1});
    const auto rsp = netRequestWithTimeout(cNetTestUrl, 1000);
    dynxxNetHttpSetProxy({});
    EXPECT_EQ(rsp.code, 0);
    EXPECT_TRUE(rsp.data.empty());
}

TEST_F(DynXXNetTestSuite, HttpSetDnsConfigsOverrideShouldFailRequest) {
    dynxxNetHttpSetDnsConfigs({{.host = "rinc.xyz", .port = 443, .address = "127.0.0.1"}});
    const auto rsp = netRequestWithTimeout(cNetTestUrl, 1000);
    dynxxNetHttpSetDnsConfigs({});
    EXPECT_EQ(rsp.code, 0);
    EXPECT_TRUE(rsp.data.empty());
}

TEST_F(DynXXNetTestSuite, HttpSetDnsConfigsReplaceShouldRebuildAndFailRequest) {
    dynxxNetHttpSetDnsConfigs({{.host = "rinc.xyz", .port = 443, .address = "192.0.2.1"}});
    dynxxNetHttpSetDnsConfigs({{.host = "rinc.xyz", .port = 443, .address = "127.0.0.1"}});
    const auto rsp = netRequestWithTimeout(cNetTestUrl, 1000);
    dynxxNetHttpSetDnsConfigs({});
    EXPECT_EQ(rsp.code, 0);
    EXPECT_TRUE(rsp.data.empty());
}

TEST_F(DynXXNetTestSuite, HttpCSetNullConfigsShouldBeSafe) {
    dynxx_net_http_set_cert_path(nullptr);
    dynxx_net_http_set_proxy(nullptr);
    dynxx_net_http_set_dns_configs(nullptr, 0);
    const auto rsp = dynxxNetHttpRequest(cNetTestUrl, DynXXHttpMethodX::Get, "a=1");
    if (rsp.code != HTTP_OK) {
        GTEST_SKIP();
    }
    EXPECT_FALSE(rsp.data.empty());
}

TEST_F(DynXXNetTestSuite, HttpCSetProxyUnreachableShouldFailRequest) {
    const DynXXHttpProxyConfig proxy{"127.0.0.1", 1, nullptr, nullptr};
    dynxx_net_http_set_proxy(&proxy);
    const auto rsp = netRequestWithTimeout(cNetTestUrl, 1000);
    dynxx_net_http_set_proxy(nullptr);
    EXPECT_EQ(rsp.code, 0);
    EXPECT_TRUE(rsp.data.empty());
}

TEST_F(DynXXNetTestSuite, HttpCSetDnsConfigsOverrideShouldFailRequest) {
    const DynXXHttpDnsConfig configs[]{{"rinc.xyz", 443, "127.0.0.1"}};
    dynxx_net_http_set_dns_configs(configs, 1);
    const auto rsp = netRequestWithTimeout(cNetTestUrl, 1000);
    dynxx_net_http_set_dns_configs(nullptr, 0);
    EXPECT_EQ(rsp.code, 0);
    EXPECT_TRUE(rsp.data.empty());
}

TEST_F(DynXXNetTestSuite, HttpCSetCertPathInvalidShouldFailHttpsRequest) {
    const auto certPath = (DynXX::TestUtil::resolveTempPath() / "dynxx_no_such_ca.pem").string();
    dynxx_net_http_set_cert_path(certPath.c_str());
    const auto rsp = netRequestWithTimeout(cNetTestUrl, 1000);
    dynxx_net_http_set_cert_path(nullptr);
    EXPECT_EQ(rsp.code, 0);
    EXPECT_TRUE(rsp.data.empty());
}
