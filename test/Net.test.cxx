#include <gtest/gtest.h>
#include <filesystem>
#include <DynXX/CXX/Net.hxx>

namespace {
    constexpr const char *cNetTestUrl = "https://rinc.xyz/index.html";
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

TEST(Net, DynxxNetHttpDownload) {
    const auto outPath = std::filesystem::temp_directory_path() / "dynxx_net_test_index.html";
    if (!dynxxNetHttpDownload(cNetTestUrl, outPath.string())) {
        GTEST_SKIP();
    }
    EXPECT_TRUE(std::filesystem::exists(outPath));
    EXPECT_GT(std::filesystem::file_size(outPath), 0);
}
