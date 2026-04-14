#include <gtest/gtest.h>
#include <filesystem>
#include <DynXX/CXX/Net.hxx>

namespace {
    constexpr const char *cNetTestUrl = "https://rinc.xyz/index.html";
    constexpr const char *cNetPostEchoUrl = "https://httpbin.org/post";
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
