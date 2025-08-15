#if (!defined(__ANDROID__) && !defined(__APPLE__) && !defined(__OHOS__) && !defined(_WIN32) && !defined(__EMSCRIPTEN__))
#include "Device.hxx"

#include <DynXX/C/Device.h>

int DynXX::Core::Device::deviceType()
{
    return DynXXDeviceTypeLinux;
}

std::string DynXX::Core::Device::deviceName()
{
    //TODO
    return {};
}

std::string DynXX::Core::Device::deviceManufacturer()
{
    //TODO
    return {};
}

std::string DynXX::Core::Device::deviceModel()
{
    //TODO
    return {};
}

std::string DynXX::Core::Device::osVersion()
{
    //TODO
    return {};
}

int DynXX::Core::Device::cpuArch()
{
    //TODO
    return DynXXDeviceCpuArchUnknown;
}

#endif