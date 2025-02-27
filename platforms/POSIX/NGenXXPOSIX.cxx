#include "NGenXXPOSIX.h"
#include "../../include/NGenXX.h"

#include <iostream>
#include <fstream>
#include <cstring>
#include <functional>

static const char *cParamsJson = "{\"url\":\"https://rinc.xyz\", \"params\":\"p0=1&p1=2&p2=3\", \"method\":0, \"header_v\":[\"Cache-Control: no-cache\"], \"timeout\":6666}";

void ngenxx_posix_init(const char *root)
{
   ngenxx_init(root);
}

void ngenxx_posix_testHttpL(void)
{
    bool loadSuccess = ngenxx_lua_loadF("../Android/app/src/main/assets/biz.lua");
    if (loadSuccess)
    {
        const char *cRsp = ngenxx_lua_call("lNetHttpRequest", cParamsJson);
        std::cout << cRsp << std::endl;
    }
}

void ngenxx_posix_testHttpJ(void)
{
    if (ngenxx_lua_loadF("../Android/app/src/main/assets/NGenXX.js") && ngenxx_js_loadF("../Android/app/src/main/assets/biz.js"))
    {
        const char *cRsp = ngenxx_js_call("NGenXXHttpReq", cParamsJson);
        std::cout << cRsp << std::endl;
    }
}

void ngenxx_posix_testDB(void)
{
    void *dbConn = ngenxx_store_sqlite_open("test");
    if (dbConn)
    {
        const char *insertSQL = "CREATE TABLE IF NOT EXISTS TestTable (_id INTEGER PRIMARY KEY AUTOINCREMENT, platform TEXT, vendor TEXT); \
            INSERT OR IGNORE INTO TestTable (platform, vendor) \
            VALUES \
            ('iOS','Apple'), \
            ('Android','Google'), \
            ('HarmonyOS','Huawei');";
        if (ngenxx_store_sqlite_execute(dbConn, insertSQL))
        {
            const char *querySQL = "SELECT * FROM TestTable;";
            void *queryResult = ngenxx_store_sqlite_query_do(dbConn, querySQL);
            if (queryResult)
            {
                while (ngenxx_store_sqlite_query_read_row(queryResult))
                {
                    const char *platform = ngenxx_store_sqlite_query_read_column_text(queryResult, "platform");
                    const char *vendor = ngenxx_store_sqlite_query_read_column_text(queryResult, "vendor");
                    std::cout << vendor << "->" << platform << std::endl;
                }
               ngenxx_store_sqlite_query_drop(queryResult);
            }
        }
       ngenxx_store_sqlite_close(dbConn);
    }
}

void ngenxx_posix_testKV(void)
{
    void *kvConn = ngenxx_store_kv_open("test");
    if (kvConn)
    {
       ngenxx_store_kv_write_string(kvConn, "s", "NGenXX");
        const char *s = ngenxx_store_kv_read_string(kvConn, "s");
        std::cout << "s->" << s << std::endl;
       ngenxx_store_kv_write_integer(kvConn, "i", 1234567890);
        long i = ngenxx_store_kv_read_integer(kvConn, "i");
        std::cout << "i->" << i << std::endl;
       ngenxx_store_kv_write_integer(kvConn, "f", 0.123456789);
        double f = ngenxx_store_kv_read_float(kvConn, "f");
        std::cout << "f->" << f << std::endl;
       ngenxx_store_kv_close(kvConn);
    }
}

void ngenxx_posix_testCrypto(void)
{
    const char *inStr = "0123456789qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM<>()[]{}|,;:'`~!@#$%^&*-=_+/";
    byte *inBytes = (unsigned char *) inStr;
    size inLen = std::strlen(inStr);
    const char *keyStr = "MNBVCXZLKJHGFDSA";
    byte *keyBytes = (unsigned char *) keyStr;
    size keyLen = std::strlen(keyStr);
    
    size aesEncodedLen;
    const byte *aesEncodedBytes = ngenxx_crypto_aes_encrypt(inBytes, inLen, keyBytes, keyLen, &aesEncodedLen);
    if (aesEncodedBytes && aesEncodedLen > 0) {
        size aesDecodedLen;
        const byte *aesDecodedBytes = ngenxx_crypto_aes_decrypt(aesEncodedBytes, aesEncodedLen, keyBytes, keyLen, &aesDecodedLen);
        if (aesDecodedBytes && aesDecodedLen > 0) {
            std::cout << "AES: " << (char *)aesDecodedBytes << std::endl;
        }
    }
    
    size ivLen = 12;
    byte ivBytes[ivLen];
   ngenxx_crypto_rand(ivLen, ivBytes);
    size aesgcmTagBits = 15 * 8;
    
    size aesgcmEncodedLen;
    const byte *aesgcmEncodedBytes = ngenxx_crypto_aes_gcm_encrypt(inBytes, inLen, keyBytes, keyLen, ivBytes, ivLen, NULL, 0, aesgcmTagBits, &aesgcmEncodedLen);
    if (aesgcmEncodedBytes && aesgcmEncodedLen > 0) {
        size aesgcmDecodedLen;
        const byte *aesgcmDecodedBytes = ngenxx_crypto_aes_gcm_decrypt(aesgcmEncodedBytes, aesgcmEncodedLen, keyBytes, keyLen, ivBytes, ivLen, NULL, 0, aesgcmTagBits, &aesgcmDecodedLen);
        if (aesgcmDecodedBytes && aesgcmDecodedLen > 0) {
            std::cout << "AES-GCM: " << (char *)aesgcmDecodedBytes << std::endl;
        }
    }
}

void ngenxx_posix_testJsonDecoder(void)
{
    void *jsonDecoder = ngenxx_json_decoder_init(cParamsJson);
    if (jsonDecoder) {
        void *urlNode = ngenxx_json_decoder_read_node(jsonDecoder, NULL, "url");
        if (urlNode) {
            const char *url = ngenxx_json_decoder_read_string(jsonDecoder, urlNode);
            std::cout << "url:" << url << std::endl;
        }
        void *methodNode = ngenxx_json_decoder_read_node(jsonDecoder, NULL, "method");
        if (methodNode) {
            double method = ngenxx_json_decoder_read_number(jsonDecoder, methodNode);
            std::cout << "method:" << method << std::endl;
        }
        void *headerVNode = ngenxx_json_decoder_read_node(jsonDecoder, NULL, "header_v");
        if (headerVNode) {
            void *headerNode = ngenxx_json_decoder_read_child(jsonDecoder, headerVNode);
            while (headerNode) {
                const char *header = ngenxx_json_decoder_read_string(jsonDecoder, headerNode);
                std::cout << "header:" << header << std::endl;
                headerNode = ngenxx_json_decoder_read_next(jsonDecoder, headerNode);
            }
        }
       ngenxx_json_decoder_release(jsonDecoder);
    }
}

void ngenxx_posix_testZip(void)
{
    static const size kZBufferSize = 1024;
    std::ifstream zipIS("../Android/app/src/main/assets/prepare_data.sql", std::ios::in);
    std::ofstream zipOS("./x.zip", std::ios::out);
    bool zipRes = ngenxx_z_cxxstream_zip(NGenXXZipCompressModeDefault, kZBufferSize, (void *)&zipIS, (void *)&zipOS);
    std::cout << "zip res:" << zipRes << std::endl;

    std::ifstream unzipIS("./x.zip", std::ios::in);
    std::ofstream unzipOS("./x.txt", std::ios::out);
    bool unzipRes = ngenxx_z_cxxstream_unzip(kZBufferSize, (void *)&unzipIS, (void *)&unzipOS);
    std::cout << "unzip res:" << unzipRes << std::endl;
}

void ngenxx_posix_release()
{
   ngenxx_release();
}