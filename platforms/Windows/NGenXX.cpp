#include <iostream>
#include <fstream>
#include <string>
#include "../../build.Windows/output/include/NGenXX.h"

#pragma comment(lib, "../../build.Windows/output/Release/lua.lib")
#pragma comment(lib, "../../build.Windows/output/Release/cjson.lib")
#pragma comment(lib, "../../build.Windows/output/Release/wolfssl.lib")
#pragma comment(lib, "../../build.Windows/output/Release/libcurl.lib")
#pragma comment(lib, "../../build.Windows/output/Release/NGenXX.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "Wldap32.lib")

int main()
{
    void *handle = ngenxx_init(true);

    bool loadSuccess = ngenxx_L_loadF(handle, "../android/app/src/main/assets/biz.lua");
    if (loadSuccess)
    {
        static const char *cParams = "{\"url\":\"https://rinc.xyz\", \"params\":\"\"}";
        const char *cRsp = ngenxx_L_call(handle, "lNetHttpReq", cParams);
        std::cout << cRsp << std::endl;
    }

    ngenxx_release(handle);
}
