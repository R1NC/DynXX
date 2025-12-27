#if defined(USE_QJS) || defined(USE_LUA)

#include "ScriptAPI.hxx"

#if defined(__cpp_lib_ranges)
#include <ranges>
#endif

#include <DynXX/CXX/DynXX.hxx>
#include <DynXX/C/Net.h>
#include "../core/json/JsonCodec.hxx"
#include "../core/util/MemUtil.hxx"

namespace
{
    using namespace DynXX::Core::Json;
    using namespace DynXX::Core::Util::Mem;

    std::string cjToStr(cJSON* cj) {
        return dynxxJsonNodeToStr(ptr2addr(cj)).value_or("");
    }

    std::string bytes2json(BytesView bytes)
    {
        if (bytes.empty()) [[unlikely]]
        {
            return {};
        }
#if defined(__cpp_lib_ranges_to_container)
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
        const auto json = cjToStr(cj);
        cJSON_Delete(cj);
        return json;
    }

    std::string strArray2json(const std::vector<std::string> &v)
    {
        const auto cj = cJSON_CreateArray();
        for (const auto& it : v)
        {
            cJSON_AddItemToArray(cj, cJSON_CreateString(it.c_str()));
        }
        const auto json = cjToStr(cj);
        cJSON_Delete(cj);
        return json;
    }

    template <typename T> 
    requires (IntegerT<T> || EnumT<T>)
    std::optional<T> parseNum(const Decoder &decoder, std::string_view k)
    {
        const auto node = decoder[k];
        if (node == 0) [[unlikely]]
        {
            return std::nullopt;
        }
        return decoder.readNumInt<T>(node);
    }

    template <FloatT T>
    std::optional<T> parseNum(const Decoder &decoder, std::string_view k)
    {
        const auto node = decoder[k];
        if (node == 0) [[unlikely]]
        {
            return std::nullopt;
        }
        return decoder.readNumFloat<T>(node);
    }

    template <NumberT... Ns, KeyT... Ks>
    auto parseNumX(const Decoder &decoder, Ks... ks)
    {
        return std::make_tuple(parseNum<Ns>(decoder, ks)...);
    }

    std::optional<std::string> parseStr(const Decoder &decoder, std::string_view k)
    {
        const auto node = decoder[k];
        return node == 0 ? std::nullopt : decoder.readString(node);
    }

    template <KeyT... Ks>
    auto parseStrX(const Decoder &decoder, Ks... ks)
    {
        return std::make_tuple(parseStr(decoder, ks)...);
    }

    template <typename T = void>
    T* parsePtr(const Decoder &decoder, std::string_view k)
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

    Bytes parseByteArray(const Decoder &decoder, std::string_view bytesK)
    {
        Bytes data;
        if (const auto byte_vNode = decoder[bytesK])
        {
            const auto len = decoder.readChildrenCount(byte_vNode);
            data.reserve(len);
            decoder.readChildren(byte_vNode, 
                            [&data, &decoder](size_t, DynXXJsonNodeHandle childNode, DynXXJsonNodeTypeX, std::string_view)
                            {
                                if (const auto n = decoder.readNumInt<byte>(childNode); n.has_value())
                                {
                                    data.emplace_back(n.value());
                                }
                            });
        }
        return data;
    }

    std::vector<std::string> parseStrArray(const Decoder &decoder, std::string_view strVK)
    {
        std::vector<std::string> v;
        if (const auto str_vNode = decoder[strVK])
        {
            const auto len = decoder.readChildrenCount(str_vNode);
            v.reserve(len);
            decoder.readChildren(str_vNode,
                             [&v, &decoder](size_t, DynXXJsonNodeHandle childNode, DynXXJsonNodeTypeX, std::string_view)
                             {
                                 v.emplace_back(decoder.readString(childNode).value_or(""));
                             });
        }
        return v;
    }

    class JsonParser
    {
    public:
        JsonParser() = delete;
        explicit JsonParser(std::string_view json) : decoder(json) {}

        bool valid() const { 
            return this->decoder.valid(); 
        }

        std::optional<std::string> str(std::string_view k) const
        {
            if (!this->valid()) [[unlikely]]
            {
                return std::nullopt;
            }
            return parseStr(this->decoder, k);
        }

        template <NumberT T>
        std::optional<T> num(std::string_view k) const
        {
            if (!this->valid()) [[unlikely]]
            {
                return std::nullopt;
            }
            return parseNum<T>(this->decoder, k);
        }

        address addr(std::string_view k) const
        {
            if (!this->valid()) [[unlikely]]
            {
                return 0;
            }
            return this->num<address>(k).value_or(0);
        }

        Bytes byteArray(std::string_view k) const
        {
            if (!this->valid()) [[unlikely]]
            {
                return {};
            }
            return parseByteArray(this->decoder, k);
        }

        std::vector<std::string> strArray(std::string_view k) const
        {
            if (!this->valid()) [[unlikely]]
            {
                return {};
            }
            return parseStrArray(this->decoder, k);
        }

        template <typename T = void>
        T* ptr(std::string_view k) const
        {
            if (!this->valid()) [[unlikely]]
            {
                return nullptr;
            }
            return parsePtr<T>(this->decoder, k);
        }

        template <KeyT... Ks>
        auto strX(Ks... ks) const
        {
            return std::make_tuple(parseStr(this->decoder, ks)...);
        }

        template <NumberT... Ns, KeyT... Ks>
        auto numX(Ks... ks) const
        {
            return std::make_tuple(parseNum<Ns>(this->decoder, ks)...);
        }

    private:
        Decoder decoder;
    };
}

std::string dynxx_get_versionS([[maybe_unused]] std::string_view json)
{
    return dynxxGetVersion();
}

std::string dynxx_root_pathS([[maybe_unused]] std::string_view json)
{
    return dynxxRootPath().value_or("");
}

// Device

#if defined(USE_DEVICE)

int dynxx_device_typeS([[maybe_unused]] std::string_view json)
{
    return static_cast<int>(dynxxDeviceType());
}

std::string dynxx_device_nameS([[maybe_unused]] std::string_view json)
{
    return dynxxDeviceName();
}

std::string dynxx_device_manufacturerS([[maybe_unused]] std::string_view json)
{
    return dynxxDeviceManufacturer();
}

std::string dynxx_device_os_versionS([[maybe_unused]] std::string_view json)
{
    return dynxxDeviceOsVersion();
}

int dynxx_device_cpu_archS([[maybe_unused]] std::string_view json)
{
    return static_cast<int>(dynxxDeviceCpuArch());
}

#endif

// Log

void dynxx_log_printS(std::string_view json)
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

std::string dynxx_net_http_requestS(std::string_view json)
{
    const JsonParser parser(json);
    if (!parser.valid())
    {
        return {};
    }
    
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

bool dynxx_net_http_downloadS(std::string_view json)
{
    const JsonParser parser(json);
    if (!parser.valid())
    {
        return false;
    }
    const auto [url, file] = parser.strX("url", "file");
    const auto timeout = parser.num<size_t>("timeout");

    if (url == std::nullopt || file == std::nullopt)
    {
        return false;
    }

    return dynxxNetHttpDownload(url.value(), file.value(), timeout.value_or(0));
}

// SQLite

#if defined(USE_DB)

std::string dynxx_sqlite_openS(std::string_view json)
{
    const JsonParser parser(json);
    if (!parser.valid())
    {
        return {};
    }
    const auto _id = parser.str("_id");
    if (_id == std::nullopt)
    {
        return {};
    }

    const auto db = dynxxSQLiteOpen(_id.value());
    if (db == 0)
    {
        return {};
    }
    return std::to_string(db);
}

bool dynxx_sqlite_executeS(std::string_view json)
{
    const JsonParser parser(json);
    if (!parser.valid())
    {
        return false;
    }
    const auto conn = parser.addr("conn");
    const auto sql = parser.str("sql");
    if (conn == 0 || sql == std::nullopt)
    {
        return false;
    }

    return dynxxSQLiteExecute(conn, sql.value());
}

std::string dynxx_sqlite_query_doS(std::string_view json)
{
    const JsonParser parser(json);
    if (!parser.valid())
    {
        return {};
    }
    const auto conn = parser.addr("conn");
    const auto sql = parser.str("sql");
    if (conn == 0 || sql == std::nullopt)
    {
        return {};
    }

    const auto res = dynxxSQLiteQueryDo(conn, sql.value());
    if (res == 0)
    {
        return {};
    }
    return std::to_string(res);
}

bool dynxx_sqlite_query_read_rowS(std::string_view json)
{
    const JsonParser parser(json);
    if (!parser.valid())
    {
        return false;
    }
    const auto query_result = parser.addr("query_result");
    if (query_result == 0)
    {
        return false;
    }

    return dynxxSQLiteQueryReadRow(query_result);
}

std::string dynxx_sqlite_query_read_column_textS(std::string_view json)
{
    const JsonParser parser(json);
    if (!parser.valid())
    {
        return {};
    }
    const auto query_result = parser.addr("query_result");
    const auto column = parser.str("column");
    if (query_result == 0 || column == std::nullopt)
    {
        return {};
    }

    const auto s = dynxxSQLiteQueryReadColumnText(query_result, column.value());
    return s.value_or(std::string{});
}

int64_t dynxx_sqlite_query_read_column_integerS(std::string_view json)
{
    const JsonParser parser(json);
    if (!parser.valid())
    {
        return 0;
    }
    const auto query_result = parser.addr("query_result");
    const auto column = parser.str("column");
    if (query_result == 0 || column == std::nullopt)
    {
        return 0;
    }

    const auto i = dynxxSQLiteQueryReadColumnInteger(query_result, column.value());
    return i.value_or(0);
}

double dynxx_sqlite_query_read_column_floatS(std::string_view json)
{
    const JsonParser parser(json);
    if (!parser.valid())
    {
        return 0;
    }
    const auto query_result = parser.addr("query_result");
    const auto column = parser.str("column");
    if (query_result == 0 || column == std::nullopt)
    {
        return 0;
    }

    const auto f = dynxxSQLiteQueryReadColumnFloat(query_result, column.value());
    return f.value_or(0.0);
}

void dynxx_sqlite_query_dropS(std::string_view json)
{
    const JsonParser parser(json);
    if (!parser.valid())
    {
        return;
    }
    const auto query_result = parser.addr("query_result");
    if (query_result == 0)
    {
        return;
    }

    dynxxSQLiteQueryDrop(query_result);
}

void dynxx_sqlite_closeS(std::string_view json)
{
    const JsonParser parser(json);
    if (!parser.valid())
    {
        return;
    }
    const auto conn = parser.addr("conn");
    if (conn == 0)
    {
        return;
    }

    dynxxSQLiteClose(conn);
}

#endif

// KV

#if defined(USE_KV)

std::string dynxx_kv_openS(std::string_view json)
{
    const JsonParser parser(json);
    if (!parser.valid())
    {
        return {};
    }
    const auto _id = parser.str("_id");
    if (_id == std::nullopt)
    {
        return {};
    }

    const auto res = dynxxKVOpen(_id.value());
    if (res == 0)
    {
        return {};
    }
    return std::to_string(res);
}

std::string dynxx_kv_read_stringS(std::string_view json)
{
    const JsonParser parser(json);
    if (!parser.valid())
    {
        return {};
    }
    const auto conn = parser.addr("conn");
    const auto k = parser.str("k");
    if (conn == 0 || k == std::nullopt)
    {
        return {};
    }

    const auto s = dynxxKVReadString(conn, k.value());
    return s.value_or(std::string{});
}

bool dynxx_kv_write_stringS(std::string_view json)
{
    const JsonParser parser(json);
    if (!parser.valid())
    {
        return false;
    }
    const auto conn = parser.addr("conn");
    const auto [k, v] = parser.strX("k", "v");
    if (conn == 0 || k == std::nullopt)
    {
        return false;
    }

    return dynxxKVWriteString(conn, k.value(), v.value_or(""));
}

int64_t dynxx_kv_read_integerS(std::string_view json)
{
    const JsonParser parser(json);
    if (!parser.valid())
    {
        return 0;
    }
    const auto conn = parser.addr("conn");
    const auto k = parser.str("k");
    if (conn == 0 || k == std::nullopt)
    {
        return 0;
    }

    const auto i = dynxxKVReadInteger(conn, k.value());
    return i.value_or(0);
}

bool dynxx_kv_write_integerS(std::string_view json)
{
    const JsonParser parser(json);
    if (!parser.valid())
    {
        return false;
    }
    const auto conn = parser.addr("conn");
    const auto k = parser.str("k");
    const auto v = parser.num<int64_t>("v");
    if (conn == 0 || k == std::nullopt)
    {
        return false;
    }

    return dynxxKVWriteInteger(conn, k.value(), v.value_or(0));
}

double dynxx_kv_read_floatS(std::string_view json)
{
    const JsonParser parser(json);
    if (!parser.valid())
    {
        return 0;
    }
    const auto conn = parser.addr("conn");
    const auto k = parser.str("k");
    if (conn == 0 || k == std::nullopt)
    {
        return 0;
    }

    const auto f = dynxxKVReadFloat(conn, k.value());
    return f.value_or(0.0);
}

bool dynxx_kv_write_floatS(std::string_view json)
{
    const JsonParser parser(json);
    if (!parser.valid())
    {
        return false;
    }
    const auto conn = parser.addr("conn");
    const auto k = parser.str("k");
    const auto v = parser.num<double>("v");
    if (conn == 0 || k == std::nullopt)
    {
        return false;
    }

    return dynxxKVWriteFloat(conn, k.value(), v.value_or(0.0));
}

std::string dynxx_kv_all_keysS(std::string_view json)
{
    const JsonParser parser(json);
    if (!parser.valid())
    {
        return {};
    }
    const auto conn = parser.addr("conn");
    if (conn == 0)
    {
        return {};
    }

    const auto res = dynxxKVAllKeys(conn);
    return strArray2json(res);
}

bool dynxx_kv_containsS(std::string_view json)
{
    const JsonParser parser(json);
    if (!parser.valid())
    {
        return false;
    }
    const auto conn = parser.addr("conn");
    const auto k = parser.str("k");
    if (conn == 0 || k == std::nullopt)
    {
        return false;
    }

    return dynxxKVContains(conn, k.value());
}

bool dynxx_kv_removeS(std::string_view json)
{
    const JsonParser parser(json);
    if (!parser.valid())
    {
        return false;
    }
    const auto conn = parser.addr("conn");
    const auto k = parser.str("k");
    if (conn == 0 || k == std::nullopt)
    {
        return false;
    }

    return dynxxKVRemove(conn, k.value());
}

void dynxx_kv_clearS(std::string_view json)
{
    const JsonParser parser(json);
    if (!parser.valid())
    {
        return;
    }
    const auto conn = parser.addr("conn");
    if (conn == 0)
    {
        return;
    }

    dynxxKVClear(conn);
}

void dynxx_kv_closeS(std::string_view json)
{
    const JsonParser parser(json);
    if (!parser.valid())
    {
        return;
    }
    const auto conn = parser.addr("conn");
    if (conn == 0)
    {
        return;
    }

    dynxxKVClose(conn);
}

#endif

// Coding

std::string dynxx_coding_case_upperS(std::string_view json)
{
    const JsonParser parser(json);
    if (!parser.valid())
    {
        return {};
    }

    const auto str = parser.str("str");
    if (str == std::nullopt)
    {
        return {};
    }
    return dynxxCodingCaseUpper(str.value());
}

std::string dynxx_coding_case_lowerS(std::string_view json)
{
    const JsonParser parser(json);
    if (!parser.valid())
    {
        return {};
    }

    const auto str = parser.str("str");
    if (str == std::nullopt)
    {
        return {};
    }
    return dynxxCodingCaseLower(str.value());
}

std::string dynxx_coding_hex_bytes2strS(std::string_view json)
{
    const JsonParser parser(json);
    if (!parser.valid())
    {
        return {};
    }

    const auto in = parser.byteArray("inBytes");
    if (in.empty())
    {
        return {};
    }

    return dynxxCodingHexBytes2str(in);
}

std::string dynxx_coding_hex_str2bytesS(std::string_view json)
{
    const JsonParser parser(json);
    if (!parser.valid())
    {
        return {};
    }

    const auto str = parser.str("str");
    if (str == std::nullopt)
    {
        return {};
    }

    const auto bytes = dynxxCodingHexStr2bytes(str.value());
    return bytes2json(bytes);
}

std::string dynxx_coding_bytes2strS(std::string_view json)
{
    const JsonParser parser(json);
    if (!parser.valid())
    {
        return {};
    }

    const auto in = parser.byteArray("inBytes");
    if (in.empty())
    {
        return {};
    }

    return dynxxCodingBytes2str(in);
}

std::string dynxx_coding_str2bytesS(std::string_view json)
{
    const JsonParser parser(json);
    if (!parser.valid())
    {
        return {};
    }

    const auto str = parser.str("str");
    if (str == std::nullopt)
    {
        return {};
    }

    const auto bytes = dynxxCodingStr2bytes(str.value());
    return bytes2json(bytes);
}

// Crypto

std::string dynxx_crypto_randS(std::string_view json)
{
    const JsonParser parser(json);
    if (!parser.valid())
    {
        return {};
    }
    const auto outLen = parser.num<size_t>("len");
    if (outLen == std::nullopt)
    {
        return {};
    }
    const auto outBytes = dynxxCryptoRand(outLen.value());
    return bytes2json(outBytes);
}

std::string dynxx_crypto_aes_encryptS(std::string_view json)
{
    const JsonParser parser(json);
    if (!parser.valid())
    {
        return {};
    }

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

std::string dynxx_crypto_aes_decryptS(std::string_view json)
{
    const JsonParser parser(json);
    if (!parser.valid())
    {
        return {};
    }

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

std::string dynxx_crypto_aes_gcm_encryptS(std::string_view json)
{
    const JsonParser parser(json);
    if (!parser.valid())
    {
        return {};
    }

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

std::string dynxx_crypto_aes_gcm_decryptS(std::string_view json)
{
    const JsonParser parser(json);
    if (!parser.valid())
    {
        return {};
    }

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

std::string dynxx_crypto_rsa_gen_keyS(std::string_view json)
{
    const JsonParser parser(json);
    if (!parser.valid())
    {
        return {};
    }

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

std::string dynxx_crypto_rsa_encryptS(std::string_view json)
{
    const JsonParser parser(json);
    if (!parser.valid())
    {
        return {};
    }

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

std::string dynxx_crypto_rsa_decryptS(std::string_view json)
{
    const JsonParser parser(json);
    if (!parser.valid())
    {
        return {};
    }

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

std::string dynxx_crypto_hash_md5S(std::string_view json)
{
    const JsonParser parser(json);
    if (!parser.valid())
    {
        return {};
    }

    const auto in = parser.byteArray("inBytes");
    if (in.empty())
    {
        return {};
    }

    const auto outBytes = dynxxCryptoHashMd5(in);
    return bytes2json(outBytes);
}

std::string dynxx_crypto_hash_sha1S(std::string_view json)
{
    const JsonParser parser(json);
    if (!parser.valid())
    {
        return {};
    }

    const auto in = parser.byteArray("inBytes");
    if (in.empty())
    {
        return {};
    }

    const auto outBytes = dynxxCryptoHashSha1(in);
    return bytes2json(outBytes);
}

std::string dynxx_crypto_hash_sha256S(std::string_view json)
{
    const JsonParser parser(json);
    if (!parser.valid())
    {
        return {};
    }

    const auto in = parser.byteArray("inBytes");
    if (in.empty())
    {
        return {};
    }

    const auto outBytes = dynxxCryptoHashSha256(in);
    return bytes2json(outBytes);
}

std::string dynxx_crypto_base64_encodeS(std::string_view json)
{
    const JsonParser parser(json);
    if (!parser.valid())
    {
        return {};
    }

    const auto in = parser.byteArray("inBytes");
    const auto noNewLines = parser.num<bool>("noNewLines");
    if (in.empty())
    {
        return {};
    }

    const auto outBytes = dynxxCryptoBase64Encode(in, noNewLines.value_or(true));
    return bytes2json(outBytes);
}

std::string dynxx_crypto_base64_decodeS(std::string_view json)
{
    const JsonParser parser(json);
    if (!parser.valid())
    {
        return {};
    }

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

std::string dynxx_z_zip_initS(std::string_view json)
{
    const JsonParser parser(json);
    if (!parser.valid())
    {
        return {};
    }
    const auto [mode, bufferSize, format] = parser.numX<DynXXZipCompressModeX, size_t, DynXXZFormatX>("mode", "bufferSize", "format");

    if (mode == std::nullopt || bufferSize == std::nullopt || format == std::nullopt)
    {
        return {};
    }

    const auto zip = dynxxZZipInit(mode.value(), bufferSize.value(), format.value());
    if (zip == 0)
    {
        return {};
    }
    return std::to_string(zip);
}

size_t dynxx_z_zip_inputS(std::string_view json)
{
    const JsonParser parser(json);
    if (!parser.valid())
    {
        return 0;
    }
    const auto zip = parser.addr("zip");
    if (zip == 0)
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

std::string dynxx_z_zip_process_doS(std::string_view json)
{
    const JsonParser parser(json);
    if (!parser.valid())
    {
        return {};
    }
    const auto zip = parser.addr("zip");
    if (zip == 0)
    {
        return {};
    }

    const auto outBytes = dynxxZZipProcessDo(zip);
    return bytes2json(outBytes);
}

bool dynxx_z_zip_process_finishedS(std::string_view json)
{
    const JsonParser parser(json);
    if (!parser.valid())
    {
        return false;
    }
    const auto zip = parser.addr("zip");
    if (zip == 0)
    {
        return false;
    }

    return dynxxZZipProcessFinished(zip);
}

void dynxx_z_zip_releaseS(std::string_view json)
{
    const JsonParser parser(json);
    if (!parser.valid())
    {
        return;
    }
    const auto zip = parser.addr("zip");
    if (zip == 0)
    {
        return;
    }

    dynxxZZipRelease(zip);
}

std::string dynxx_z_unzip_initS(std::string_view json)
{
    const JsonParser parser(json);
    if (!parser.valid())
    {
        return {};
    }
    const auto [bufferSize, format] = parser.numX<size_t, DynXXZFormatX>("bufferSize", "format");
    if (bufferSize == std::nullopt || format == std::nullopt)
    {
        return {};
    }

    const auto unzip = dynxxZUnzipInit(bufferSize.value(), format.value());
    if (unzip == 0)
    {
        return {};
    }
    return std::to_string(unzip);
}

size_t dynxx_z_unzip_inputS(std::string_view json)
{
    const JsonParser parser(json);
    if (!parser.valid())
    {
        return 0;
    }
    const auto unzip = parser.addr("unzip");
    if (unzip == 0)
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

std::string dynxx_z_unzip_process_doS(std::string_view json)
{
    const JsonParser parser(json);
    if (!parser.valid())
    {
        return {};
    }
    const auto unzip = parser.addr("unzip");
    if (unzip == 0)
    {
        return {};
    }

    const auto outBytes = dynxxZUnzipProcessDo(unzip);
    return bytes2json(outBytes);
}

bool dynxx_z_unzip_process_finishedS(std::string_view json)
{
    const JsonParser parser(json);
    if (!parser.valid())
    {
        return false;
    }
    const auto unzip = parser.addr("unzip");
    if (unzip == 0)
    {
        return false;
    }

    return dynxxZUnzipProcessFinished(unzip);
}

void dynxx_z_unzip_releaseS(std::string_view json)
{
    const JsonParser parser(json);
    if (!parser.valid())
    {
        return;
    }
    const auto unzip = parser.addr("unzip");
    if (unzip == 0)
    {
        return;
    }

    dynxxZUnzipRelease(unzip);
}

std::string dynxx_z_bytes_zipS(std::string_view json)
{
    const JsonParser parser(json);
    if (!parser.valid())
    {
        return {};
    }
    const auto [mode, bufferSize, format] = parser.numX<DynXXZipCompressModeX, size_t, DynXXZFormatX>("mode", "bufferSize", "format");
    const auto in = parser.byteArray("inBytes");
    if (in.empty() || mode == std::nullopt || bufferSize == std::nullopt || format == std::nullopt)
    {
        return {};
    }

    const auto outBytes = dynxxZBytesZip(in, mode.value(), bufferSize.value(), format.value());
    return bytes2json(outBytes);
}

std::string dynxx_z_bytes_unzipS(std::string_view json)
{
    const JsonParser parser(json);
    if (!parser.valid())
    {
        return {};
    }
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
