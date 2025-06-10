#if (defined(_WIN32) || defined(_WIN64))
#include "Device.hxx"

#include <cstddef>
#include <cstdint>

#include <windows.h>

#include <NGenXXDevice.h>

int NGenXX::Core::Device::deviceType()
{
    return NGenXXDeviceTypeWindows;
}

std::string NGenXX::Core::Device::deviceName()
{
    //TODO
    return {};
}

std::string NGenXX::Core::Device::deviceManufacturer()
{
    return "Microsoft";
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