#if defined(__EMSCRIPTEN__)
#include "Device.hxx"

#include <NGenXXDevice.h>

int NGenXX::Core::Device::deviceType()
{
    return NGenXXDeviceTypeWeb;
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