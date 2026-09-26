#include <gtest/gtest.h>

#include <cstdlib>
#include <cstring>
#include <string>

#include <DynXX/C/Coding.h>
#include <DynXX/C/Crypto.h>
#include <DynXX/C/DynXX.h>
#include <DynXX/C/Json.h>
#include <DynXX/C/KV.h>
#include <DynXX/C/Net.h>
#include <DynXX/C/Zip.h>

/**
 * The C API is an entry surface independent from the C++ API: it hands out
 * `std::malloc`-allocated results through the `dupStr`/`dupBytes`/`dupCharsArray`
 * helpers and converts the raw `const char **` vectors taken by the net calls.
 * Every other suite drives the C++ API only, so the C-side conversion layer was
 * never executed; this suite covers the pure C entry points.
 */
class DynXXCAPITestSuite : public ::testing::Test {};

namespace {
    // Results of the C API are owned by the caller and released with `std::free`.
    void freeCOut(const void *ptr) {
        std::free(const_cast<void *>(ptr));
    }

    // Reads a C API result without taking ownership of it.
    std::string cStr(const char *ptr) {
        return ptr == nullptr ? std::string{} : std::string{ptr};
    }

    constexpr char SAMPLE_TEXT[] = "DynXX-C-API";
    constexpr size_t SAMPLE_TEXT_LEN = sizeof(SAMPLE_TEXT) - 1;
    constexpr char SAMPLE_HEX[] = "44796e58582d432d415049";
    constexpr char AES_KEY_TEXT[] = "1234567890abcdef";
    constexpr size_t AES_KEY_LEN = sizeof(AES_KEY_TEXT) - 1;
    constexpr size_t ZIP_BUFFER_SIZE = 1024;
}

TEST_F(DynXXCAPITestSuite, VersionAndRootPath) {
    const auto *version = dynxx_get_version();
    ASSERT_NE(version, nullptr);
    EXPECT_FALSE(cStr(version).empty());
    freeCOut(version);

    // The engine is initialized by the global test environment.
    const auto *root = dynxx_root_path();
    ASSERT_NE(root, nullptr);
    EXPECT_FALSE(cStr(root).empty());
    freeCOut(root);
}

TEST_F(DynXXCAPITestSuite, CodingCase) {
    const auto *upper = dynxx_coding_case_upper("aBc_123");
    ASSERT_NE(upper, nullptr);
    EXPECT_STREQ(upper, "ABC_123");
    freeCOut(upper);

    const auto *lower = dynxx_coding_case_lower("AbC_123");
    ASSERT_NE(lower, nullptr);
    EXPECT_STREQ(lower, "abc_123");
    freeCOut(lower);

    // A `nullptr` input is answered with a static empty literal, not an allocation.
    EXPECT_STREQ(dynxx_coding_case_upper(nullptr), "");
    EXPECT_STREQ(dynxx_coding_case_lower(nullptr), "");
}

TEST_F(DynXXCAPITestSuite, CodingBytes) {
    const auto *inBytes = reinterpret_cast<const byte *>(SAMPLE_TEXT);

    const auto *hex = dynxx_coding_hex_bytes2str(inBytes, SAMPLE_TEXT_LEN);
    ASSERT_NE(hex, nullptr);
    EXPECT_STREQ(hex, SAMPLE_HEX);
    freeCOut(hex);

    size_t outLen = 0;
    const auto *fromHex = dynxx_coding_hex_str2bytes(SAMPLE_HEX, &outLen);
    ASSERT_NE(fromHex, nullptr);
    ASSERT_EQ(outLen, SAMPLE_TEXT_LEN);
    EXPECT_EQ(std::memcmp(fromHex, SAMPLE_TEXT, SAMPLE_TEXT_LEN), 0);
    freeCOut(fromHex);

    // A `nullptr` length out-parameter is discarded instead of being dereferenced.
    outLen = 0U;
    const auto *fromHexNoLen = dynxx_coding_hex_str2bytes("4479", nullptr);
    ASSERT_NE(fromHexNoLen, nullptr);
    EXPECT_EQ(std::memcmp(fromHexNoLen, "Dy", 2), 0);
    freeCOut(fromHexNoLen);
    EXPECT_EQ(outLen, 0U);

    outLen = 0U;
    const auto *fromStr = dynxx_coding_str2bytes(SAMPLE_TEXT, &outLen);
    ASSERT_NE(fromStr, nullptr);
    ASSERT_EQ(outLen, SAMPLE_TEXT_LEN);
    EXPECT_EQ(std::memcmp(fromStr, SAMPLE_TEXT, SAMPLE_TEXT_LEN), 0);
    freeCOut(fromStr);

    const auto *bytes2Str = dynxx_coding_bytes2str(inBytes, SAMPLE_TEXT_LEN);
    ASSERT_NE(bytes2Str, nullptr);
    EXPECT_STREQ(bytes2Str, SAMPLE_TEXT);
    freeCOut(bytes2Str);

    // Invalid inputs are rejected with `nullptr` instead of an allocation.
    EXPECT_EQ(dynxx_coding_hex_str2bytes(nullptr, nullptr), nullptr);
    EXPECT_EQ(dynxx_coding_str2bytes(nullptr, nullptr), nullptr);
}

TEST_F(DynXXCAPITestSuite, CodingStringHelpers) {
    const auto *trimmed = dynxx_coding_str_trim(" \t dynxx \r\n ");
    ASSERT_NE(trimmed, nullptr);
    EXPECT_STREQ(trimmed, "dynxx");
    freeCOut(trimmed);

    const auto *escaped = dynxx_coding_str_escape_quotes("a\"b");
    ASSERT_NE(escaped, nullptr);
    EXPECT_STREQ(escaped, R"(a\"b)");
    freeCOut(escaped);

    EXPECT_STREQ(dynxx_coding_str_trim(nullptr), "");
    EXPECT_STREQ(dynxx_coding_str_escape_quotes(nullptr), "");
}

TEST_F(DynXXCAPITestSuite, CryptoOutParams) {
    const auto *randBytes = dynxx_crypto_rand(24);
    ASSERT_NE(randBytes, nullptr);
    freeCOut(randBytes);

    const auto *inBytes = reinterpret_cast<const byte *>(SAMPLE_TEXT);
    size_t outLen = 0;

    const auto *md5 = dynxx_crypto_hash_md5(inBytes, SAMPLE_TEXT_LEN, &outLen);
    ASSERT_NE(md5, nullptr);
    EXPECT_EQ(outLen, 16U);
    freeCOut(md5);

    const auto *sha1 = dynxx_crypto_hash_sha1(inBytes, SAMPLE_TEXT_LEN, &outLen);
    ASSERT_NE(sha1, nullptr);
    EXPECT_EQ(outLen, 20U);
    freeCOut(sha1);

    const auto *sha256 = dynxx_crypto_hash_sha256(inBytes, SAMPLE_TEXT_LEN, &outLen);
    ASSERT_NE(sha256, nullptr);
    EXPECT_EQ(outLen, 32U);
    freeCOut(sha256);

    size_t encodedLen = 0;
    const auto *encoded = dynxx_crypto_base64_encode(inBytes, SAMPLE_TEXT_LEN, true, &encodedLen);
    ASSERT_NE(encoded, nullptr);
    ASSERT_GT(encodedLen, 0U);

    size_t decodedLen = 0;
    const auto *decoded = dynxx_crypto_base64_decode(encoded, encodedLen, true, &decodedLen);
    ASSERT_NE(decoded, nullptr);
    EXPECT_EQ(decodedLen, SAMPLE_TEXT_LEN);
    EXPECT_EQ(std::memcmp(decoded, SAMPLE_TEXT, SAMPLE_TEXT_LEN), 0);

    freeCOut(encoded);
    freeCOut(decoded);
}

TEST_F(DynXXCAPITestSuite, CryptoSymmetricRoundTrip) {
    const auto *inBytes = reinterpret_cast<const byte *>(SAMPLE_TEXT);
    const auto *keyBytes = reinterpret_cast<const byte *>(AES_KEY_TEXT);

    size_t encryptedLen = 0;
    const auto *encrypted = dynxx_crypto_aes_encrypt(inBytes, SAMPLE_TEXT_LEN, keyBytes, AES_KEY_LEN, &encryptedLen);
    ASSERT_NE(encrypted, nullptr);
    ASSERT_GT(encryptedLen, 0U);

    size_t decryptedLen = 0;
    const auto *decrypted = dynxx_crypto_aes_decrypt(encrypted, encryptedLen, keyBytes, AES_KEY_LEN, &decryptedLen);
    ASSERT_NE(decrypted, nullptr);
    EXPECT_EQ(decryptedLen, SAMPLE_TEXT_LEN);
    EXPECT_EQ(std::memcmp(decrypted, SAMPLE_TEXT, SAMPLE_TEXT_LEN), 0);

    freeCOut(encrypted);
    freeCOut(decrypted);

    // An invalid key length is rejected on both directions.
    size_t rejectedLen = 0;
    EXPECT_EQ(dynxx_crypto_aes_encrypt(inBytes, SAMPLE_TEXT_LEN,
                                       reinterpret_cast<const byte *>("12345678"), 8, &rejectedLen), nullptr);
    EXPECT_EQ(dynxx_crypto_aes_decrypt(inBytes, SAMPLE_TEXT_LEN,
                                       reinterpret_cast<const byte *>("12345678"), 8, &rejectedLen), nullptr);
}

TEST_F(DynXXCAPITestSuite, JsonDecoder) {
    const auto decoder = dynxx_json_decoder_init(R"({"s":"v","i":7,"f":1.5,"a":[1,2],"o":{"k":"n"}})");
    ASSERT_NE(decoder, 0U);

    const auto sNode = dynxx_json_decoder_read_node(decoder, 0, "s");
    ASSERT_NE(sNode, 0U);
    EXPECT_EQ(dynxx_json_node_read_type(sNode), DynXXJsonNodeTypeString);

    const auto *sValue = dynxx_json_decoder_read_string(decoder, sNode);
    ASSERT_NE(sValue, nullptr);
    EXPECT_STREQ(sValue, "v");
    freeCOut(sValue);

    const auto iNode = dynxx_json_decoder_read_node(decoder, 0, "i");
    ASSERT_NE(iNode, 0U);
    EXPECT_EQ(dynxx_json_decoder_read_integer(decoder, iNode), 7);

    const auto fNode = dynxx_json_decoder_read_node(decoder, 0, "f");
    ASSERT_NE(fNode, 0U);
    EXPECT_DOUBLE_EQ(dynxx_json_decoder_read_float(decoder, fNode), 1.5);

    const auto aNode = dynxx_json_decoder_read_node(decoder, 0, "a");
    ASSERT_NE(aNode, 0U);
    EXPECT_EQ(dynxx_json_decoder_read_children_count(decoder, aNode), 2U);

    const auto firstChild = dynxx_json_decoder_read_child(decoder, aNode);
    ASSERT_NE(firstChild, 0U);
    EXPECT_EQ(dynxx_json_decoder_read_integer(decoder, firstChild), 1);
    const auto secondChild = dynxx_json_decoder_read_next(decoder, firstChild);
    ASSERT_NE(secondChild, 0U);
    EXPECT_EQ(dynxx_json_decoder_read_integer(decoder, secondChild), 2);

    const auto oNode = dynxx_json_decoder_read_node(decoder, 0, "o");
    ASSERT_NE(oNode, 0U);
    const auto kNode = dynxx_json_decoder_read_child(decoder, oNode);
    ASSERT_NE(kNode, 0U);

    const auto *nodeName = dynxx_json_node_read_name(kNode);
    ASSERT_NE(nodeName, nullptr);
    EXPECT_STREQ(nodeName, "k");
    freeCOut(nodeName);

    const auto *nodeStr = dynxx_json_node_to_str(kNode);
    ASSERT_NE(nodeStr, nullptr);
    EXPECT_FALSE(cStr(nodeStr).empty());
    freeCOut(nodeStr);

    // A `nullptr` key or decoder is rejected before touching the decoder.
    EXPECT_EQ(dynxx_json_decoder_read_node(decoder, 0, nullptr), 0U);
    EXPECT_EQ(dynxx_json_decoder_read_node(0, 0, "s"), 0U);

    dynxx_json_decoder_release(decoder);
}

TEST_F(DynXXCAPITestSuite, JsonDecoderRejectsUnknownHandles) {
    // A zero handle is rejected by every reader itself, before the cache is consulted.
    EXPECT_EQ(dynxx_json_decoder_read_string(0, 0), nullptr);
    EXPECT_EQ(dynxx_json_decoder_read_integer(0, 0), 0);
    EXPECT_EQ(dynxx_json_decoder_read_float(0, 0), 0.0);
    EXPECT_EQ(dynxx_json_decoder_read_child(0, 0), 0U);
    EXPECT_EQ(dynxx_json_decoder_read_children_count(0, 0), 0U);
    EXPECT_EQ(dynxx_json_decoder_read_next(0, 0), 0U);
    dynxx_json_decoder_release(0);

    // A handle that was never handed out by the cache resolves to no decoder at all.
    constexpr DynXXJsonDecoderHandle unknown = 1U;
    EXPECT_EQ(dynxx_json_decoder_read_node(unknown, 0, "s"), 0U);
    EXPECT_EQ(dynxx_json_decoder_read_string(unknown, 0), nullptr);
    EXPECT_EQ(dynxx_json_decoder_read_integer(unknown, 0), 0);
    EXPECT_EQ(dynxx_json_decoder_read_float(unknown, 0), 0.0);
    EXPECT_EQ(dynxx_json_decoder_read_child(unknown, 0), 0U);
    EXPECT_EQ(dynxx_json_decoder_read_children_count(unknown, 0), 0U);
    EXPECT_EQ(dynxx_json_decoder_read_next(unknown, 0), 0U);
    dynxx_json_decoder_release(unknown);
}

TEST_F(DynXXCAPITestSuite, KVAllKeys) {
    const auto conn = dynxx_kv_open("capi_kv_all_keys");
    ASSERT_NE(conn, 0U);
    ASSERT_TRUE(dynxx_kv_write_string(conn, "k1", "v1"));
    ASSERT_TRUE(dynxx_kv_write_integer(conn, "k2", 2));

    size_t keyCount = 0;
    const auto **keys = dynxx_kv_all_keys(conn, &keyCount);
    ASSERT_NE(keys, nullptr);
    ASSERT_GE(keyCount, 2U);

    bool foundFirst = false;
    bool foundSecond = false;
    for (size_t i = 0; i < keyCount; ++i) {
        if (keys[i] == nullptr) {
            continue;
        }
        foundFirst = foundFirst || std::strcmp(keys[i], "k1") == 0;
        foundSecond = foundSecond || std::strcmp(keys[i], "k2") == 0;
        freeCOut(keys[i]);
    }
    EXPECT_TRUE(foundFirst);
    EXPECT_TRUE(foundSecond);
    freeCOut(keys);

    // A `nullptr` length out-parameter is accepted and simply discarded.
    const auto **keysNoLen = dynxx_kv_all_keys(conn, nullptr);
    ASSERT_NE(keysNoLen, nullptr);
    for (size_t i = 0; i < keyCount; ++i) {
        freeCOut(keysNoLen[i]);
    }
    freeCOut(keysNoLen);

    // Invalid handles and keys are rejected instead of being dereferenced.
    EXPECT_EQ(dynxx_kv_read_string(0, "k1"), nullptr);
    EXPECT_EQ(dynxx_kv_read_string(conn, nullptr), nullptr);

    // A zero connection is answered with an owned empty array, not a dereference.
    size_t noKeyCount = 1U;
    const auto **noKeys = dynxx_kv_all_keys(0, &noKeyCount);
    ASSERT_NE(noKeys, nullptr);
    EXPECT_EQ(noKeyCount, 0U);
    EXPECT_EQ(noKeys[0], nullptr);
    freeCOut(noKeys);

    dynxx_kv_clear(0);
    dynxx_kv_close(0);

    dynxx_kv_close(conn);
}

TEST_F(DynXXCAPITestSuite, ZipBytesRoundTrip) {
    const auto *inBytes = reinterpret_cast<const byte *>(SAMPLE_TEXT);

    size_t zippedLen = 0;
    const auto *zipped = dynxx_z_bytes_zip(DynXXZipCompressModeDefault, ZIP_BUFFER_SIZE, DynXXZFormatGZip,
                                           inBytes, SAMPLE_TEXT_LEN, &zippedLen);
    ASSERT_NE(zipped, nullptr);
    ASSERT_GT(zippedLen, 0U);

    size_t unzippedLen = 0;
    const auto *unzipped = dynxx_z_bytes_unzip(ZIP_BUFFER_SIZE, DynXXZFormatGZip,
                                               zipped, zippedLen, &unzippedLen);
    ASSERT_NE(unzipped, nullptr);
    ASSERT_EQ(unzippedLen, SAMPLE_TEXT_LEN);
    EXPECT_EQ(std::memcmp(unzipped, SAMPLE_TEXT, SAMPLE_TEXT_LEN), 0);

    freeCOut(zipped);
    freeCOut(unzipped);
}

TEST_F(DynXXCAPITestSuite, ZipBytesRejectsEmptyAndInvalidInput) {
    // Empty input is answered with an empty result, not with an allocation.
    size_t emptyLen = 1U;
    EXPECT_EQ(dynxx_z_bytes_zip(DynXXZipCompressModeDefault, ZIP_BUFFER_SIZE, DynXXZFormatGZip,
                                nullptr, 0, &emptyLen), nullptr);
    EXPECT_EQ(emptyLen, 0U);

    // Data that was never compressed is reported as a failure: zlib answers with Z_DATA_ERROR
    // and the pipeline turns that into an empty result instead of handing back garbage.
    const byte notCompressed[]{0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};
    size_t failedLen = 1U;
    EXPECT_EQ(dynxx_z_bytes_unzip(ZIP_BUFFER_SIZE, DynXXZFormatGZip,
                                  notCompressed, sizeof(notCompressed), &failedLen), nullptr);
    EXPECT_EQ(failedLen, 0U);
}

TEST_F(DynXXCAPITestSuite, NetHttpRequestConvertsCharVectorArgs) {
    // The `const char **` argument vectors are converted before the transfer starts,
    // so an unreachable endpoint covers the conversion without needing the network.
    const char *headers[]{"X-DynXX-Test: capi"};
    const char *formNames[]{"desc"};
    const char *formMimes[]{"text/plain"};
    const char *formData[]{"capi-body"};

    const auto *rsp = dynxx_net_http_request("http://127.0.0.1:1/", "", DynXXHttpMethodPost,
                                              headers, 1,
                                              formNames, formMimes, formData, 1,
                                              nullptr, 0, 1000);
    // The failed transfer is still reported as a serialized response.
    ASSERT_NE(rsp, nullptr);
    EXPECT_NE(std::strstr(rsp, "\"code\""), nullptr);
    freeCOut(rsp);

    // A `nullptr` URL short-circuits with a static empty literal that must not be freed.
    EXPECT_STREQ(dynxx_net_http_request(nullptr, "", DynXXHttpMethodGet, nullptr, 0,
                                        nullptr, nullptr, nullptr, 0, nullptr, 0, 1000), "");

    // Null/empty argument vectors are accepted and read as "no arguments".
    const auto *rspNoArgs = dynxx_net_http_request("http://127.0.0.1:1/", "", DynXXHttpMethodGet,
                                                   nullptr, 0, nullptr, nullptr, nullptr, 0,
                                                   nullptr, 0, 1000);
    ASSERT_NE(rspNoArgs, nullptr);
    EXPECT_NE(std::strstr(rspNoArgs, "\"code\""), nullptr);
    freeCOut(rspNoArgs);

    // A non-null vector with a zero count is also read as "no arguments".
    const auto *rspZeroCount = dynxx_net_http_request("http://127.0.0.1:1/", "", DynXXHttpMethodGet,
                                                      headers, 0, formNames, formMimes, formData, 0,
                                                      nullptr, 0, 1000);
    ASSERT_NE(rspZeroCount, nullptr);
    EXPECT_NE(std::strstr(rspZeroCount, "\"code\""), nullptr);
    freeCOut(rspZeroCount);
}
