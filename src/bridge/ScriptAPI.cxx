#if defined(USE_QJS) || defined(USE_LUA)

#include "ScriptAPI.hxx"

#include <functional>
#if defined(__cpp_lib_ranges)
#include <ranges>
#endif

#include <DynXX/CXX/DynXX.hxx>
#include <DynXX/C/Net.h>
#include "../core/json/JsonCodec.hxx"

namespace
{
    using namespace DynXX::Core::Json;

    std::string bytes2json(const BytesView bytes)
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
        const auto json = dynxxJsonToStr(cj);
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
        const auto json = dynxxJsonToStr(cj);
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
        explicit JsonParser(const std::string_view json) : decoder(json) {}

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

std::string dynxx_get_versionS([[maybe_unused]] const std::string_view json)
{
    return dynxxGetVersion();
}

std::string dynxx_root_pathS([[maybe_unused]] const std::string_view json)
{
    return dynxxRootPath();
}

// Device.DeviceInfo

int dynxx_device_typeS([[maybe_unused]] const std::string_view json)
{
    return static_cast<int>(dynxxDeviceType());
}

std::string dynxx_device_nameS([[maybe_unused]] const std::string_view json)
{
    return dynxxDeviceName();
}

std::string dynxx_device_manufacturerS([[maybe_unused]] const std::string_view json)
{
    return dynxxDeviceManufacturer();
}

std::string dynxx_device_os_versionS([[maybe_unused]] const std::string_view json)
{
    return dynxxDeviceOsVersion();
}

int dynxx_device_cpu_archS([[maybe_unused]] const std::string_view json)
{
    return static_cast<int>(dynxxDeviceCpuArch());
}

// Log

void dynxx_log_printS(const std::string_view json)
{
    if (json.empty())
    {
        return;
    }
    const JsonParser parser(json);
    const auto level = parser.num<DynXXLogLevelX>("level");
    const auto content = parser.str("content");
    if (level == std::nullopt || content == std::nullopt)
    {
        return;
    }

    dynxxLogPrint(level.value(), content.value());
}

// Net.Http

std::string dynxx_net_http_requestS(const std::string_view json)
{
    if (json.empty())
    {
        return {};
    }
    const JsonParser parser(json);
    const auto [url, params] = parser.strX("url", "params");
    const auto [method, fileSize, timeout] = parser.numX<DynXXHttpMethodX, size_t, size_t>("method", "fileSize", "timeout");

    const auto rawBody = parser.byteArray("rawBodyBytes");

    const auto header_v = parser.strArray("header_v");

    const auto form_field_name_v = parser.strArray("form_field_name_v");
    const auto form_field_mime_v = parser.strArray("form_field_mime_v");
    const auto form_field_data_v = parser.strArray("form_field_data_v");

    const auto cFILE = parser.ptr<std::FILE>("cFILE");

    const auto header_c = header_v.size();
    const auto form_field_count = form_field_name_v.size();
    if (url == std::nullopt || method == std::nullopt || header_c > DYNXX_HTTP_HEADER_MAX_COUNT)
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

    const auto t = dynxxNetHttpRequest(url.value(), method.value(), 
                        params.value_or(""), rawBody, header_v,
                        form_field_name_v, form_field_mime_v, form_field_data_v,
                        cFILE, fileSize.value_or(0), 
                        timeout.value_or(0));
    return t.toJson().value_or("");
}

bool dynxx_net_http_downloadS(const std::string_view json)
{
    if (json.empty())
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

    return dynxxNetHttpDownload(url.value(), file.value(), timeout.value_or(0));
}

// Store.SQLite

std::string dynxx_store_sqlite_openS(const std::string_view json)
{
    if (json.empty())
    {
        return {};
    }
    const JsonParser parser(json);
    const auto _id = parser.str("_id");
    if (_id == std::nullopt)
    {
        return {};
    }

    const auto db = dynxxStoreSqliteOpen(_id.value());
    if (db == nullptr)
    {
        return {};
    }
    return std::to_string(ptr2addr(db));
}

bool dynxx_store_sqlite_executeS(const std::string_view json)
{
    if (json.empty())
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

    return dynxxStoreSqliteExecute(conn, sql.value());
}

std::string dynxx_store_sqlite_query_doS(const std::string_view json)
{
    if (json.empty())
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

    const auto res = dynxxStoreSqliteQueryDo(conn, sql.value());
    if (res == nullptr)
    {
        return {};
    }
    return std::to_string(ptr2addr(res));
}

bool dynxx_store_sqlite_query_read_rowS(const std::string_view json)
{
    if (json.empty())
    {
        return false;
    }
    const JsonParser parser(json);
    const auto query_result = parser.ptr("query_result");
    if (query_result == nullptr)
    {
        return false;
    }

    return dynxxStoreSqliteQueryReadRow(query_result);
}

std::string dynxx_store_sqlite_query_read_column_textS(const std::string_view json)
{
    if (json.empty())
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

    const auto s = dynxxStoreSqliteQueryReadColumnText(query_result, column.value());
    return s.value_or(std::string{});
}

int64_t dynxx_store_sqlite_query_read_column_integerS(const std::string_view json)
{
    if (json.empty())
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

    const auto i = dynxxStoreSqliteQueryReadColumnInteger(query_result, column.value());
    return i.value_or(0);
}

double dynxx_store_sqlite_query_read_column_floatS(const std::string_view json)
{
    if (json.empty())
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

    const auto f = dynxxStoreSqliteQueryReadColumnFloat(query_result, column.value());
    return f.value_or(0.0);
}

void dynxx_store_sqlite_query_dropS(const std::string_view json)
{
    if (json.empty())
    {
        return;
    }
    const JsonParser parser(json);
    const auto query_result = parser.ptr("query_result");
    if (query_result == nullptr)
    {
        return;
    }

    dynxxStoreSqliteQueryDrop(query_result);
}

void dynxx_store_sqlite_closeS(const std::string_view json)
{
    if (json.empty())
    {
        return;
    }
    const JsonParser parser(json);
    const auto conn = parser.ptr("conn");
    if (conn == nullptr)
    {
        return;
    }

    dynxxStoreSqliteClose(conn);
}

// Store.KV

std::string dynxx_store_kv_openS(const std::string_view json)
{
    if (json.empty())
    {
        return {};
    }
    const JsonParser parser(json);
    const auto _id = parser.str("_id");
    if (_id == std::nullopt)
    {
        return {};
    }

    const auto res = dynxxStoreKvOpen(_id.value());
    if (res == nullptr)
    {
        return {};
    }
    return std::to_string(ptr2addr(res));
}

std::string dynxx_store_kv_read_stringS(const std::string_view json)
{
    if (json.empty())
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

    const auto s = dynxxStoreKvReadString(conn, k.value());
    return s.value_or(std::string{});
}

bool dynxx_store_kv_write_stringS(const std::string_view json)
{
    if (json.empty())
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

    return dynxxStoreKvWriteString(conn, k.value(), v.value_or(""));
}

int64_t dynxx_store_kv_read_integerS(const std::string_view json)
{
    if (json.empty())
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

    const auto i = dynxxStoreKvReadInteger(conn, k.value());
    return i.value_or(0);
}

bool dynxx_store_kv_write_integerS(const std::string_view json)
{
    if (json.empty())
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

    return dynxxStoreKvWriteInteger(conn, k.value(), v.value_or(0));
}

double dynxx_store_kv_read_floatS(const std::string_view json)
{
    if (json.empty())
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

    const auto f = dynxxStoreKvReadFloat(conn, k.value());
    return f.value_or(0.0);
}

bool dynxx_store_kv_write_floatS(const std::string_view json)
{
    if (json.empty())
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

    return dynxxStoreKvWriteFloat(conn, k.value(), v.value_or(0.0));
}

std::string dynxx_store_kv_all_keysS(const std::string_view json)
{
    if (json.empty())
    {
        return {};
    }
    const JsonParser parser(json);
    const auto conn = parser.ptr("conn");
    if (conn == nullptr)
    {
        return {};
    }

    const auto res = dynxxStoreKvAllKeys(conn);
    return strArray2json(res);
}

bool dynxx_store_kv_containsS(const std::string_view json)
{
    if (json.empty())
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

    return dynxxStoreKvContains(conn, k.value());
}

bool dynxx_store_kv_removeS(const std::string_view json)
{
    if (json.empty())
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

    return dynxxStoreKvRemove(conn, k.value());
}

void dynxx_store_kv_clearS(const std::string_view json)
{
    if (json.empty())
    {
        return;
    }
    const JsonParser parser(json);
    const auto conn = parser.ptr("conn");
    if (conn == nullptr)
    {
        return;
    }

    dynxxStoreKvClear(conn);
}

void dynxx_store_kv_closeS(const std::string_view json)
{
    if (json.empty())
    {
        return;
    }
    const JsonParser parser(json);
    const auto conn = parser.ptr("conn");
    if (conn == nullptr)
    {
        return;
    }

    dynxxStoreKvClose(conn);
}

// Coding

std::string dynxx_coding_case_upperS(const std::string_view json)
{
    if (json.empty())
    {
        return {};
    }
    const JsonParser parser(json);

    const auto str = parser.str("str");
    if (str == std::nullopt)
    {
        return {};
    }
    return dynxxCodingCaseUpper(str.value());
}

std::string dynxx_coding_case_lowerS(const std::string_view json)
{
    if (json.empty())
    {
        return {};
    }
    const JsonParser parser(json);

    const auto str = parser.str("str");
    if (str == std::nullopt)
    {
        return {};
    }
    return dynxxCodingCaseLower(str.value());
}

std::string dynxx_coding_hex_bytes2strS(const std::string_view json)
{
    if (json.empty())
    {
        return {};
    }
    const JsonParser parser(json);

    const auto in = parser.byteArray("inBytes");
    if (in.empty())
    {
        return {};
    }

    return dynxxCodingHexBytes2str(in);
}

std::string dynxx_coding_hex_str2bytesS(const std::string_view json)
{
    if (json.empty())
    {
        return {};
    }
    const JsonParser parser(json);

    const auto str = parser.str("str");
    if (str == std::nullopt)
    {
        return {};
    }

    const auto bytes = dynxxCodingHexStr2bytes(str.value());
    return bytes2json(bytes);
}

std::string dynxx_coding_bytes2strS(const std::string_view json)
{
    if (json.empty())
    {
        return {};
    }
    const JsonParser parser(json);

    const auto in = parser.byteArray("inBytes");
    if (in.empty())
    {
        return {};
    }

    return dynxxCodingBytes2str(in);
}

std::string dynxx_coding_str2bytesS(const std::string_view json)
{
    if (json.empty())
    {
        return {};
    }
    const JsonParser parser(json);

    const auto str = parser.str("str");
    if (str == std::nullopt)
    {
        return {};
    }

    const auto bytes = dynxxCodingStr2bytes(str.value());
    return bytes2json(bytes);
}

// Crypto

std::string dynxx_crypto_randS(const std::string_view json)
{
    if (json.empty())
    {
        return {};
    }
    const JsonParser parser(json);
    const auto outLen = parser.num<size_t>("len");
    if (outLen == std::nullopt)
    {
        return {};
    }
    const auto outBytes = dynxxCryptoRand(outLen.value());
    return bytes2json(outBytes);
}

std::string dynxx_crypto_aes_encryptS(const std::string_view json)
{
    if (json.empty())
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

    const auto outBytes = dynxxCryptoAesEncrypt(in, key);
    return bytes2json(outBytes);
}

std::string dynxx_crypto_aes_decryptS(const std::string_view json)
{
    if (json.empty())
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

    const auto outBytes = dynxxCryptoAesDecrypt(in, key);
    return bytes2json(outBytes);
}

std::string dynxx_crypto_aes_gcm_encryptS(const std::string_view json)
{
    if (json.empty())
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

    const auto outBytes = dynxxCryptoAesGcmEncrypt(in, key, iv, tagBits.value_or(0), aad);
    return bytes2json(outBytes);
}

std::string dynxx_crypto_aes_gcm_decryptS(const std::string_view json)
{
    if (json.empty())
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

    const auto outBytes = dynxxCryptoAesGcmDecrypt(in, key, iv, tagBits.value_or(0), aad);
    return bytes2json(outBytes);
}

std::string dynxx_crypto_rsa_gen_keyS(const std::string_view json)
{
    if (json.empty())
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

    return dynxxCryptoRsaGenKey(base64.value(), isPublic.value());
}

std::string dynxx_crypto_rsa_encryptS(const std::string_view json)
{
    if (json.empty())
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

    const auto padding = parser.num<DynXXCryptoRSAPaddingX>("padding");
    if (padding == std::nullopt)
    {
        return {};
    }

    const auto outBytes = dynxxCryptoRsaEncrypt(in, key, padding.value());
    return bytes2json(outBytes);
}

std::string dynxx_crypto_rsa_decryptS(const std::string_view json)
{
    if (json.empty())
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

    const auto padding = parser.num<DynXXCryptoRSAPaddingX>("padding");
    if (padding == std::nullopt)
    {
        return {};
    }

    const auto outBytes = dynxxCryptoRsaDecrypt(in, key, padding.value());
    return bytes2json(outBytes);
}

std::string dynxx_crypto_hash_md5S(const std::string_view json)
{
    if (json.empty())
    {
        return {};
    }
    const JsonParser parser(json);

    const auto in = parser.byteArray("inBytes");
    if (in.empty())
    {
        return {};
    }

    const auto outBytes = dynxxCryptoHashMd5(in);
    return bytes2json(outBytes);
}

std::string dynxx_crypto_hash_sha1S(const std::string_view json)
{
    if (json.empty())
    {
        return {};
    }
    const JsonParser parser(json);

    const auto in = parser.byteArray("inBytes");
    if (in.empty())
    {
        return {};
    }

    const auto outBytes = dynxxCryptoHashSha1(in);
    return bytes2json(outBytes);
}

std::string dynxx_crypto_hash_sha256S(const std::string_view json)
{
    if (json.empty())
    {
        return {};
    }
    const JsonParser parser(json);

    const auto in = parser.byteArray("inBytes");
    if (in.empty())
    {
        return {};
    }

    const auto outBytes = dynxxCryptoHashSha256(in);
    return bytes2json(outBytes);
}

std::string dynxx_crypto_base64_encodeS(const std::string_view json)
{
    if (json.empty())
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

    const auto outBytes = dynxxCryptoBase64Encode(in, noNewLines.value_or(true));
    return bytes2json(outBytes);
}

std::string dynxx_crypto_base64_decodeS(const std::string_view json)
{
    if (json.empty())
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

    const auto outBytes = dynxxCryptoBase64Decode(in, noNewLines.value_or(true));
    return bytes2json(outBytes);
}

// Zip

std::string dynxx_z_zip_initS(const std::string_view json)
{
    if (json.empty())
    {
        return {};
    }
    const JsonParser parser(json);
    const auto [mode, bufferSize, format] = parser.numX<DynXXZipCompressModeX, size_t, DynXXZFormatX>("mode", "bufferSize", "format");

    if (mode == std::nullopt || bufferSize == std::nullopt || format == std::nullopt)
    {
        return {};
    }

    const auto zip = dynxxZZipInit(mode.value(), bufferSize.value(), format.value());
    if (zip == nullptr)
    {
        return {};
    }
    return std::to_string(ptr2addr(zip));
}

size_t dynxx_z_zip_inputS(const std::string_view json)
{
    if (json.empty())
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

    return dynxxZZipInput(zip, in, inFinish.value());
}

std::string dynxx_z_zip_process_doS(const std::string_view json)
{
    if (json.empty())
    {
        return {};
    }
    const JsonParser parser(json);
    const auto zip = parser.ptr("zip");
    if (zip == nullptr)
    {
        return {};
    }

    const auto outBytes = dynxxZZipProcessDo(zip);
    return bytes2json(outBytes);
}

bool dynxx_z_zip_process_finishedS(const std::string_view json)
{
    if (json.empty())
    {
        return false;
    }
    const JsonParser parser(json);
    const auto zip = parser.ptr("zip");
    if (zip == nullptr)
    {
        return false;
    }

    return dynxxZZipProcessFinished(zip);
}

void dynxx_z_zip_releaseS(const std::string_view json)
{
    if (json.empty())
    {
        return;
    }
    const JsonParser parser(json);
    const auto zip = parser.ptr("zip");
    if (zip == nullptr)
    {
        return;
    }

    dynxxZZipRelease(zip);
}

std::string dynxx_z_unzip_initS(const std::string_view json)
{
    if (json.empty())
    {
        return {};
    }
    const JsonParser parser(json);
    const auto [bufferSize, format] = parser.numX<size_t, DynXXZFormatX>("bufferSize", "format");
    if (bufferSize == std::nullopt || format == std::nullopt)
    {
        return {};
    }

    const auto unzip = dynxxZUnzipInit(bufferSize.value(), format.value());
    if (unzip == nullptr)
    {
        return {};
    }
    return std::to_string(ptr2addr(unzip));
}

size_t dynxx_z_unzip_inputS(const std::string_view json)
{
    if (json.empty())
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

    return dynxxZUnzipInput(unzip, in, inFinish.value());
}

std::string dynxx_z_unzip_process_doS(const std::string_view json)
{
    if (json.empty())
    {
        return {};
    }
    const JsonParser parser(json);
    const auto unzip = parser.ptr("unzip");
    if (unzip == nullptr)
    {
        return {};
    }

    const auto outBytes = dynxxZUnzipProcessDo(unzip);
    return bytes2json(outBytes);
}

bool dynxx_z_unzip_process_finishedS(const std::string_view json)
{
    if (json.empty())
    {
        return false;
    }
    const JsonParser parser(json);
    const auto unzip = parser.ptr("unzip");
    if (unzip == nullptr)
    {
        return false;
    }

    return dynxxZUnzipProcessFinished(unzip);
}

void dynxx_z_unzip_releaseS(const std::string_view json)
{
    if (json.empty())
    {
        return;
    }
    const JsonParser parser(json);
    const auto unzip = parser.ptr("unzip");
    if (unzip == nullptr)
    {
        return;
    }

    dynxxZUnzipRelease(unzip);
}

std::string dynxx_z_bytes_zipS(const std::string_view json)
{
    if (json.empty())
    {
        return {};
    }
    const JsonParser parser(json);
    const auto [mode, bufferSize, format] = parser.numX<DynXXZipCompressModeX, size_t, DynXXZFormatX>("mode", "bufferSize", "format");
    const auto in = parser.byteArray("inBytes");
    if (in.empty() || mode == std::nullopt || bufferSize == std::nullopt || format == std::nullopt)
    {
        return {};
    }

    const auto outBytes = dynxxZBytesZip(in, mode.value(), bufferSize.value(), format.value());
    return bytes2json(outBytes);
}

std::string dynxx_z_bytes_unzipS(const std::string_view json)
{
    if (json.empty())
    {
        return {};
    }
    const JsonParser parser(json);
    const auto [bufferSize, format] = parser.numX<size_t, DynXXZFormatX>("bufferSize", "format");
    const auto in = parser.byteArray("inBytes");
    if (in.empty() || bufferSize == std::nullopt || format == std::nullopt)
    {
        return {};
    }

    const auto outBytes = dynxxZBytesUnzip(in, bufferSize.value(), format.value());
    return bytes2json(outBytes);
}

#endif