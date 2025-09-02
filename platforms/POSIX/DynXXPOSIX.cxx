#include "DynXXPOSIX.h"

#include "../../include/DynXX/CXX/DynXX.hxx"

#include <iostream>
#include <fstream>
#include <cstring>

static auto cParamsJson = "{\"url\":\"https://rinc.xyz\", \"params\":\"p0=1&p1=2&p2=3\", \"method\":0, \"header_v\":[\"Cache-Control: no-cache\"], \"timeout\":6666}";

void dynxx_posix_init(const char *root)
{
   dynxx_init(root);
}

void dynxx_posix_testHttpL()
{
    if (dynxx_lua_loadF("../Android/app/src/main/assets/biz.lua"))
    {
        const auto cRsp = dynxx_lua_call("lNetHttpRequest", cParamsJson);
        std::cout << cRsp << std::endl;
    }
}

void dynxx_posix_testHttpJ()
{
    if (dynxx_lua_loadF("../Android/app/src/main/assets/DynXX.js") && dynxx_js_loadF("../Android/app/src/main/assets/biz.js", false))
    {
        const char *cRsp = dynxx_js_call("DynXXHttpReq", cParamsJson, false);
        std::cout << cRsp << std::endl;
    }
}

void dynxx_posix_testDB()
{
    if (const auto dbConn = dynxx_sqlite_open("test"))
    {
        const auto insertSQL = "CREATE TABLE IF NOT EXISTS TestTable (_id INTEGER PRIMARY KEY AUTOINCREMENT, platform TEXT, vendor TEXT); \
            INSERT OR IGNORE INTO TestTable (platform, vendor) \
            VALUES \
            ('iOS','Apple'), \
            ('Android','Google'), \
            ('HarmonyOS','Huawei');";
        if (dynxx_sqlite_execute(dbConn, insertSQL))
        {
            const auto querySQL = "SELECT * FROM TestTable;";
            if (const auto queryResult = dynxx_sqlite_query_do(dbConn, querySQL))
            {
                while (dynxx_sqlite_query_read_row(queryResult))
                {
                    const auto platform = dynxx_sqlite_query_read_column_text(queryResult, "platform");
                    const auto vendor = dynxx_sqlite_query_read_column_text(queryResult, "vendor");
                    std::cout << vendor << "->" << platform << std::endl;
                }
               dynxx_sqlite_query_drop(queryResult);
            }
        }
       dynxx_sqlite_close(dbConn);
    }
}

void dynxx_posix_testKV()
{
    if (const auto kvConn = dynxx_kv_open("test"))
    {
       dynxx_kv_write_string(kvConn, "s", "DynXX");
        const auto s = dynxx_kv_read_string(kvConn, "s");
        std::cout << "s->" << s << std::endl;
       dynxx_kv_write_integer(kvConn, "i", 1234567890);
        const auto i = dynxx_kv_read_integer(kvConn, "i");
        std::cout << "i->" << i << std::endl;
       dynxx_kv_write_float(kvConn, "f", 0.123456789f);
        const auto f = dynxx_kv_read_float(kvConn, "f");
        std::cout << "f->" << f << std::endl;
       dynxx_kv_close(kvConn);
    }
}

void dynxx_posix_testCrypto()
{
    const auto inStr = "0123456789qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM<>()[]{}|,;:'`~!@#$%^&*-=_+/";
    const auto inBytes = (unsigned char *) inStr;
    const auto inLen = std::strlen(inStr);
    const auto keyStr = "MNBVCXZLKJHGFDSA";
    const byte *keyBytes = (unsigned char *) keyStr;
    const auto keyLen = std::strlen(keyStr);
    
    size_t aesEncodedLen;
    const auto aesEncodedBytes = dynxx_crypto_aes_encrypt(inBytes, inLen, keyBytes, keyLen, &aesEncodedLen);
    if (aesEncodedBytes && aesEncodedLen > 0)
    {
        size_t aesDecodedLen;
        const byte *aesDecodedBytes = dynxx_crypto_aes_decrypt(aesEncodedBytes, aesEncodedLen, keyBytes, keyLen, &aesDecodedLen);
        if (aesDecodedBytes && aesDecodedLen > 0)
        {
            std::cout << "AES: " << (char *)aesDecodedBytes << std::endl;
        }
    }
    
    size_t ivLen = 12;
    const auto ivBytes = dynxx_crypto_rand(ivLen);
    size_t aesgcmTagBits = 15 * 8;
    
    size_t aesgcmEncodedLen;
    const auto aesgcmEncodedBytes = dynxx_crypto_aes_gcm_encrypt(inBytes, inLen, keyBytes, keyLen, ivBytes, ivLen, nullptr, 0, aesgcmTagBits, &aesgcmEncodedLen);
    if (aesgcmEncodedBytes && aesgcmEncodedLen > 0)
    {
        size_t aesgcmDecodedLen;
        const auto aesgcmDecodedBytes = dynxx_crypto_aes_gcm_decrypt(aesgcmEncodedBytes, aesgcmEncodedLen, keyBytes, keyLen, ivBytes, ivLen, nullptr, 0, aesgcmTagBits, &aesgcmDecodedLen);
        if (aesgcmDecodedBytes && aesgcmDecodedLen > 0)
        {
            std::cout << "AES-GCM: " << (char *)aesgcmDecodedBytes << std::endl;
        }
    }
}

void dynxx_posix_testJsonDecoder()
{
    if (const auto jsonDecoder = dynxx_json_decoder_init(cParamsJson))
    {
        if (const auto urlNode = dynxx_json_decoder_read_node(jsonDecoder, nullptr, "url"))
        {
            const auto url = dynxx_json_decoder_read_string(jsonDecoder, urlNode);
            std::cout << "url:" << url << std::endl;
        }
        if (const auto methodNode = dynxx_json_decoder_read_node(jsonDecoder, nullptr, "method"))
        {
            const auto method = dynxx_json_decoder_read_integer(jsonDecoder, methodNode);
            std::cout << "method:" << method << std::endl;
        }
        if (const auto headerVNode = dynxx_json_decoder_read_node(jsonDecoder, nullptr, "header_v"))
        {
            auto headerNode = dynxx_json_decoder_read_child(jsonDecoder, headerVNode);
            while (headerNode)
            {
                const auto header = dynxx_json_decoder_read_string(jsonDecoder, headerNode);
                std::cout << "header:" << header << std::endl;
                headerNode = dynxx_json_decoder_read_next(jsonDecoder, headerNode);
            }
        }
       dynxx_json_decoder_release(jsonDecoder);
    }
}

void dynxx_posix_testZip()
{
    static constexpr auto kZBufferSize = 1024z;
    static const auto format = DynXXZFormatX::GZip;
    static const auto mode = DynXXZipCompressModeX::Default;
    std::ifstream zipIS("../Android/app/src/main/assets/prepare_data.sql", std::ios::in);
    std::ofstream zipOS("./x.zip", std::ios::out);
    auto zipRes = dynxxZCxxStreamZip(mode, kZBufferSize, format, &zipIS, &zipOS);
    std::cout << "zip res:" << zipRes << std::endl;

    std::ifstream unzipIS("./x.zip", std::ios::in);
    std::ofstream unzipOS("./x.txt", std::ios::out);
    auto unzipRes = dynxxZCxxStreamUnzip(kZBufferSize, format, &unzipIS, &unzipOS);
    std::cout << "unzip res:" << unzipRes << std::endl;
}

void dynxx_posix_release()
{
   dynxx_release();
}