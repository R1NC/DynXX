#include "../POSIX/NGenXXPOSIX.h"

#pragma comment(lib, "../../build.Windows/output/libs/lua.lib")
#pragma comment(lib, "../../build.Windows/output/libs/qjs.lib")
#pragma comment(lib, "../../build.Windows/output/libs/cjson.lib")
//#pragma comment(lib, "../../build.Windows/output/libs/libssl.lib")
//#pragma comment(lib, "../../build.Windows/output/libs/libcrypto.lib")
#pragma comment(lib, "../../build.Windows/output/libs/libcurl.lib")
#pragma comment(lib, "../../build.Windows/output/libs/libuv.lib")
#pragma comment(lib, "../../build.Windows/output/libs/sqlite3.lib")
#pragma comment(lib, "../../build.Windows/output/libs/mmkvcore.lib")
#pragma comment(lib, "../../build.Windows/output/libs/mmkv.lib")
#pragma comment(lib, "../../build.Windows/output/libs/spdlog.lib")
#pragma comment(lib, "../../build.Windows/output/libs/NGenXX.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "Wldap32.lib")

int main()
{
   ngenxx_posix_init("D://");

   ngenxx_posix_testHttpL();

   ngenxx_posix_testDB();

   ngenxx_posix_testKV();

   ngenxx_posix_release();
}
