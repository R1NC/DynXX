#if (!defined(__ANDROID__) && !defined(__APPLE__) && !defined(__OHOS__) && !defined(_WIN32) && !defined(__EMSCRIPTEN__))

#include "DeviceInfo.hxx"
#include "../../include/NGenXXDeviceInfo.h"

int NGenXX::Device::DeviceInfo::deviceType()
{
    return NGenXXDeviceTypeWindows;
}

std::string NGenXX::Device::DeviceInfo::deviceName()
{
    //TODO
    return "";
}

std::string NGenXX::Device::DeviceInfo::deviceManufacturer()
{
    return "";
}

std::string NGenXX::Device::DeviceInfo::osVersion()
{
    //TODO
    return "";
}

int NGenXX::Device::DeviceInfo::cpuArch()
{
    //TODO
    return NGenXXDeviceCpuArchUnknown;
}

#endif