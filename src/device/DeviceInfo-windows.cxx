#if (defined(_WIN32) || defined(_WIN64))
#include "DeviceInfo.hxx"

#include <cstddef>
#include <cstdint>

#include <windows.h>

#include <NGenXXDeviceInfo.h>

int NGenXX::DeviceInfo::deviceType()
{
    return NGenXXDeviceTypeWindows;
}

std::string NGenXX::DeviceInfo::deviceName()
{
    //TODO
    return {};
}

std::string NGenXX::DeviceInfo::deviceManufacturer()
{
    return "Microsoft";
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