#include <gtest/gtest.h>
#include <DynXX/CXX/Json.hxx>

TEST(Json, DynxxJsonNodeReadType) {
    const auto decoder = dynxxJsonDecoderInit(R"({"name":"dynxx"})");
    ASSERT_NE(decoder, 0U);
    const auto node = dynxxJsonDecoderReadNode(decoder, "name");
    ASSERT_NE(node, 0U);
    EXPECT_EQ(dynxxJsonNodeReadType(node), DynXXJsonNodeTypeX::String);
    dynxxJsonDecoderRelease(decoder);
}

TEST(Json, DynxxJsonNodeReadName) {
    const auto decoder = dynxxJsonDecoderInit(R"({"name":"dynxx"})");
    ASSERT_NE(decoder, 0U);
    const auto node = dynxxJsonDecoderReadNode(decoder, "name");
    ASSERT_NE(node, 0U);
    EXPECT_EQ(dynxxJsonNodeReadName(node).value_or(""), "name");
    dynxxJsonDecoderRelease(decoder);
}

TEST(Json, DynxxJsonNodeToStr) {
    const auto decoder = dynxxJsonDecoderInit(R"({"name":"dynxx"})");
    ASSERT_NE(decoder, 0U);
    const auto node = dynxxJsonDecoderReadNode(decoder, "name");
    ASSERT_NE(node, 0U);
    EXPECT_FALSE(dynxxJsonNodeToStr(node).value_or("").empty());
    dynxxJsonDecoderRelease(decoder);
}

TEST(Json, DynxxJsonFromDictAny) {
    const DictAny dict{
        {"name", std::string("dynxx")},
        {"n", int64_t(1)}
    };
    const auto json = dynxxJsonFromDictAny(dict);
    ASSERT_TRUE(json.has_value());
    EXPECT_NE(json->find("name"), std::string::npos);
}

TEST(Json, DynxxJsonToDictAny) {
    const auto dict = dynxxJsonToDictAny(R"({"name":"dynxx","n":1})");
    ASSERT_TRUE(dict.has_value());
    EXPECT_EQ(dict->count("name"), 1U);
}

TEST(Json, DynxxJsonDecoderInit) {
    const auto decoder = dynxxJsonDecoderInit(R"({"x":1})");
    EXPECT_NE(decoder, 0U);
    dynxxJsonDecoderRelease(decoder);
}

TEST(Json, DynxxJsonDecoderReadNode) {
    const auto decoder = dynxxJsonDecoderInit(R"({"x":1})");
    ASSERT_NE(decoder, 0U);
    EXPECT_NE(dynxxJsonDecoderReadNode(decoder, "x"), 0U);
    dynxxJsonDecoderRelease(decoder);
}

TEST(Json, DynxxJsonDecoderReadString) {
    const auto decoder = dynxxJsonDecoderInit(R"({"name":"dynxx"})");
    ASSERT_NE(decoder, 0U);
    const auto node = dynxxJsonDecoderReadNode(decoder, "name");
    ASSERT_NE(node, 0U);
    EXPECT_EQ(dynxxJsonDecoderReadString(decoder, node).value_or(""), "dynxx");
    dynxxJsonDecoderRelease(decoder);
}

TEST(Json, DynxxJsonDecoderReadInteger) {
    const auto decoder = dynxxJsonDecoderInit(R"({"n":7})");
    ASSERT_NE(decoder, 0U);
    const auto node = dynxxJsonDecoderReadNode(decoder, "n");
    ASSERT_NE(node, 0U);
    EXPECT_EQ(dynxxJsonDecoderReadInteger(decoder, node).value_or(0), 7);
    dynxxJsonDecoderRelease(decoder);
}

TEST(Json, DynxxJsonDecoderReadFloat) {
    const auto decoder = dynxxJsonDecoderInit(R"({"f":3.5})");
    ASSERT_NE(decoder, 0U);
    const auto node = dynxxJsonDecoderReadNode(decoder, "f");
    ASSERT_NE(node, 0U);
    EXPECT_NEAR(dynxxJsonDecoderReadFloat(decoder, node).value_or(0.0), 3.5, 1e-9);
    dynxxJsonDecoderRelease(decoder);
}

TEST(Json, DynxxJsonDecoderReadChild) {
    const auto decoder = dynxxJsonDecoderInit(R"({"arr":[1,2]})");
    ASSERT_NE(decoder, 0U);
    const auto arr = dynxxJsonDecoderReadNode(decoder, "arr");
    ASSERT_NE(arr, 0U);
    EXPECT_NE(dynxxJsonDecoderReadChild(decoder, arr), 0U);
    dynxxJsonDecoderRelease(decoder);
}

TEST(Json, DynxxJsonDecoderReadChildrenCount) {
    const auto decoder = dynxxJsonDecoderInit(R"({"arr":[1,2]})");
    ASSERT_NE(decoder, 0U);
    const auto arr = dynxxJsonDecoderReadNode(decoder, "arr");
    ASSERT_NE(arr, 0U);
    EXPECT_EQ(dynxxJsonDecoderReadChildrenCount(decoder, arr), 2U);
    dynxxJsonDecoderRelease(decoder);
}

TEST(Json, DynxxJsonDecoderReadChildren) {
    const auto decoder = dynxxJsonDecoderInit(R"({"arr":[1,2]})");
    ASSERT_NE(decoder, 0U);
    const auto arr = dynxxJsonDecoderReadNode(decoder, "arr");
    ASSERT_NE(arr, 0U);
    size_t count = 0;
    dynxxJsonDecoderReadChildren(
        decoder,
        [&count](size_t, DynXXJsonNodeHandle, DynXXJsonNodeTypeX, std::string_view) {
            count++;
        },
        arr
    );
    EXPECT_EQ(count, 2U);
    dynxxJsonDecoderRelease(decoder);
}

TEST(Json, DynxxJsonDecoderReadNext) {
    const auto decoder = dynxxJsonDecoderInit(R"({"arr":[1,2]})");
    ASSERT_NE(decoder, 0U);
    const auto arr = dynxxJsonDecoderReadNode(decoder, "arr");
    ASSERT_NE(arr, 0U);
    const auto child = dynxxJsonDecoderReadChild(decoder, arr);
    ASSERT_NE(child, 0U);
    EXPECT_NE(dynxxJsonDecoderReadNext(decoder, child), 0U);
    dynxxJsonDecoderRelease(decoder);
}

TEST(Json, DynxxJsonDecoderRelease) {
    const auto decoder = dynxxJsonDecoderInit(R"({"x":1})");
    ASSERT_NE(decoder, 0U);
    EXPECT_NO_THROW(dynxxJsonDecoderRelease(decoder));
}

TEST(Json, DictAnyJsonRoundTrip) {
    const DictAny in{
        {"name", std::string("dynxx")},
        {"enabled", true},
        {"count", int64_t(9)}
    };
    const auto json = dynxxJsonFromDictAny(in);
    ASSERT_TRUE(json.has_value());
    const auto out = dynxxJsonToDictAny(json.value());
    ASSERT_TRUE(out.has_value());
    EXPECT_EQ(out->count("name"), 1U);
    EXPECT_EQ(out->count("enabled"), 1U);
    EXPECT_EQ(out->count("count"), 1U);
}
