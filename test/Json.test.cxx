#include <gtest/gtest.h>
#include <DynXX/CXX/Json.hxx>

class DynXXJsonTestSuite : public ::testing::Test {};

TEST_F(DynXXJsonTestSuite, NodeReadType) {
    const auto decoder = dynxxJsonDecoderInit(R"({"name":"dynxx"})");
    ASSERT_NE(decoder, 0U);
    const auto node = dynxxJsonDecoderReadNode(decoder, "name");
    ASSERT_NE(node, 0U);
    EXPECT_EQ(dynxxJsonNodeReadType(node), DynXXJsonNodeTypeX::String);
    dynxxJsonDecoderRelease(decoder);
}

TEST_F(DynXXJsonTestSuite, NodeReadName) {
    const auto decoder = dynxxJsonDecoderInit(R"({"name":"dynxx"})");
    ASSERT_NE(decoder, 0U);
    const auto node = dynxxJsonDecoderReadNode(decoder, "name");
    ASSERT_NE(node, 0U);
    EXPECT_EQ(dynxxJsonNodeReadName(node).value_or(""), "name");
    dynxxJsonDecoderRelease(decoder);
}

TEST_F(DynXXJsonTestSuite, NodeToStr) {
    const auto decoder = dynxxJsonDecoderInit(R"({"name":"dynxx"})");
    ASSERT_NE(decoder, 0U);
    const auto node = dynxxJsonDecoderReadNode(decoder, "name");
    ASSERT_NE(node, 0U);
    EXPECT_FALSE(dynxxJsonNodeToStr(node).value_or("").empty());
    dynxxJsonDecoderRelease(decoder);
}

TEST_F(DynXXJsonTestSuite, FromDictAny) {
    const DictAny dict{
        {"name", std::string("dynxx")},
        {"n", 1Z}
    };
    const auto json = dynxxJsonFromDictAny(dict);
    ASSERT_TRUE(json.has_value());
    EXPECT_NE(json->find("name"), std::string::npos);
}

TEST_F(DynXXJsonTestSuite, ToDictAny) {
    const auto dict = dynxxJsonToDictAny(R"({"name":"dynxx","n":1})");
    ASSERT_TRUE(dict.has_value());
    EXPECT_EQ(dict->count("name"), 1U);
}

TEST_F(DynXXJsonTestSuite, DecoderInit) {
    const auto decoder = dynxxJsonDecoderInit(R"({"x":1})");
    EXPECT_NE(decoder, 0U);
    dynxxJsonDecoderRelease(decoder);
}

TEST_F(DynXXJsonTestSuite, DecoderReadNode) {
    const auto decoder = dynxxJsonDecoderInit(R"({"x":1})");
    ASSERT_NE(decoder, 0U);
    EXPECT_NE(dynxxJsonDecoderReadNode(decoder, "x"), 0U);
    EXPECT_EQ(dynxxJsonDecoderReadNode(decoder, ""), 0U);
    dynxxJsonDecoderRelease(decoder);
}

TEST_F(DynXXJsonTestSuite, DecoderReadString) {
    const auto decoder = dynxxJsonDecoderInit(R"({"name":"dynxx"})");
    ASSERT_NE(decoder, 0U);
    const auto node = dynxxJsonDecoderReadNode(decoder, "name");
    ASSERT_NE(node, 0U);
    EXPECT_EQ(dynxxJsonDecoderReadString(decoder, node).value_or(""), "dynxx");
    dynxxJsonDecoderRelease(decoder);
}

TEST_F(DynXXJsonTestSuite, DecoderReadStringWithNestedJsonString) {
    const auto decoder = dynxxJsonDecoderInit(R"({"payload":"{\"inner\":{\"k\":\"v\"},\"arr\":[1,2]}","text":"line1\nline2\t\"q\""})");
    ASSERT_NE(decoder, 0U);
    const auto payloadNode = dynxxJsonDecoderReadNode(decoder, "payload");
    const auto textNode = dynxxJsonDecoderReadNode(decoder, "text");
    ASSERT_NE(payloadNode, 0U);
    ASSERT_NE(textNode, 0U);
    EXPECT_EQ(
        dynxxJsonDecoderReadString(decoder, payloadNode).value_or(""),
        R"({"inner":{"k":"v"},"arr":[1,2]})"
    );
    EXPECT_EQ(
        dynxxJsonDecoderReadString(decoder, textNode).value_or(""),
        "line1\nline2\t\"q\""
    );
    dynxxJsonDecoderRelease(decoder);
}

TEST_F(DynXXJsonTestSuite, DecoderReadInteger) {
    const auto decoder = dynxxJsonDecoderInit(R"({"n":7})");
    ASSERT_NE(decoder, 0U);
    const auto node = dynxxJsonDecoderReadNode(decoder, "n");
    ASSERT_NE(node, 0U);
    EXPECT_EQ(dynxxJsonDecoderReadInteger(decoder, node).value_or(0), 7);
    dynxxJsonDecoderRelease(decoder);
}

TEST_F(DynXXJsonTestSuite, DecoderReadFloat) {
    const auto decoder = dynxxJsonDecoderInit(R"({"f":3.5})");
    ASSERT_NE(decoder, 0U);
    const auto node = dynxxJsonDecoderReadNode(decoder, "f");
    ASSERT_NE(node, 0U);
    EXPECT_NEAR(dynxxJsonDecoderReadFloat(decoder, node).value_or(0.0), 3.5, 1e-9);
    dynxxJsonDecoderRelease(decoder);
}

TEST_F(DynXXJsonTestSuite, DecoderReadChild) {
    const auto decoder = dynxxJsonDecoderInit(R"({"arr":[1,2]})");
    ASSERT_NE(decoder, 0U);
    const auto arr = dynxxJsonDecoderReadNode(decoder, "arr");
    ASSERT_NE(arr, 0U);
    EXPECT_NE(dynxxJsonDecoderReadChild(decoder, arr), 0U);
    dynxxJsonDecoderRelease(decoder);
}

TEST_F(DynXXJsonTestSuite, DecoderReadChildrenCount) {
    const auto decoder = dynxxJsonDecoderInit(R"({"arr":[1,2]})");
    ASSERT_NE(decoder, 0U);
    const auto arr = dynxxJsonDecoderReadNode(decoder, "arr");
    ASSERT_NE(arr, 0U);
    EXPECT_EQ(dynxxJsonDecoderReadChildrenCount(decoder, arr), 2U);
    dynxxJsonDecoderRelease(decoder);
}

TEST_F(DynXXJsonTestSuite, DecoderReadChildrenCountObjectField) {
    const auto decoder = dynxxJsonDecoderInit(R"({"obj":{"a":1,"b":2}})");
    ASSERT_NE(decoder, 0U);
    const auto obj = dynxxJsonDecoderReadNode(decoder, "obj");
    ASSERT_NE(obj, 0U);
    EXPECT_EQ(dynxxJsonDecoderReadChildrenCount(decoder, obj), 2U);
    dynxxJsonDecoderRelease(decoder);
}

TEST_F(DynXXJsonTestSuite, DecoderReadChildren) {
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

TEST_F(DynXXJsonTestSuite, DecoderReadNext) {
    const auto decoder = dynxxJsonDecoderInit(R"({"arr":[1,2]})");
    ASSERT_NE(decoder, 0U);
    const auto arr = dynxxJsonDecoderReadNode(decoder, "arr");
    ASSERT_NE(arr, 0U);
    const auto child = dynxxJsonDecoderReadChild(decoder, arr);
    ASSERT_NE(child, 0U);
    EXPECT_NE(dynxxJsonDecoderReadNext(decoder, child), 0U);
    dynxxJsonDecoderRelease(decoder);
}

TEST_F(DynXXJsonTestSuite, DecoderReadNumberFromStringField) {
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

TEST_F(DynXXJsonTestSuite, DecoderReadStringFromNonStringField) {
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

TEST_F(DynXXJsonTestSuite, DecoderRelease) {
    const auto decoder = dynxxJsonDecoderInit(R"({"x":1})");
    ASSERT_NE(decoder, 0U);
    EXPECT_NO_THROW(dynxxJsonDecoderRelease(decoder));
}

TEST_F(DynXXJsonTestSuite, DecoderReadWithInvalidDecoder) {
    EXPECT_EQ(dynxxJsonDecoderReadNode(0, "x"), 0U);
    EXPECT_FALSE(dynxxJsonDecoderReadString(0).has_value());
    EXPECT_FALSE(dynxxJsonDecoderReadInteger(0).has_value());
    EXPECT_FALSE(dynxxJsonDecoderReadFloat(0).has_value());
    EXPECT_EQ(dynxxJsonDecoderReadChild(0), 0U);
    EXPECT_EQ(dynxxJsonDecoderReadChildrenCount(0), 0U);
    EXPECT_EQ(dynxxJsonDecoderReadNext(0), 0U);
}

TEST_F(DynXXJsonTestSuite, DictAnyJsonRoundTrip) {
    const DictAny in{
        {"name", std::string("dynxx")},
        {"enabled", true},
        {"count", 9Z},
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

TEST_F(DynXXJsonTestSuite, FromToDictAnyMultiTypeRoundTrip) {
    const DictAny in{
        {"name", std::string("dynxx")},
        {"age", 18Z},
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

TEST_F(DynXXJsonTestSuite, DecoderReadFieldByType) {
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

TEST_F(DynXXJsonTestSuite, DecoderNodeTypeObjectArrayBoolNull) {
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

TEST_F(DynXXJsonTestSuite, ToDictAnyDegradedStringReDecodeObjectAndArray) {
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

TEST_F(DynXXJsonTestSuite, ToDictAnyNestedJsonStringFieldReDecode) {
    const auto dict = dynxxJsonToDictAny(R"({"payload":"{\"inner\":{\"k\":1},\"arr\":[1,2]}","name":"dynxx"})");
    ASSERT_TRUE(dict.has_value());
    const auto payload = dictAnyReadString(*dict, "payload");
    ASSERT_TRUE(payload.has_value());
    EXPECT_EQ(dictAnyReadString(*dict, "name").value_or(""), "dynxx");

    const auto payloadDecoder = dynxxJsonDecoderInit(*payload);
    ASSERT_NE(payloadDecoder, 0U);
    const auto innerNode = dynxxJsonDecoderReadNode(payloadDecoder, "inner");
    const auto arrNode = dynxxJsonDecoderReadNode(payloadDecoder, "arr");
    ASSERT_NE(innerNode, 0U);
    ASSERT_NE(arrNode, 0U);
    EXPECT_EQ(dynxxJsonNodeReadType(innerNode), DynXXJsonNodeTypeX::Object);
    EXPECT_EQ(dynxxJsonNodeReadType(arrNode), DynXXJsonNodeTypeX::Array);
    dynxxJsonDecoderRelease(payloadDecoder);
}

TEST_F(DynXXJsonTestSuite, ToDictAnyInvalidInputShouldReturnNullopt) {
    for (const auto &input : {
        std::string(""),
        std::string("{"),
        std::string("[]"),
        std::string("not-json"),
        std::string(R"({"bad":"\x"})"),
        std::string("{\"bad\":\"line1\nline2\"}")
    }) {
        EXPECT_FALSE(dynxxJsonToDictAny(input).has_value());
    }
}



