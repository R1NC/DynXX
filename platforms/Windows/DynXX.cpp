#include "../POSIX/DynXXPOSIX.h"

#pragma comment(lib, "../../build.Windows/output/libs/DynXX-All.lib")
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
