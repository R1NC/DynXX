#include "../POSIX/DynXXPOSIX.h"

#pragma comment(lib, "../../build.Windows/output/libs/lua.lib")
#pragma comment(lib, "../../build.Windows/output/libs/qjs.lib")
#pragma comment(lib, "../../build.Windows/output/libs/cjson.lib")
#pragma comment(lib, "../../build.Windows/output/libs/libssl-x64.lib")
#pragma comment(lib, "../../build.Windows/output/libs/libcrypto-x64.lib")
#pragma comment(lib, "../../build.Windows/output/libs/libcurl.lib")
#pragma comment(lib, "../../build.Windows/output/libs/libuv.lib")
#pragma comment(lib, "../../build.Windows/output/libs/zlibstatic.lib")
#pragma comment(lib, "../../build.Windows/output/libs/sqlite3.lib")
#pragma comment(lib, "../../build.Windows/output/libs/mmkvcore.lib")
#pragma comment(lib, "../../build.Windows/output/libs/mmkv.lib")
#pragma comment(lib, "../../build.Windows/output/libs/spdlog.lib")
#pragma comment(lib, "../../build.Windows/output/libs/DynXX.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "Wldap32.lib")

int main()
{
   dynxx_posix_init("D://");

   dynxx_posix_testHttpL();

   dynxx_posix_testDB();

   dynxx_posix_testKV();

   dynxx_posix_release();
}
