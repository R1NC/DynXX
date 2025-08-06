#if defined(USE_QJS) || defined(USE_LUA)

#include "ScriptBridge.hxx"

#include <algorithm>
#include <functional>
#if defined(__cpp_lib_ranges)
#include <ranges>
#endif

#include <NGenXX.hxx>
#include <NGenXXNet.h>
#include "../core/json/JsonCodec.hxx"

namespace
{
    using namespace NGenXX::Core::Json;

    std::string bytes2json(const Bytes &bytes)
    {
        if (bytes.empty()) [[unlikely]]
        {
            return {};
        }
#if defined(__cpp_lib_ranges)
        auto intV = bytes 
            | std::views::transform([](const auto b) { return static_cast<int>(b); })
            | std::ranges::to<std::vector>();
#else
        std::vector<int> intV(bytes.size());
        std::transform(bytes.begin(), bytes.end(), intV.begin(), [](const auto b) { 
            return static_cast<int>(b); 
        });
#endif
        const auto cj = cJSON_CreateIntArray(intV.data(), static_cast<int>(intV.size()));
        const auto json = ngenxxJsonToStr(cj);
        cJSON_Delete(cj);
        return json.value_or("");
    }

    std::string strArray2json(const std::vector<std::string> &v)
    {
        const auto cj = cJSON_CreateArray();
        for (const auto& it : v)
        {
            cJSON_AddItemToArray(cj, cJSON_CreateString(it.c_str()));
        }
        const auto json = ngenxxJsonToStr(cj);
        cJSON_Delete(cj);
        return json.value_or("");
    }

    template <NumberT T>
    std::optional<T> parseNum(const Decoder &decoder, const std::string_view& k)
    {
        const auto node = decoder[k];
        if (node == nullptr) [[unlikely]]
        {
            return std::nullopt;
        }
        const auto v = decoder.readNumber(node).value();
        return static_cast<T>(v);
    }

    template <NumberT... Ns, KeyType... Ks>
    auto parseNumX(const Decoder &decoder, Ks... ks)
    {
        return std::make_tuple(parseNum<Ns>(decoder, ks)...);
    }

    std::optional<std::string> parseStr(const Decoder &decoder, const std::string_view& k)
    {
        const auto node = decoder[k];
        return node == nullptr ? std::nullopt : decoder.readString(node);
    }

    template <KeyType... Ks>
    auto parseStrX(const Decoder &decoder, Ks... ks)
    {
        return std::make_tuple(parseStr(decoder, ks)...);
    }

    template <typename T = void>
    T* parsePtr(const Decoder &decoder, const std::string_view& k)
    {
        address addr = 0;
        const auto s = parseStr(decoder, k);
        if (s == std::nullopt)
        {
            return nullptr;
        }
    
        addr = str2int64(s.value());

        return addr2ptr<T>(addr);
    }

    Bytes parseByteArray(const Decoder &decoder, const std::string_view& bytesK)
    {
        Bytes data;
        if (const auto byte_vNode = decoder[bytesK])
        {
            const auto len = decoder.readChildrenCount(byte_vNode);
            data.reserve(len);
            decoder.readChildren(byte_vNode, 
                            [&data, &decoder](size_t, void *const child)
                            {
                                data.emplace_back(decoder.readNumber(child).value_or(0));
                            });
        }
        return data;
    }

    std::vector<std::string> parseStrArray(const Decoder &decoder, const std::string_view& strVK)
    {
        std::vector<std::string> v;
        if (const auto str_vNode = decoder[strVK])
        {
            const auto len = decoder.readChildrenCount(str_vNode);
            v.reserve(len);
            decoder.readChildren(str_vNode,
                             [&v, &decoder](size_t, void *const child)
                             {
                                 v.emplace_back(decoder.readString(child).value_or(""));
                             });
        }
        return v;
    }

    class JsonParser
    {
    public:
        JsonParser() = delete;
        explicit JsonParser(const std::string &json) : decoder(json) {}

        std::optional<std::string> str(const std::string_view& k) const
        {
            return parseStr(this->decoder, k);
        }

        template <NumberT T>
        std::optional<T> num(const std::string_view& k) const
        {
            return parseNum<T>(this->decoder, k);
        }

        Bytes byteArray(const std::string_view& k) const
        {
            return parseByteArray(this->decoder, k);
        }

        std::vector<std::string> strArray(const std::string_view& k) const
        {
            return parseStrArray(this->decoder, k);
        }

        template <typename T = void>
        T* ptr(const std::string_view& k) const
        {
            return parsePtr<T>(this->decoder, k);
        }

        template <KeyType... Ks>
        auto strX(Ks... ks) const
        {
            return std::make_tuple(parseStr(this->decoder, ks)...);
        }

        template <NumberT... Ns, KeyType... Ks>
        auto numX(Ks... ks) const
        {
            return std::make_tuple(parseNum<Ns>(this->decoder, ks)...);
        }

    private:
        Decoder decoder;
    };
}

std::string ngenxx_get_versionS(const char *json)
{
    return ngenxxGetVersion();
}

std::string ngenxx_root_pathS(const char *json)
{
    return ngenxxRootPath();
}

#pragma mark Device.DeviceInfo

int ngenxx_device_typeS(const char *json)
{
    return static_cast<int>(ngenxxDeviceType());
}

std::string ngenxx_device_nameS(const char *json)
{
    return ngenxxDeviceName();
}

std::string ngenxx_device_manufacturerS(const char *json)
{
    return ngenxxDeviceManufacturer();
}

std::string ngenxx_device_os_versionS(const char *json)
{
    return ngenxxDeviceOsVersion();
}

int ngenxx_device_cpu_archS(const char *json)
{
    return static_cast<int>(ngenxxDeviceCpuArch());
}

#pragma mark Log

void ngenxx_log_printS(const char *json)
{
    if (json == nullptr)
    {
        return;
    }
    const JsonParser parser(json);
    const auto level = parser.num<NGenXXLogLevelX>("level");
    const auto content = parser.str("content");
    if (level == std::nullopt || content == std::nullopt)
    {
        return;
    }

    ngenxxLogPrint(level.value(), content.value());
}

#pragma mark Net.Http

std::string ngenxx_net_http_requestS(const char *json)
{
    if (json == nullptr)
    {
        return {};
    }
    const JsonParser parser(json);
    const auto [url, params] = parser.strX("url", "params");
    const auto [method, fileSize, timeout] = parser.numX<NGenXXHttpMethodX, size_t, size_t>("method", "fileSize", "timeout");

    const auto rawBody = parser.byteArray("rawBodyBytes");

    const auto header_v = parser.strArray("header_v");

    const auto form_field_name_v = parser.strArray("form_field_name_v");
    const auto form_field_mime_v = parser.strArray("form_field_mime_v");
    const auto form_field_data_v = parser.strArray("form_field_data_v");

    const auto cFILE = parser.ptr<std::FILE>("cFILE");

    const auto header_c = header_v.size();
    const auto form_field_count = form_field_name_v.size();
    if (url == std::nullopt || method == std::nullopt || header_c > NGENXX_HTTP_HEADER_MAX_COUNT)
    {
        return {};
    }
    if (form_field_count == 0 && (!form_field_name_v.empty() || !form_field_mime_v.empty() || !form_field_data_v.empty())) [[unlikely]]
    {
        return {};
    }
    if (form_field_count > 0 && (form_field_name_v.empty() || form_field_mime_v.empty() || form_field_data_v.empty())) [[unlikely]]
    {
        return {};
    }
    if ((cFILE != nullptr && fileSize.value_or(0) == 0) || (cFILE == nullptr && fileSize.value_or(0) > 0)) [[unlikely]]
    {
        return {};
    }

    const auto t = ngenxxNetHttpRequest(url.value(), method.value(), 
                        params.value_or(""), rawBody, header_v,
                        form_field_name_v, form_field_mime_v, form_field_data_v,
                        cFILE, fileSize.value_or(0), 
                        timeout.value_or(0));
    return t.toJson().value_or("");
}

bool ngenxx_net_http_downloadS(const char *json)
{
    if (json == nullptr)
    {
        return false;
    }
    const JsonParser parser(json);
    const auto [url, file] = parser.strX("url", "file");
    const auto timeout = parser.num<size_t>("timeout");

    if (url == std::nullopt || file == std::nullopt)
    {
        return false;
    }

    return ngenxxNetHttpDownload(url.value(), file.value(), timeout.value_or(0));
}

#pragma mark Store.SQLite

std::string ngenxx_store_sqlite_openS(const char *json)
{
    if (json == nullptr)
    {
        return {};
    }
    const JsonParser parser(json);
    const auto _id = parser.str("_id");
    if (_id == std::nullopt)
    {
        return {};
    }

    const auto db = ngenxxStoreSqliteOpen(_id.value());
    if (db == nullptr)
    {
        return {};
    }
    return std::to_string(ptr2addr(db));
}

bool ngenxx_store_sqlite_executeS(const char *json)
{
    if (json == nullptr)
    {
        return false;
    }
    const JsonParser parser(json);
    const auto conn = parser.ptr("conn");
    const auto sql = parser.str("sql");
    if (conn == nullptr || sql == std::nullopt)
    {
        return false;
    }

    return ngenxxStoreSqliteExecute(conn, sql.value());
}

std::string ngenxx_store_sqlite_query_doS(const char *json)
{
    if (json == nullptr)
    {
        return {};
    }
    const JsonParser parser(json);
    const auto conn = parser.ptr("conn");
    const auto sql = parser.str("sql");
    if (conn == nullptr || sql == std::nullopt)
    {
        return {};
    }

    const auto res = ngenxxStoreSqliteQueryDo(conn, sql.value());
    if (res == nullptr)
    {
        return {};
    }
    return std::to_string(ptr2addr(res));
}

bool ngenxx_store_sqlite_query_read_rowS(const char *json)
{
    if (json == nullptr)
    {
        return false;
    }
    const JsonParser parser(json);
    const auto query_result = parser.ptr("query_result");
    if (query_result == nullptr)
    {
        return false;
    }

    return ngenxxStoreSqliteQueryReadRow(query_result);
}

std::string ngenxx_store_sqlite_query_read_column_textS(const char *json)
{
    if (json == nullptr)
    {
        return {};
    }
    const JsonParser parser(json);
    const auto query_result = parser.ptr("query_result");
    const auto column = parser.str("column");
    if (query_result == nullptr || column == std::nullopt)
    {
        return {};
    }

    const auto s = ngenxxStoreSqliteQueryReadColumnText(query_result, column.value());
    return s.value_or(std::string{});
}

int64_t ngenxx_store_sqlite_query_read_column_integerS(const char *json)
{
    if (json == nullptr)
    {
        return 0;
    }
    const JsonParser parser(json);
    const auto query_result = parser.ptr("query_result");
    const auto column = parser.str("column");
    if (query_result == nullptr || column == std::nullopt)
    {
        return 0;
    }

    const auto i = ngenxxStoreSqliteQueryReadColumnInteger(query_result, column.value());
    return i.value_or(0);
}

double ngenxx_store_sqlite_query_read_column_floatS(const char *json)
{
    if (json == nullptr)
    {
        return 0;
    }
    const JsonParser parser(json);
    const auto query_result = parser.ptr("query_result");
    const auto column = parser.str("column");
    if (query_result == nullptr || column == std::nullopt)
    {
        return 0;
    }

    const auto f = ngenxxStoreSqliteQueryReadColumnFloat(query_result, column.value());
    return f.value_or(0.0);
}

void ngenxx_store_sqlite_query_dropS(const char *json)
{
    if (json == nullptr)
    {
        return;
    }
    const JsonParser parser(json);
    const auto query_result = parser.ptr("query_result");
    if (query_result == nullptr)
    {
        return;
    }

    ngenxxStoreSqliteQueryDrop(query_result);
}

void ngenxx_store_sqlite_closeS(const char *json)
{
    if (json == nullptr)
    {
        return;
    }
    const JsonParser parser(json);
    const auto conn = parser.ptr("conn");
    if (conn == nullptr)
    {
        return;
    }

    ngenxxStoreSqliteClose(conn);
}

#pragma mark Store.KV

std::string ngenxx_store_kv_openS(const char *json)
{
    if (json == nullptr)
    {
        return {};
    }
    const JsonParser parser(json);
    const auto _id = parser.str("_id");
    if (_id == std::nullopt)
    {
        return {};
    }

    const auto res = ngenxxStoreKvOpen(_id.value());
    if (res == nullptr)
    {
        return {};
    }
    return std::to_string(ptr2addr(res));
}

std::string ngenxx_store_kv_read_stringS(const char *json)
{
    if (json == nullptr)
    {
        return {};
    }
    const JsonParser parser(json);
    const auto conn = parser.ptr("conn");
    const auto k = parser.str("k");
    if (conn == nullptr || k == std::nullopt)
    {
        return {};
    }

    const auto s = ngenxxStoreKvReadString(conn, k.value());
    return s.value_or(std::string{});
}

bool ngenxx_store_kv_write_stringS(const char *json)
{
    if (json == nullptr)
    {
        return false;
    }
    const JsonParser parser(json);
    const auto conn = parser.ptr("conn");
    const auto [k, v] = parser.strX("k", "v");
    if (conn == nullptr || k == std::nullopt)
    {
        return false;
    }

    return ngenxxStoreKvWriteString(conn, k.value(), v.value_or(""));
}

int64_t ngenxx_store_kv_read_integerS(const char *json)
{
    if (json == nullptr)
    {
        return 0;
    }
    const JsonParser parser(json);
    const auto conn = parser.ptr("conn");
    const auto k = parser.str("k");
    if (conn == nullptr || k == std::nullopt)
    {
        return 0;
    }

    const auto i = ngenxxStoreKvReadInteger(conn, k.value());
    return i.value_or(0);
}

bool ngenxx_store_kv_write_integerS(const char *json)
{
    if (json == nullptr)
    {
        return false;
    }
    const JsonParser parser(json);
    const auto conn = parser.ptr("conn");
    const auto k = parser.str("k");
    const auto v = parser.num<int64_t>("v");
    if (conn == nullptr || k == std::nullopt)
    {
        return false;
    }

    return ngenxxStoreKvWriteInteger(conn, k.value(), v.value_or(0));
}

double ngenxx_store_kv_read_floatS(const char *json)
{
    if (json == nullptr)
    {
        return 0;
    }
    const JsonParser parser(json);
    const auto conn = parser.ptr("conn");
    const auto k = parser.str("k");
    if (conn == nullptr || k == std::nullopt)
    {
        return false;
    }

    const auto f = ngenxxStoreKvReadFloat(conn, k.value());
    return f.value_or(0.0);
}

bool ngenxx_store_kv_write_floatS(const char *json)
{
    if (json == nullptr)
    {
        return false;
    }
    const JsonParser parser(json);
    const auto conn = parser.ptr("conn");
    const auto k = parser.str("k");
    const auto v = parser.num<double>("v");
    if (conn == nullptr || k == std::nullopt)
    {
        return false;
    }

    return ngenxxStoreKvWriteFloat(conn, k.value(), v.value_or(0.0));
}

std::string ngenxx_store_kv_all_keysS(const char *json)
{
    if (json == nullptr)
    {
        return {};
    }
    const JsonParser parser(json);
    const auto conn = parser.ptr("conn");
    if (conn == nullptr)
    {
        return {};
    }

    const auto res = ngenxxStoreKvAllKeys(conn);
    return strArray2json(res);
}

bool ngenxx_store_kv_containsS(const char *json)
{
    if (json == nullptr)
    {
        return false;
    }
    const JsonParser parser(json);
    const auto conn = parser.ptr("conn");
    const auto k = parser.str("k");
    if (conn == nullptr || k == std::nullopt)
    {
        return false;
    }

    return ngenxxStoreKvContains(conn, k.value());
}

bool ngenxx_store_kv_removeS(const char *json)
{
    if (json == nullptr)
    {
        return false;
    }
    const JsonParser parser(json);
    const auto conn = parser.ptr("conn");
    const auto k = parser.str("k");
    if (conn == nullptr || k == std::nullopt)
    {
        return false;
    }

    return ngenxxStoreKvRemove(conn, k.value());
}

void ngenxx_store_kv_clearS(const char *json)
{
    if (json == nullptr)
    {
        return;
    }
    const JsonParser parser(json);
    const auto conn = parser.ptr("conn");
    if (conn == nullptr)
    {
        return;
    }

    ngenxxStoreKvClear(conn);
}

void ngenxx_store_kv_closeS(const char *json)
{
    if (json == nullptr)
    {
        return;
    }
    const JsonParser parser(json);
    const auto conn = parser.ptr("conn");
    if (conn == nullptr)
    {
        return;
    }

    ngenxxStoreKvClose(conn);
}

#pragma mark Coding

std::string ngenxx_coding_hex_bytes2strS(const char *json)
{
    if (json == nullptr)
    {
        return {};
    }
    const JsonParser parser(json);

    const auto in = parser.byteArray("inBytes");
    if (in.empty())
    {
        return {};
    }

    return ngenxxCodingHexBytes2str(in);
}

std::string ngenxx_coding_hex_str2bytesS(const char *json)
{
    if (json == nullptr)
    {
        return {};
    }
    const JsonParser parser(json);

    const auto str = parser.str("str");
    if (str == std::nullopt)
    {
        return {};
    }

    const auto bytes = ngenxxCodingHexStr2bytes(str.value());
    return bytes2json(bytes);
}

std::string ngenxx_coding_bytes2strS(const char *json)
{
    if (json == nullptr)
    {
        return {};
    }
    const JsonParser parser(json);

    const auto in = parser.byteArray("inBytes");
    if (in.empty())
    {
        return {};
    }

    return ngenxxCodingBytes2str(in);
}

std::string ngenxx_coding_str2bytesS(const char *json)
{
    if (json == nullptr)
    {
        return {};
    }
    const JsonParser parser(json);

    const auto str = parser.str("str");
    if (str == std::nullopt)
    {
        return {};
    }

    const auto bytes = ngenxxCodingStr2bytes(str.value());
    return bytes2json(bytes);
}

std::string ngenxx_coding_case_upperS(const char *json)
{
    if (json == nullptr)
    {
        return {};
    }
    const JsonParser parser(json);

    const auto str = parser.str("str");
    if (str == std::nullopt)
    {
        return {};
    }
    return ngenxxCodingCaseUpper(str.value());
}

std::string ngenxx_coding_case_lowerS(const char *json)
{
    if (json == nullptr)
    {
        return {};
    }
    const JsonParser parser(json);

    const auto str = parser.str("str");
    if (str == std::nullopt)
    {
        return {};
    }
    return ngenxxCodingCaseLower(str.value());
}

#pragma mark Crypto

std::string ngenxx_crypto_randS(const char *json)
{
    if (json == nullptr)
    {
        return {};
    }
    const JsonParser parser(json);
    const auto outLen = parser.num<size_t>("len");
    if (outLen == std::nullopt)
    {
        return {};
    }
    Bytes outBytes(outLen.value());
    ngenxxCryptoRand(outBytes.size(), outBytes.data());
    return bytes2json(outBytes);
}

std::string ngenxx_crypto_aes_encryptS(const char *json)
{
    if (json == nullptr)
    {
        return {};
    }
    const JsonParser parser(json);

    const auto in = parser.byteArray("inBytes");
    if (in.empty())
    {
        return {};
    }

    const auto key = parser.byteArray("keyBytes");
    if (key.empty())
    {
        return {};
    }

    const auto outBytes = ngenxxCryptoAesEncrypt(in, key);
    return bytes2json(outBytes);
}

std::string ngenxx_crypto_aes_decryptS(const char *json)
{
    if (json == nullptr)
    {
        return {};
    }
    const JsonParser parser(json);

    const auto in = parser.byteArray("inBytes");
    if (in.empty())
    {
        return {};
    }

    const auto key = parser.byteArray("keyBytes");
    if (key.empty())
    {
        return {};
    }

    const auto outBytes = ngenxxCryptoAesDecrypt(in, key);
    return bytes2json(outBytes);
}

std::string ngenxx_crypto_aes_gcm_encryptS(const char *json)
{
    if (json == nullptr)
    {
        return {};
    }
    const JsonParser parser(json);

    const auto in = parser.byteArray("inBytes");
    if (in.empty())
    {
        return {};
    }

    const auto key = parser.byteArray("keyBytes");
    if (key.empty())
    {
        return {};
    }

    const auto iv = parser.byteArray("initVectorBytes");
    if (iv.empty())
    {
        return {};
    }

    const auto aad = parser.byteArray("aadBytes");

    const auto tagBits = parser.num<size_t>("tagBits");

    const auto outBytes = ngenxxCryptoAesGcmEncrypt(in, key, iv, tagBits.value_or(0), aad);
    return bytes2json(outBytes);
}

std::string ngenxx_crypto_aes_gcm_decryptS(const char *json)
{
    if (json == nullptr)
    {
        return {};
    }
    const JsonParser parser(json);

    const auto in = parser.byteArray("inBytes");
    if (in.empty())
    {
        return {};
    }

    const auto key = parser.byteArray("keyBytes");
    if (key.empty())
    {
        return {};
    }

    const auto iv = parser.byteArray("initVectorBytes");
    if (iv.empty())
    {
        return {};
    }

    const auto aad = parser.byteArray("aadBytes");

    const auto tagBits = parser.num<size_t>("tagBits");

    const auto outBytes = ngenxxCryptoAesGcmDecrypt(in, key, iv, tagBits.value_or(0), aad);
    return bytes2json(outBytes);
}

std::string ngenxx_crypto_rsa_gen_keyS(const char *json)
{
    if (json == nullptr)
    {
        return {};
    }
    const JsonParser parser(json);

    const auto base64 = parser.str("base64");
    if (base64 == std::nullopt)
    {
        return {};
    }

    const auto isPublic = parser.num<bool>("isPublic");
    if (isPublic == std::nullopt)
    {
        return {};
    }

    return ngenxxCryptoRsaGenKey(base64.value(), isPublic.value());
}

std::string ngenxx_crypto_rsa_encryptS(const char *json)
{
    if (json == nullptr)
    {
        return {};
    }
    const JsonParser parser(json);

    const auto in = parser.byteArray("inBytes");
    if (in.empty())
    {
        return {};
    }

    const auto key = parser.byteArray("keyBytes");
    if (key.empty())
    {
        return {};
    }

    const auto padding = parser.num<NGenXXCryptoRSAPaddingX>("padding");
    if (padding == std::nullopt)
    {
        return {};
    }

    const auto outBytes = ngenxxCryptoRsaEncrypt(in, key, padding.value());
    return bytes2json(outBytes);
}

std::string ngenxx_crypto_rsa_decryptS(const char *json)
{
    if (json == nullptr)
    {
        return {};
    }
    const JsonParser parser(json);

    const auto in = parser.byteArray("inBytes");
    if (in.empty())
    {
        return {};
    }

    const auto key = parser.byteArray("keyBytes");
    if (key.empty())
    {
        return {};
    }

    const auto padding = parser.num<NGenXXCryptoRSAPaddingX>("padding");
    if (padding == std::nullopt)
    {
        return {};
    }

    const auto outBytes = ngenxxCryptoRsaDecrypt(in, key, padding.value());
    return bytes2json(outBytes);
}

std::string ngenxx_crypto_hash_md5S(const char *json)
{
    if (json == nullptr)
    {
        return {};
    }
    const JsonParser parser(json);

    const auto in = parser.byteArray("inBytes");
    if (in.empty())
    {
        return {};
    }

    const auto outBytes = ngenxxCryptoHashMd5(in);
    return bytes2json(outBytes);
}

std::string ngenxx_crypto_hash_sha256S(const char *json)
{
    if (json == nullptr)
    {
        return {};
    }
    const JsonParser parser(json);

    const auto in = parser.byteArray("inBytes");
    if (in.empty())
    {
        return {};
    }

    const auto outBytes = ngenxxCryptoHashSha256(in);
    return bytes2json(outBytes);
}

std::string ngenxx_crypto_base64_encodeS(const char *json)
{
    if (json == nullptr)
    {
        return {};
    }
    const JsonParser parser(json);

    const auto in = parser.byteArray("inBytes");
    const auto noNewLines = parser.num<bool>("noNewLines");
    if (in.empty())
    {
        return {};
    }

    const auto outBytes = ngenxxCryptoBase64Encode(in, noNewLines.value_or(true));
    return bytes2json(outBytes);
}

std::string ngenxx_crypto_base64_decodeS(const char *json)
{
    if (json == nullptr)
    {
        return {};
    }
    const JsonParser parser(json);

    const auto in = parser.byteArray("inBytes");
    const auto noNewLines = parser.num<bool>("noNewLines");
    if (in.empty())
    {
        return {};
    }

    const auto outBytes = ngenxxCryptoBase64Decode(in, noNewLines.value_or(true));
    return bytes2json(outBytes);
}

#pragma mark Zip

std::string ngenxx_z_zip_initS(const char *json)
{
    if (json == nullptr)
    {
        return {};
    }
    const JsonParser parser(json);
    const auto [mode, bufferSize, format] = parser.numX<NGenXXZipCompressModeX, size_t, NGenXXZFormatX>("mode", "bufferSize", "format");

    if (mode == std::nullopt || bufferSize == std::nullopt || format == std::nullopt)
    {
        return {};
    }

    const auto zip = ngenxxZZipInit(mode.value(), bufferSize.value(), format.value());
    if (zip == nullptr)
    {
        return {};
    }
    return std::to_string(ptr2addr(zip));
}

size_t ngenxx_z_zip_inputS(const char *json)
{
    if (json == nullptr)
    {
        return 0;
    }
    const JsonParser parser(json);
    const auto zip = parser.ptr("zip");
    if (zip == nullptr)
    {
        return 0;
    }

    const auto in = parser.byteArray("inBytes");
    if (in.empty())
    {
        return 0;
    }

    const auto inFinish = parser.num<bool>("inFinish");
    if (inFinish == std::nullopt)
    {
        return 0;
    }

    return ngenxxZZipInput(zip, in, inFinish.value());
}

std::string ngenxx_z_zip_process_doS(const char *json)
{
    if (json == nullptr)
    {
        return {};
    }
    const JsonParser parser(json);
    const auto zip = parser.ptr("zip");
    if (zip == nullptr)
    {
        return {};
    }

    const auto outBytes = ngenxxZZipProcessDo(zip);
    return bytes2json(outBytes);
}

bool ngenxx_z_zip_process_finishedS(const char *json)
{
    if (json == nullptr)
    {
        return false;
    }
    const JsonParser parser(json);
    const auto zip = parser.ptr("zip");
    if (zip == nullptr)
    {
        return false;
    }

    return ngenxxZZipProcessFinished(zip);
}

void ngenxx_z_zip_releaseS(const char *json)
{
    if (json == nullptr)
    {
        return;
    }
    const JsonParser parser(json);
    const auto zip = parser.ptr("zip");
    if (zip == nullptr)
    {
        return;
    }

    ngenxxZZipRelease(zip);
}

std::string ngenxx_z_unzip_initS(const char *json)
{
    if (json == nullptr)
    {
        return {};
    }
    const JsonParser parser(json);
    const auto [bufferSize, format] = parser.numX<size_t, NGenXXZFormatX>("bufferSize", "format");
    if (bufferSize == std::nullopt || format == std::nullopt)
    {
        return {};
    }

    const auto unzip = ngenxxZUnzipInit(bufferSize.value(), format.value());
    if (unzip == nullptr)
    {
        return {};
    }
    return std::to_string(ptr2addr(unzip));
}

size_t ngenxx_z_unzip_inputS(const char *json)
{
    if (json == nullptr)
    {
        return 0;
    }
    const JsonParser parser(json);
    const auto unzip = parser.ptr("unzip");
    if (unzip == nullptr)
    {
        return 0;
    }

    const auto in = parser.byteArray("inBytes");
    if (in.empty())
    {
        return 0;
    }

    const auto inFinish = parser.num<bool>("inFinish");
    if (inFinish == std::nullopt)
    {
        return 0;
    }

    return ngenxxZUnzipInput(unzip, in, inFinish.value());
}

std::string ngenxx_z_unzip_process_doS(const char *json)
{
    if (json == nullptr)
    {
        return {};
    }
    const JsonParser parser(json);
    const auto unzip = parser.ptr("unzip");
    if (unzip == nullptr)
    {
        return {};
    }

    const auto outBytes = ngenxxZUnzipProcessDo(unzip);
    return bytes2json(outBytes);
}

bool ngenxx_z_unzip_process_finishedS(const char *json)
{
    if (json == nullptr)
    {
        return false;
    }
    const JsonParser parser(json);
    const auto unzip = parser.ptr("unzip");
    if (unzip == nullptr)
    {
        return false;
    }

    return ngenxxZUnzipProcessFinished(unzip);
}

void ngenxx_z_unzip_releaseS(const char *json)
{
    if (json == nullptr)
    {
        return;
    }
    const JsonParser parser(json);
    const auto unzip = parser.ptr("unzip");
    if (unzip == nullptr)
    {
        return;
    }

    ngenxxZUnzipRelease(unzip);
}

std::string ngenxx_z_bytes_zipS(const char *json)
{
    if (json == nullptr)
    {
        return {};
    }
    const JsonParser parser(json);
    const auto [mode, bufferSize, format] = parser.numX<NGenXXZipCompressModeX, size_t, NGenXXZFormatX>("mode", "bufferSize", "format");
    const auto in = parser.byteArray("inBytes");
    if (in.empty() || mode == std::nullopt || bufferSize == std::nullopt || format == std::nullopt)
    {
        return {};
    }

    const auto outBytes = ngenxxZBytesZip(in, mode.value(), bufferSize.value(), format.value());
    return bytes2json(outBytes);
}

std::string ngenxx_z_bytes_unzipS(const char *json)
{
    if (json == nullptr)
    {
        return {};
    }
    const JsonParser parser(json);
    const auto [bufferSize, format] = parser.numX<size_t, NGenXXZFormatX>("bufferSize", "format");
    const auto in = parser.byteArray("inBytes");
    if (in.empty() || bufferSize == std::nullopt || format == std::nullopt)
    {
        return {};
    }

    const auto outBytes = ngenxxZBytesUnzip(in, bufferSize.value(), format.value());
    return bytes2json(outBytes);
}

#endif