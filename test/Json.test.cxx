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

TEST(Json, DynxxJsonDecoderReadChildrenCount_ObjectField) {
    const auto decoder = dynxxJsonDecoderInit(R"({"obj":{"a":1,"b":2}})");
    ASSERT_NE(decoder, 0U);
    const auto obj = dynxxJsonDecoderReadNode(decoder, "obj");
    ASSERT_NE(obj, 0U);
    EXPECT_EQ(dynxxJsonDecoderReadChildrenCount(decoder, obj), 2U);
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

TEST(Json, DynxxJsonDecoderReadNumber_FromStringField) {
    const auto decoder = dynxxJsonDecoderInit(R"({"i":"42","f":"3.1415"})");
    ASSERT_NE(decoder, 0U);
    const auto iNode = dynxxJsonDecoderReadNode(decoder, "i");
    const auto fNode = dynxxJsonDecoderReadNode(decoder, "f");
    ASSERT_NE(iNode, 0U);
    ASSERT_NE(fNode, 0U);

    EXPECT_EQ(dynxxJsonNodeReadType(iNode), DynXXJsonNodeTypeX::String);
    EXPECT_EQ(dynxxJsonNodeReadType(fNode), DynXXJsonNodeTypeX::String);
    EXPECT_EQ(dynxxJsonDecoderReadInteger(decoder, iNode).value_or(0), 42);
    EXPECT_NEAR(dynxxJsonDecoderReadFloat(decoder, fNode).value_or(0.0), 3.1415, 1e-9);

    dynxxJsonDecoderRelease(decoder);
}

TEST(Json, DynxxJsonDecoderReadString_FromNonStringField) {
    const auto decoder = dynxxJsonDecoderInit(R"({"i":7,"f":2.5,"b":true})");
    ASSERT_NE(decoder, 0U);
    const auto iNode = dynxxJsonDecoderReadNode(decoder, "i");
    const auto fNode = dynxxJsonDecoderReadNode(decoder, "f");
    const auto bNode = dynxxJsonDecoderReadNode(decoder, "b");
    ASSERT_NE(iNode, 0U);
    ASSERT_NE(fNode, 0U);
    ASSERT_NE(bNode, 0U);

    EXPECT_EQ(dynxxJsonDecoderReadString(decoder, iNode).value_or(""), "7");
    EXPECT_NE(dynxxJsonDecoderReadString(decoder, fNode).value_or("").find("2.5"), std::string::npos);
    EXPECT_EQ(dynxxJsonDecoderReadString(decoder, bNode).value_or(""), "true");

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
        {"count", int64_t(9)},
        {"ratio", 3.25}
    };
    const auto json = dynxxJsonFromDictAny(in);
    ASSERT_TRUE(json.has_value());
    const auto out = dynxxJsonToDictAny(json.value());
    ASSERT_TRUE(out.has_value());
    EXPECT_EQ(out->count("name"), 1U);
    EXPECT_EQ(out->count("enabled"), 1U);
    EXPECT_EQ(out->count("count"), 1U);
    EXPECT_EQ(out->count("ratio"), 1U);
    EXPECT_NEAR(dictAnyReadFloat(*out, "ratio").value_or(0.0), 3.25, 1e-9);
}

TEST(Json, DynxxJsonFromToDictAny_MultiTypeRoundTrip) {
    const DictAny in{
        {"name", std::string("dynxx")},
        {"age", int64_t(18)},
        {"score", 99.75},
        {"obj", std::string(R"({"k":1})")},
        {"arr", std::string(R"([1,2,3])")},
        {"enabled", std::string("true")},
        {"none", std::string("null")}
    };

    const auto json = dynxxJsonFromDictAny(in);
    ASSERT_TRUE(json.has_value());

    const auto out = dynxxJsonToDictAny(*json);
    ASSERT_TRUE(out.has_value());

    EXPECT_EQ(dictAnyReadString(*out, "name").value_or(""), "dynxx");
    EXPECT_EQ(dictAnyReadInteger(*out, "age").value_or(0), 18);
    EXPECT_NEAR(dictAnyReadFloat(*out, "score").value_or(0.0), 99.75, 1e-9);
    EXPECT_EQ(dictAnyReadString(*out, "obj").value_or(""), R"({"k":1})");
    EXPECT_EQ(dictAnyReadString(*out, "arr").value_or(""), "[1,2,3]");
    EXPECT_EQ(dictAnyReadString(*out, "enabled").value_or(""), "true");
    EXPECT_EQ(dictAnyReadString(*out, "none").value_or(""), "null");
}

TEST(Json, DynxxJsonDecoderReadFieldByType) {
    const auto decoder = dynxxJsonDecoderInit(R"({"name":"dynxx","age":23,"score":88.5})");
    ASSERT_NE(decoder, 0U);

    const auto nameNode = dynxxJsonDecoderReadNode(decoder, "name");
    const auto ageNode = dynxxJsonDecoderReadNode(decoder, "age");
    const auto scoreNode = dynxxJsonDecoderReadNode(decoder, "score");
    ASSERT_NE(nameNode, 0U);
    ASSERT_NE(ageNode, 0U);
    ASSERT_NE(scoreNode, 0U);

    EXPECT_EQ(dynxxJsonNodeReadType(nameNode), DynXXJsonNodeTypeX::String);
    EXPECT_EQ(dynxxJsonNodeReadType(ageNode), DynXXJsonNodeTypeX::Int32);
    EXPECT_EQ(dynxxJsonNodeReadType(scoreNode), DynXXJsonNodeTypeX::Float);

    EXPECT_EQ(dynxxJsonDecoderReadString(decoder, nameNode).value_or(""), "dynxx");
    EXPECT_EQ(dynxxJsonDecoderReadInteger(decoder, ageNode).value_or(0), 23);
    EXPECT_NEAR(dynxxJsonDecoderReadFloat(decoder, scoreNode).value_or(0.0), 88.5, 1e-9);

    dynxxJsonDecoderRelease(decoder);
}

TEST(Json, DynxxJsonDecoderNodeType_ObjectArrayBoolNull) {
    const auto decoder = dynxxJsonDecoderInit(R"({"obj":{"k":1},"arr":[1,2],"enabled":true,"none":null})");
    ASSERT_NE(decoder, 0U);

    const auto obj = dynxxJsonDecoderReadNode(decoder, "obj");
    const auto arr = dynxxJsonDecoderReadNode(decoder, "arr");
    const auto enabled = dynxxJsonDecoderReadNode(decoder, "enabled");
    const auto none = dynxxJsonDecoderReadNode(decoder, "none");
    ASSERT_NE(obj, 0U);
    ASSERT_NE(arr, 0U);
    ASSERT_NE(enabled, 0U);
    ASSERT_NE(none, 0U);

    EXPECT_EQ(dynxxJsonNodeReadType(obj), DynXXJsonNodeTypeX::Object);
    EXPECT_EQ(dynxxJsonNodeReadType(arr), DynXXJsonNodeTypeX::Array);
    EXPECT_EQ(dynxxJsonNodeReadType(enabled), DynXXJsonNodeTypeX::Boolean);
    EXPECT_EQ(dynxxJsonNodeReadType(none), DynXXJsonNodeTypeX::Null);

    dynxxJsonDecoderRelease(decoder);
}

TEST(Json, DynxxJsonToDictAny_DegradedString_ReDecodeObjectAndArray) {
    const auto dict = dynxxJsonToDictAny(R"({"obj":{"k":1,"name":"dynxx"},"arr":[1,2,3]})");
    ASSERT_TRUE(dict.has_value());

    const auto objStr = dictAnyReadString(*dict, "obj");
    const auto arrStr = dictAnyReadString(*dict, "arr");
    ASSERT_TRUE(objStr.has_value());
    ASSERT_TRUE(arrStr.has_value());

    // After json->dict conversion, object/array fields degrade to JSON strings.
    // Re-decode those strings to continue structured reads.
    const auto objDecoder = dynxxJsonDecoderInit(*objStr);
    ASSERT_NE(objDecoder, 0U);
    const auto objKNode = dynxxJsonDecoderReadNode(objDecoder, "k");
    const auto objNameNode = dynxxJsonDecoderReadNode(objDecoder, "name");
    ASSERT_NE(objKNode, 0U);
    ASSERT_NE(objNameNode, 0U);
    EXPECT_EQ(dynxxJsonNodeReadType(objKNode), DynXXJsonNodeTypeX::Int32);
    EXPECT_EQ(dynxxJsonNodeReadType(objNameNode), DynXXJsonNodeTypeX::String);
    EXPECT_EQ(dynxxJsonDecoderReadInteger(objDecoder, objKNode).value_or(0), 1);
    EXPECT_EQ(dynxxJsonDecoderReadString(objDecoder, objNameNode).value_or(""), "dynxx");
    dynxxJsonDecoderRelease(objDecoder);

    const auto wrappedArrJson = std::string(R"({"arr":)") + *arrStr + "}";
    const auto arrDecoder = dynxxJsonDecoderInit(wrappedArrJson);
    ASSERT_NE(arrDecoder, 0U);
    const auto arrNode = dynxxJsonDecoderReadNode(arrDecoder, "arr");
    ASSERT_NE(arrNode, 0U);
    EXPECT_EQ(dynxxJsonNodeReadType(arrNode), DynXXJsonNodeTypeX::Array);
    EXPECT_EQ(dynxxJsonDecoderReadChildrenCount(arrDecoder, arrNode), 3U);
    dynxxJsonDecoderRelease(arrDecoder);
}
