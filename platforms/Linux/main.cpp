#include <iostream>
#include <fstream>
#include <string>

#include "../../build.Linux/output/include/NGenXX.h"

int main()
{
    void* lstate = ngenxx_L_create();
    int ret = ngenxx_L_loadF(lstate, "../Android/app/src/main/assets/biz.lua");
    if (ret == 0) {
        static const char* cParams = "{\"url\":\"https://rinc.xyz\", \"params\":\"\"}";
        const char* cRsp = ngenxx_L_call(lstate, "lNetHttpReq", cParams);
        std::cout << cRsp << std::endl;
    }
    ngenxx_L_destroy(lstate);
}
