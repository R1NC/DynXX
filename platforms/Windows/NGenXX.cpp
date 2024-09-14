#include <iostream>
#include <fstream>
#include <string>
#include "../../build.Windows/output/include/NGenXX.h"

#pragma comment(lib, "../../build.Windows/output/Release/lua.lib")
#pragma comment(lib, "../../build.Windows/output/Release/cjson.lib")
#pragma comment(lib, "../../build.Windows/output/Release/wolfssl.lib")
#pragma comment(lib, "../../build.Windows/output/Release/libcurl.lib")
#pragma comment(lib, "../../build.Windows/output/Release/libsqlite3.lib")
#pragma comment(lib, "../../build.Windows/output/Release/libmmkvcore.lib")
#pragma comment(lib, "../../build.Windows/output/Release/libmmkv.lib")
#pragma comment(lib, "../../build.Windows/output/Release/NGenXX.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "Wldap32.lib")

int main()
{
    ngenxx_init("D://");

    bool loadSuccess = ngenxx_L_loadF("../Android/app/src/main/assets/biz.lua");
    if (loadSuccess)
    {
        static const char *cParams = "{\"url\":\"https://rinc.xyz\", \"params\":\"p0=1&p1=2&p2=3\", \"method\":0, \"headers_v\":[\"Cache-Control: no-cache\"], \"headers_c\": 1, \"timeout\":6666}";
        const char *cRsp = ngenxx_L_call("lNetHttpRequest", cParams);
        std::cout << cRsp << std::endl;

        void *dbConn = ngenxx_store_sqlite_open("test");
        if (dbConn) {
            const char *insertSQL = "";
            if (ngenxx_store_sqlite_execute(dbConn, insertSQL)) {
                const char *querySQL = "";
                void *queryResult = ngenxx_store_sqlite_query_do(dbConn, querySQL);
                if (queryResult) {
                    while(ngenxx_store_sqlite_query_read_row(queryResult)) {
                        const char *platform = ngenxx_store_sqlite_query_read_column_text(queryResult, "platform");
                        const char *vendor = ngenxx_store_sqlite_query_read_column_text(queryResult, "vendor");
                        std::cout << vendor << "->" << platform << std::endl;
                    }
                    ngenxx_store_sqlite_query_drop(queryResult);
                }
            }
            ngenxx_store_sqlite_close(dbConn);
        }

        void *kvConn = ngenxx_store_kv_open("test");
        if (kvConn) {
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

    ngenxx_release();
}
