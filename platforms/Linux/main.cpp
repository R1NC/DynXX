#include "../POSIX/NGenXXPOSIX.h"

int main()
{
    ngenxx_posix_init("/var/www/");

    ngenxx_posix_testHttpJ();

    ngenxx_posix_testDB();

    ngenxx_posix_testKV();

    ngenxx_posix_testJsonDecoder();

    ngenxx_posix_testCrypto();

    ngenxx_posix_testZip();

    ngenxx_posix_release();
}
