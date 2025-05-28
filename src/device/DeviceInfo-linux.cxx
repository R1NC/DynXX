#if (!defined(__ANDROID__) && !defined(__APPLE__) && !defined(__OHOS__) && !defined(_WIN32) && !defined(__EMSCRIPTEN__))
#include "DeviceInfo.hxx"

#include <NGenXXDeviceInfo.h>

int NGenXX::DeviceInfo::deviceType()
{
    return NGenXXDeviceTypeLinux;
}

std::string NGenXX::DeviceInfo::deviceName()
{
    //TODO
    return {};
}

std::string NGenXX::DeviceInfo::deviceManufacturer()
{
    //TODO
    return {};
}

std::string NGenXX::DeviceInfo::deviceModel()
{
    //TODO
    return {};
}

std::string NGenXX::DeviceInfo::osVersion()
{
    //TODO
    return {};
}

int NGenXX::DeviceInfo::cpuArch()
{
    //TODO
    return NGenXXDeviceCpuArchUnknown;
}

#endif