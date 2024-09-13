#include <iostream>
#include <fstream>
#include <string>

#include "../../build.Linux/output/include/NGenXX.h"

int main()
{
    ngenxx_init("/var/www/NGenXX/");

    bool loadSuccess = ngenxx_L_loadF("../Android/app/src/main/assets/biz.lua");
    if (loadSuccess)
    {
        static const char *cParams = "{\"url\":\"https://rinc.xyz\", \"params\":\"\"}";
        const char *cRsp = ngenxx_L_call("lNetHttpRequest", cParams);
        std::cout << cRsp << std::endl;
    }

    ngenxx_release();
}
