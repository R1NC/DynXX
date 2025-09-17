#if (!defined(__ANDROID__) && !defined(__APPLE__) && !defined(__OHOS__) && !defined(_WIN32) && !defined(__EMSCRIPTEN__))
#include "Device.hxx"

#include <DynXX/C/Device.h>

namespace DynXX::Core::Device {

int deviceType()
{
    return DynXXDeviceTypeLinux;
}

std::string deviceName()
{
    //TODO
    return {};
}

std::string deviceManufacturer()
{
    //TODO
    return {};
}

std::string deviceModel()
{
    //TODO
    return {};
}

std::string osVersion()
{
    //TODO
    return {};
}

int cpuArch()
{
    //TODO
    return DynXXDeviceCpuArchUnknown;
}

} // namespace DynXX::Core::Device

#endif