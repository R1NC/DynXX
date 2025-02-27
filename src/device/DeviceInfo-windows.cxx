#if (defined(_WIN32) || defined(_WIN64))
#include "DeviceInfo.hxx"

#include <cstddef>
#include <cstdint>

#include <windows.h>

#include <NGenXXDeviceInfo.h>

int NGenXX::Device::DeviceInfo::deviceType()
{
    return NGenXXDeviceTypeWindows;
}

std::string NGenXX::Device::DeviceInfo::deviceName()
{
    //TODO
    return {};
}

std::string NGenXX::Device::DeviceInfo::deviceManufacturer()
{
    return "Microsoft";
}

std::string NGenXX::Device::DeviceInfo::osVersion()
{
    //TODO
    return {};
}

int NGenXX::Device::DeviceInfo::cpuArch()
{
    //TODO
    return NGenXXDeviceCpuArchUnknown;
}

#endif