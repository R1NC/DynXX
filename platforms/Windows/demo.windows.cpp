#include <iostream>
#include <fstream>
#include <string>
#include "../../build.Windows/output/include/EngineXX.h"

#pragma comment(lib, "../../build.Windows/output/Release/lua.lib")
#pragma comment(lib, "../../build.Windows/output/Release/cjson.lib")
#pragma comment(lib, "../../build.Windows/output/Release/wolfssl.lib")
#pragma comment(lib, "../../build.Windows/output/Release/libcurl.lib")
#pragma comment(lib, "../../build.Windows/output/Release/EngineXX.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "Wldap32.lib")

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

