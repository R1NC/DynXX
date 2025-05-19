#include "NGenXX-Script.hxx"

#include <algorithm>
#include <functional>
#if defined(USE_STD_RANGES)
#include <ranges>
#endif

#include <NGenXX.hxx>
#include <NGenXXNetHttp.h>
#include "json/JsonCodec.hxx"

namespace
{
    std::string bytes2json(const Bytes &bytes)
    {
        if (bytes.empty()) [[unlikely]]
        {
            return {};
        }
#if defined(USE_STD_RANGES)
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
        auto json = ngenxxJsonCJSON2Str(cj);
        cJSON_Delete(cj);
        return json;
    }

    std::string strArray2json(const std::vector<std::string> &v)
    {
        const auto cj = cJSON_CreateArray();
        for (const auto &it : v)
        {
            cJSON_AddItemToArray(cj, cJSON_CreateString(it.c_str()));
        }
        auto json = ngenxxJsonCJSON2Str(cj);
        cJSON_Delete(cj);
        return json;
    }

    template <NumberT T>
    T parseNum(const NGenXX::Json::Decoder &decoder, const char *k)
    {
        return static_cast<T>(decoder.readNumber(decoder[k]));
    }

    std::string parseStr(const NGenXX::Json::Decoder &decoder, const char *k)
    {
        return decoder.readString(decoder[k]);
    }

    template <typename T = void>
    T* parsePtr(const NGenXX::Json::Decoder &decoder, const char *k)
    {
        address addr = 0;
        const auto s = parseStr(decoder, k);
        if (s.empty())
        {
            return nullptr;
        }
    
        addr = str2int64(s);

        return addr2ptr<T>(addr);
    }

    Bytes parseByteArray(const NGenXX::Json::Decoder &decoder, const char *bytesK)
    {
        Bytes data;
        if (const auto byte_vNode = decoder[bytesK])
        {
            decoder.readChildren(byte_vNode, 
                            [&data, &decoder](size_t, const void *const child)
                            {
                                data.emplace_back(decoder.readNumber(child));
                            });
        }
        return data;
    }

    std::vector<std::string> parseStrArray(const NGenXX::Json::Decoder &decoder, const char *strVK)
    {
        std::vector<std::string> v;
        if (const auto str_vNode = decoder[strVK])
        {
            decoder.readChildren(str_vNode,
                             [&v, &decoder](size_t, const void *const child)
                             {
                                 v.emplace_back(decoder.readString(child));
                             });
        }
        return v;
    }
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
    const NGenXX::Json::Decoder decoder(json);
    const auto level = parseNum<NGenXXLogLevelX>(decoder, "level");
    const auto content = parseStr(decoder, "content");
    if (content.empty())
    {
        return;
    }

    ngenxxLogPrint(level, content);
}

#pragma mark Net.Http

std::string ngenxx_net_http_requestS(const char *json)
{
    if (json == nullptr)
    {
        return {};
    }
    const NGenXX::Json::Decoder decoder(json);
    const auto url = parseStr(decoder, "url");
    const auto params = parseStr(decoder, "params");
    const auto method = parseNum<NGenXXHttpMethodX>(decoder, "method");

    const auto rawBody = parseByteArray(decoder, "rawBodyBytes");

    const auto header_v = parseStrArray(decoder, "header_v");

    const auto form_field_name_v = parseStrArray(decoder, "form_field_name_v");
    const auto form_field_mime_v = parseStrArray(decoder, "form_field_mime_v");
    const auto form_field_data_v = parseStrArray(decoder, "form_field_data_v");

    const auto cFILE = parsePtr<std::FILE>(decoder, "cFILE");
    const auto fileSize = parseNum<size_t>(decoder, "file_size");

    const auto timeout = parseNum<size_t>(decoder, "timeout");

    const auto header_c = header_v.size();
    const auto form_field_count = form_field_name_v.size();
    if (url.empty() || header_c > NGENXX_HTTP_HEADER_MAX_COUNT)
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
    if ((cFILE != nullptr && fileSize == 0) || (cFILE == nullptr && fileSize > 0)) [[unlikely]]
    {
        return {};
    }

    const auto t = ngenxxNetHttpRequest(url, method, params, rawBody,
                                  header_v,
                                  form_field_name_v,
                                  form_field_mime_v,
                                  form_field_data_v,
                                  cFILE,
                                  fileSize,
                                  timeout);
    return t.toJson();
}

bool ngenxx_net_http_downloadS(const char *json)
{
    if (json == nullptr)
    {
        return false;
    }
    const NGenXX::Json::Decoder decoder(json);
    const auto url = parseStr(decoder, "url");
    const auto file = parseStr(decoder, "file");
    const auto timeout = parseNum<size_t>(decoder, "timeout");

    if (url.empty() || file.empty())
    {
        return false;
    }

    return ngenxxNetHttpDownload(url, file, timeout);
}

#pragma mark Store.SQLite

std::string ngenxx_store_sqlite_openS(const char *json)
{
    if (json == nullptr)
    {
        return {};
    }
    const NGenXX::Json::Decoder decoder(json);
    const auto _id = parseStr(decoder, "_id");
    if (_id.empty())
    {
        return {};
    }

    const auto db = ngenxxStoreSqliteOpen(_id);
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
    const NGenXX::Json::Decoder decoder(json);
    const auto conn = parsePtr(decoder, "conn");
    const auto sql = parseStr(decoder, "sql");
    if (conn == nullptr || sql.empty())
    {
        return false;
    }

    return ngenxxStoreSqliteExecute(conn, sql);
}

std::string ngenxx_store_sqlite_query_doS(const char *json)
{
    if (json == nullptr)
    {
        return {};
    }
    const NGenXX::Json::Decoder decoder(json);
    const auto conn = parsePtr(decoder, "conn");
    const auto sql = parseStr(decoder, "sql");
    if (conn == nullptr || sql.empty())
    {
        return {};
    }

    const auto res = ngenxxStoreSqliteQueryDo(conn, sql);
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
    const NGenXX::Json::Decoder decoder(json);
    const auto query_result = parsePtr(decoder, "query_result");
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
    const NGenXX::Json::Decoder decoder(json);
    const auto query_result = parsePtr(decoder, "query_result");
    const auto column = parseStr(decoder, "column");
    if (query_result == nullptr || column.empty())
    {
        return {};
    }

    return ngenxxStoreSqliteQueryReadColumnText(query_result, column);
}

int64_t ngenxx_store_sqlite_query_read_column_integerS(const char *json)
{
    if (json == nullptr)
    {
        return 0;
    }
    const NGenXX::Json::Decoder decoder(json);
    const auto query_result = parsePtr(decoder, "query_result");
    const auto column = parseStr(decoder, "column");
    if (query_result == nullptr || column.empty())
    {
        return 0;
    }

    return ngenxxStoreSqliteQueryReadColumnInteger(query_result, column);
}

double ngenxx_store_sqlite_query_read_column_floatS(const char *json)
{
    if (json == nullptr)
    {
        return 0;
    }
    const NGenXX::Json::Decoder decoder(json);
    const auto query_result = parsePtr(decoder, "query_result");
    const auto column = parseStr(decoder, "column");
    if (query_result == nullptr || column.empty())
    {
        return 0;
    }

    return ngenxxStoreSqliteQueryReadColumnFloat(query_result, column);
}

void ngenxx_store_sqlite_query_dropS(const char *json)
{
    if (json == nullptr)
    {
        return;
    }
    const NGenXX::Json::Decoder decoder(json);
    const auto query_result = parsePtr(decoder, "query_result");
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
    const NGenXX::Json::Decoder decoder(json);
    const auto conn = parsePtr(decoder, "conn");
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
    const NGenXX::Json::Decoder decoder(json);
    const auto _id = parseStr(decoder, "_id");
    if (_id.empty())
    {
        return {};
    }

    const auto res = ngenxxStoreKvOpen(_id);
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
    const NGenXX::Json::Decoder decoder(json);
    const auto conn = parsePtr(decoder, "conn");
    const auto k = parseStr(decoder, "k");
    if (conn == nullptr || k.empty())
    {
        return {};
    }

    return ngenxxStoreKvReadString(conn, k);
}

bool ngenxx_store_kv_write_stringS(const char *json)
{
    if (json == nullptr)
    {
        return false;
    }
    const NGenXX::Json::Decoder decoder(json);
    const auto conn = parsePtr(decoder, "conn");
    const auto k = parseStr(decoder, "k");
    const auto v = parseStr(decoder, "v");
    if (conn == nullptr || k.empty())
    {
        return false;
    }

    return ngenxxStoreKvWriteString(conn, k, v);
}

int64_t ngenxx_store_kv_read_integerS(const char *json)
{
    if (json == nullptr)
    {
        return 0;
    }
    const NGenXX::Json::Decoder decoder(json);
    const auto conn = parsePtr(decoder, "conn");
    const auto k = parseStr(decoder, "k");
    if (conn == nullptr || k.empty())
    {
        return 0;
    }

    return ngenxxStoreKvReadInteger(conn, k);
}

bool ngenxx_store_kv_write_integerS(const char *json)
{
    if (json == nullptr)
    {
        return false;
    }
    const NGenXX::Json::Decoder decoder(json);
    const auto conn = parsePtr(decoder, "conn");
    const auto k = parseStr(decoder, "k");
    const auto v = parseNum<int64_t>(decoder, "v");
    if (conn == nullptr || k.empty())
    {
        return false;
    }

    return ngenxxStoreKvWriteInteger(conn, k, v);
}

double ngenxx_store_kv_read_floatS(const char *json)
{
    if (json == nullptr)
    {
        return 0;
    }
    const NGenXX::Json::Decoder decoder(json);
    const auto conn = parsePtr(decoder, "conn");
    const auto k = parseStr(decoder, "k");
    if (conn == nullptr || k.empty())
    {
        return false;
    }

    return ngenxxStoreKvReadFloat(conn, k);
}

bool ngenxx_store_kv_write_floatS(const char *json)
{
    if (json == nullptr)
    {
        return false;
    }
    const NGenXX::Json::Decoder decoder(json);
    const auto conn = parsePtr(decoder, "conn");
    const auto k = parseStr(decoder, "k");
    const auto v = parseNum<double>(decoder, "v");
    if (conn == nullptr || k.empty())
    {
        return false;
    }

    return ngenxxStoreKvWriteFloat(conn, k, v);
}

std::string ngenxx_store_kv_all_keysS(const char *json)
{
    if (json == nullptr)
    {
        return {};
    }
    const NGenXX::Json::Decoder decoder(json);
    const auto conn = parsePtr(decoder, "conn");
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
    const NGenXX::Json::Decoder decoder(json);
    const auto conn = parsePtr(decoder, "conn");
    const auto k = parseStr(decoder, "k");
    if (conn == nullptr || k.empty())
    {
        return false;
    }

    return ngenxxStoreKvContains(conn, k);
}

bool ngenxx_store_kv_removeS(const char *json)
{
    if (json == nullptr)
    {
        return false;
    }
    const NGenXX::Json::Decoder decoder(json);
    const auto conn = parsePtr(decoder, "conn");
    const auto k = parseStr(decoder, "k");
    if (conn == nullptr || k.empty())
    {
        return false;
    }

    return ngenxxStoreKvRemove(conn, k);
}

void ngenxx_store_kv_clearS(const char *json)
{
    if (json == nullptr)
    {
        return;
    }
    const NGenXX::Json::Decoder decoder(json);
    const auto conn = parsePtr(decoder, "conn");
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
    const NGenXX::Json::Decoder decoder(json);
    const auto conn = parsePtr(decoder, "conn");
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
    const NGenXX::Json::Decoder decoder(json);

    const auto in = parseByteArray(decoder, "inBytes");
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
    const NGenXX::Json::Decoder decoder(json);

    const auto str = parseStr(decoder, "str");
    if (str.empty())
    {
        return {};
    }

    const auto bytes = ngenxxCodingHexStr2bytes(str);
    return bytes2json(bytes);
}

std::string ngenxx_coding_bytes2strS(const char *json)
{
    if (json == nullptr)
    {
        return {};
    }
    const NGenXX::Json::Decoder decoder(json);

    const auto in = parseByteArray(decoder, "inBytes");
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
    const NGenXX::Json::Decoder decoder(json);

    const auto str = parseStr(decoder, "str");
    if (str.empty())
    {
        return {};
    }

    const auto bytes = ngenxxCodingStr2bytes(str);
    return bytes2json(bytes);
}

std::string ngenxx_coding_case_upperS(const char *json)
{
    if (json == nullptr)
    {
        return {};
    }
    const NGenXX::Json::Decoder decoder(json);

    const auto str = parseStr(decoder, "str");
    if (str.empty())
    {
        return {};
    }
    return ngenxxCodingCaseUpper(str);
}

std::string ngenxx_coding_case_lowerS(const char *json)
{
    if (json == nullptr)
    {
        return {};
    }
    const NGenXX::Json::Decoder decoder(json);

    const auto str = parseStr(decoder, "str");
    if (str.empty())
    {
        return {};
    }
    return ngenxxCodingCaseLower(str);
}

#pragma mark Crypto

std::string ngenxx_crypto_randS(const char *json)
{
    if (json == nullptr)
    {
        return {};
    }
    const NGenXX::Json::Decoder decoder(json);
    const auto outLen = parseNum<size_t>(decoder, "len");
    if (outLen == 0)
    {
        return {};
    }
    byte outBytes[outLen];

    ngenxxCryptoRand(outLen, outBytes);
    return bytes2json(wrapBytes(outBytes, outLen));
}

std::string ngenxx_crypto_aes_encryptS(const char *json)
{
    if (json == nullptr)
    {
        return {};
    }
    const NGenXX::Json::Decoder decoder(json);

    const auto in = parseByteArray(decoder, "inBytes");
    if (in.empty())
    {
        return {};
    }

    const auto key = parseByteArray(decoder, "keyBytes");
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
    const NGenXX::Json::Decoder decoder(json);

    const auto in = parseByteArray(decoder, "inBytes");
    if (in.empty())
    {
        return {};
    }

    const auto key = parseByteArray(decoder, "keyBytes");
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
    const NGenXX::Json::Decoder decoder(json);

    const auto in = parseByteArray(decoder, "inBytes");
    if (in.empty())
    {
        return {};
    }

    const auto key = parseByteArray(decoder, "keyBytes");
    if (key.empty())
    {
        return {};
    }

    const auto iv = parseByteArray(decoder, "initVectorBytes");
    if (iv.empty())
    {
        return {};
    }

    const auto aad = parseByteArray(decoder, "aadBytes");

    const auto tagBits = parseNum<size_t>(decoder, "tagBits");

    const auto outBytes = ngenxxCryptoAesGcmEncrypt(in, key, iv, tagBits, aad);
    return bytes2json(outBytes);
}

std::string ngenxx_crypto_aes_gcm_decryptS(const char *json)
{
    if (json == nullptr)
    {
        return {};
    }
    const NGenXX::Json::Decoder decoder(json);

    const auto in = parseByteArray(decoder, "inBytes");
    if (in.empty())
    {
        return {};
    }

    const auto key = parseByteArray(decoder, "keyBytes");
    if (key.empty())
    {
        return {};
    }

    const auto iv = parseByteArray(decoder, "initVectorBytes");
    if (iv.empty())
    {
        return {};
    }

    const auto aad = parseByteArray(decoder, "aadBytes");

    const auto tagBits = parseNum<size_t>(decoder, "tagBits");

    const auto outBytes = ngenxxCryptoAesGcmDecrypt(in, key, iv, tagBits, aad);
    return bytes2json(outBytes);
}

std::string ngenxx_crypto_hash_md5S(const char *json)
{
    if (json == nullptr)
    {
        return {};
    }
    const NGenXX::Json::Decoder decoder(json);

    const auto in = parseByteArray(decoder, "inBytes");
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
    const NGenXX::Json::Decoder decoder(json);

    const auto in = parseByteArray(decoder, "inBytes");
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
    const NGenXX::Json::Decoder decoder(json);

    const auto in = parseByteArray(decoder, "inBytes");
    if (in.empty())
    {
        return {};
    }

    const auto outBytes = ngenxxCryptoBase64Encode(in);
    return bytes2json(outBytes);
}

std::string ngenxx_crypto_base64_decodeS(const char *json)
{
    if (json == nullptr)
    {
        return {};
    }
    const NGenXX::Json::Decoder decoder(json);

    const auto in = parseByteArray(decoder, "inBytes");
    if (in.empty())
    {
        return {};
    }

    const auto outBytes = ngenxxCryptoBase64Decode(in);
    return bytes2json(outBytes);
}

#pragma mark Zip

std::string ngenxx_z_zip_initS(const char *json)
{
    if (json == nullptr)
    {
        return {};
    }
    const NGenXX::Json::Decoder decoder(json);
    const auto mode = parseNum<NGenXXZipCompressModeX>(decoder, "mode");
    const auto bufferSize = parseNum<size_t>(decoder, "bufferSize");
    const auto format = parseNum<NGenXXZFormatX>(decoder, "format");

    const auto zip = ngenxxZZipInit(mode, bufferSize, format);
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
    const NGenXX::Json::Decoder decoder(json);
    const auto zip = parsePtr(decoder, "zip");
    if (zip == nullptr)
    {
        return 0;
    }

    const auto in = parseByteArray(decoder, "inBytes");
    if (in.empty())
    {
        return 0;
    }

    const auto inFinish = parseNum<bool>(decoder, "inFinish");

    return ngenxxZZipInput(zip, in, inFinish);
}

std::string ngenxx_z_zip_process_doS(const char *json)
{
    if (json == nullptr)
    {
        return {};
    }
    const NGenXX::Json::Decoder decoder(json);
    const auto zip = parsePtr(decoder, "zip");
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
    const NGenXX::Json::Decoder decoder(json);
    const auto zip = parsePtr(decoder, "zip");
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
    const NGenXX::Json::Decoder decoder(json);
    const auto zip = parsePtr(decoder, "zip");
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
    const NGenXX::Json::Decoder decoder(json);
    const auto bufferSize = parseNum<size_t>(decoder, "bufferSize");
    const auto format = parseNum<NGenXXZFormatX>(decoder, "format");

    const auto unzip = ngenxxZUnzipInit(bufferSize, format);
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
    const NGenXX::Json::Decoder decoder(json);
    const auto unzip = parsePtr(decoder, "unzip");
    if (unzip == nullptr)
    {
        return 0;
    }

    const auto in = parseByteArray(decoder, "inBytes");
    if (in.empty())
    {
        return 0;
    }

    const auto inFinish = parseNum<bool>(decoder, "inFinish");

    return ngenxxZUnzipInput(unzip, in, inFinish);
}

std::string ngenxx_z_unzip_process_doS(const char *json)
{
    if (json == nullptr)
    {
        return {};
    }
    const NGenXX::Json::Decoder decoder(json);
    const auto unzip = parsePtr(decoder, "unzip");
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
    const NGenXX::Json::Decoder decoder(json);
    const auto unzip = parsePtr(decoder, "unzip");
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
    const NGenXX::Json::Decoder decoder(json);
    const auto unzip = parsePtr(decoder, "unzip");
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
    const NGenXX::Json::Decoder decoder(json);
    const auto mode = parseNum<NGenXXZipCompressModeX>(decoder, "mode");
    const auto bufferSize = parseNum<size_t>(decoder, "bufferSize");
    const auto format = parseNum<NGenXXZFormatX>(decoder, "format");
    const auto in = parseByteArray(decoder, "inBytes");
    if (in.empty())
    {
        return {};
    }

    const auto outBytes = ngenxxZBytesZip(in, mode, bufferSize, format);
    return bytes2json(outBytes);
}

std::string ngenxx_z_bytes_unzipS(const char *json)
{
    if (json == nullptr)
    {
        return {};
    }
    const NGenXX::Json::Decoder decoder(json);
    const auto bufferSize = parseNum<size_t>(decoder, "bufferSize");
    const auto format = parseNum<NGenXXZFormatX>(decoder, "format");
    const auto in = parseByteArray(decoder, "inBytes");
    if (in.empty())
    {
        return {};
    }

    const auto outBytes = ngenxxZBytesUnzip(in, bufferSize, format);
    return bytes2json(outBytes);
}