#if (defined(_WIN32) || defined(_WIN64))
#include "Device.hxx"

#include <cstddef>
#include <cstdint>

#include <windows.h>

#include <DynXX/C/Device.h>

int DynXX::Core::Device::deviceType()
{
    return DynXXDeviceTypeWindows;
}

std::string DynXX::Core::Device::deviceName()
{
    //TODO
    return {};
}

std::string DynXX::Core::Device::deviceManufacturer()
{
    return "Microsoft";
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