#if defined(__EMSCRIPTEN__)
#include "Device.hxx"

#include <DynXX/C/Device.h>

int DynXX::Core::Device::deviceType()
{
    return DynXXDeviceTypeWeb;
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