#include "../POSIX/NGenXXPOSIX.h"

#pragma comment(lib, "../../build.Windows/output/Release/lua.lib")
#pragma comment(lib, "../../build.Windows/output/Release/cjson.lib")
#pragma comment(lib, "../../build.Windows/output/Release/wolfssl.lib")
#pragma comment(lib, "../../build.Windows/output/Release/libcurl.lib")
#pragma comment(lib, "../../build.Windows/output/Release/sqlite3.lib")
#pragma comment(lib, "../../build.Windows/output/Release/mmkvcore.lib")
#pragma comment(lib, "../../build.Windows/output/Release/mmkv.lib")
#pragma comment(lib, "../../build.Windows/output/Release/NGenXX.lib")
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
