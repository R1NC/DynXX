#include <iostream>
#include <fstream>
#include <string>

#include "../../build.Linux/output/include/EngineXX.h"

int main()
{
    void* lstate = enginexx_L_create();
    int ret = enginexx_L_loadF(lstate, "../Android/app/src/main/assets/biz.lua");
    if (ret == 0) {
        static const char* cParams = "{\"url\":\"https://rinc.xyz\", \"params\":\"\"}";
        const char* cRsp = enginexx_L_call(lstate, "lNetHttpReq", cParams);
        std::cout << cRsp << std::endl;
    }
    enginexx_L_destroy(lstate);
}
