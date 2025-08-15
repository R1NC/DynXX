#include "../POSIX/DynXXPOSIX.h"

int main()
{
   dynxx_posix_init("/var/www/");

   dynxx_posix_testHttpJ();

   dynxx_posix_testDB();

   dynxx_posix_testKV();

   dynxx_posix_testJsonDecoder();

   dynxx_posix_testCrypto();

   dynxx_posix_testZip();

   dynxx_posix_release();
}
