#include "../POSIX/NGenXXPOSIX.h"

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
    ngenxx_posix_init("D://");

    ngenxx_posix_testHttpL();

    ngenxx_posix_testDB();

    ngenxx_posix_testKV();

    ngenxx_posix_release();
}
