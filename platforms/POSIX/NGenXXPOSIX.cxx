#include "NGenXXPOSIX.h"

#include "../../include/NGenXX.h"

#include <iostream>
#include <fstream>
#include <cstring>

static auto cParamsJson = "{\"url\":\"https://rinc.xyz\", \"params\":\"p0=1&p1=2&p2=3\", \"method\":0, \"header_v\":[\"Cache-Control: no-cache\"], \"timeout\":6666}";

void ngenxx_posix_init(const char *root)
{
   ngenxx_init(root);
}

void ngenxx_posix_testHttpL()
{
    if (ngenxx_lua_loadF("../Android/app/src/main/assets/biz.lua"))
    {
        const auto cRsp = ngenxx_lua_call("lNetHttpRequest", cParamsJson);
        std::cout << cRsp << std::endl;
    }
}

void ngenxx_posix_testHttpJ()
{
    if (ngenxx_lua_loadF("../Android/app/src/main/assets/NGenXX.js") && ngenxx_js_loadF("../Android/app/src/main/assets/biz.js", false))
    {
        const char *cRsp = ngenxx_js_call("NGenXXHttpReq", cParamsJson, false);
        std::cout << cRsp << std::endl;
    }
}

void ngenxx_posix_testDB()
{
    if (const auto dbConn = ngenxx_store_sqlite_open("test"))
    {
        const auto insertSQL = "CREATE TABLE IF NOT EXISTS TestTable (_id INTEGER PRIMARY KEY AUTOINCREMENT, platform TEXT, vendor TEXT); \
            INSERT OR IGNORE INTO TestTable (platform, vendor) \
            VALUES \
            ('iOS','Apple'), \
            ('Android','Google'), \
            ('HarmonyOS','Huawei');";
        if (ngenxx_store_sqlite_execute(dbConn, insertSQL))
        {
            const auto querySQL = "SELECT * FROM TestTable;";
            if (const auto queryResult = ngenxx_store_sqlite_query_do(dbConn, querySQL))
            {
                while (ngenxx_store_sqlite_query_read_row(queryResult))
                {
                    const auto platform = ngenxx_store_sqlite_query_read_column_text(queryResult, "platform");
                    const auto vendor = ngenxx_store_sqlite_query_read_column_text(queryResult, "vendor");
                    std::cout << vendor << "->" << platform << std::endl;
                }
               ngenxx_store_sqlite_query_drop(queryResult);
            }
        }
       ngenxx_store_sqlite_close(dbConn);
    }
}

void ngenxx_posix_testKV()
{
    if (const auto kvConn = ngenxx_store_kv_open("test"))
    {
       ngenxx_store_kv_write_string(kvConn, "s", "NGenXX");
        const auto s = ngenxx_store_kv_read_string(kvConn, "s");
        std::cout << "s->" << s << std::endl;
       ngenxx_store_kv_write_integer(kvConn, "i", 1234567890);
        const auto i = ngenxx_store_kv_read_integer(kvConn, "i");
        std::cout << "i->" << i << std::endl;
       ngenxx_store_kv_write_float(kvConn, "f", 0.123456789f);
        const auto f = ngenxx_store_kv_read_float(kvConn, "f");
        std::cout << "f->" << f << std::endl;
       ngenxx_store_kv_close(kvConn);
    }
}

void ngenxx_posix_testCrypto()
{
    const auto inStr = "0123456789qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM<>()[]{}|,;:'`~!@#$%^&*-=_+/";
    const auto inBytes = (unsigned char *) inStr;
    const auto inLen = std::strlen(inStr);
    const auto keyStr = "MNBVCXZLKJHGFDSA";
    const byte *keyBytes = (unsigned char *) keyStr;
    const auto keyLen = std::strlen(keyStr);
    
    size_t aesEncodedLen;
    const auto aesEncodedBytes = ngenxx_crypto_aes_encrypt(inBytes, inLen, keyBytes, keyLen, &aesEncodedLen);
    if (aesEncodedBytes && aesEncodedLen > 0)
    {
        size_t aesDecodedLen;
        const byte *aesDecodedBytes = ngenxx_crypto_aes_decrypt(aesEncodedBytes, aesEncodedLen, keyBytes, keyLen, &aesDecodedLen);
        if (aesDecodedBytes && aesDecodedLen > 0)
        {
            std::cout << "AES: " << (char *)aesDecodedBytes << std::endl;
        }
    }
    
    size_t ivLen = 12;
    byte ivBytes[ivLen];
    ngenxx_crypto_rand(ivLen, ivBytes);
    size_t aesgcmTagBits = 15 * 8;
    
    size_t aesgcmEncodedLen;
    const auto aesgcmEncodedBytes = ngenxx_crypto_aes_gcm_encrypt(inBytes, inLen, keyBytes, keyLen, ivBytes, ivLen, nullptr, 0, aesgcmTagBits, &aesgcmEncodedLen);
    if (aesgcmEncodedBytes && aesgcmEncodedLen > 0)
    {
        size_t aesgcmDecodedLen;
        const auto aesgcmDecodedBytes = ngenxx_crypto_aes_gcm_decrypt(aesgcmEncodedBytes, aesgcmEncodedLen, keyBytes, keyLen, ivBytes, ivLen, nullptr, 0, aesgcmTagBits, &aesgcmDecodedLen);
        if (aesgcmDecodedBytes && aesgcmDecodedLen > 0)
        {
            std::cout << "AES-GCM: " << (char *)aesgcmDecodedBytes << std::endl;
        }
    }
}

void ngenxx_posix_testJsonDecoder()
{
    if (const auto jsonDecoder = ngenxx_json_decoder_init(cParamsJson))
    {
        if (const auto urlNode = ngenxx_json_decoder_read_node(jsonDecoder, nullptr, "url"))
        {
            const auto url = ngenxx_json_decoder_read_string(jsonDecoder, urlNode);
            std::cout << "url:" << url << std::endl;
        }
        if (const auto methodNode = ngenxx_json_decoder_read_node(jsonDecoder, nullptr, "method"))
        {
            const auto method = ngenxx_json_decoder_read_number(jsonDecoder, methodNode);
            std::cout << "method:" << method << std::endl;
        }
        if (const auto headerVNode = ngenxx_json_decoder_read_node(jsonDecoder, nullptr, "header_v"))
        {
            auto headerNode = ngenxx_json_decoder_read_child(jsonDecoder, headerVNode);
            while (headerNode)
            {
                const auto header = ngenxx_json_decoder_read_string(jsonDecoder, headerNode);
                std::cout << "header:" << header << std::endl;
                headerNode = ngenxx_json_decoder_read_next(jsonDecoder, headerNode);
            }
        }
       ngenxx_json_decoder_release(jsonDecoder);
    }
}

void ngenxx_posix_testZip()
{
    static constexpr auto kZBufferSize = 1024z;
    std::ifstream zipIS("../Android/app/src/main/assets/prepare_data.sql", std::ios::in);
    std::ofstream zipOS("./x.zip", std::ios::out);
    auto zipRes = ngenxx_z_cxxstream_zip(NGenXXZipCompressModeDefault, kZBufferSize, NGenXXZFormatZLib, (void *)&zipIS, (void *)&zipOS);
    std::cout << "zip res:" << zipRes << std::endl;

    std::ifstream unzipIS("./x.zip", std::ios::in);
    std::ofstream unzipOS("./x.txt", std::ios::out);
    auto unzipRes = ngenxx_z_cxxstream_unzip(kZBufferSize, NGenXXZFormatZLib, (void *)&unzipIS, (void *)&unzipOS);
    std::cout << "unzip res:" << unzipRes << std::endl;
}

void ngenxx_posix_release()
{
   ngenxx_release();
}