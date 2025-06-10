#if (!defined(__ANDROID__) && !defined(__APPLE__) && !defined(__OHOS__) && !defined(_WIN32) && !defined(__EMSCRIPTEN__))
#include "Device.hxx"

#include <NGenXXDevice.h>

int NGenXX::Core::Device::deviceType()
{
    return NGenXXDeviceTypeLinux;
}

std::string NGenXX::Core::Device::deviceName()
{
    //TODO
    return {};
}

std::string NGenXX::Core::Device::deviceManufacturer()
{
    //TODO
    return {};
}

std::string NGenXX::Core::Device::deviceModel()
{
    //TODO
    return {};
}

std::string NGenXX::Core::Device::osVersion()
{
    //TODO
    return {};
}

int NGenXX::Core::Device::cpuArch()
{
    //TODO
    return NGenXXDeviceCpuArchUnknown;
}

#endif