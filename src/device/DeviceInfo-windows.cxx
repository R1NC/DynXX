#if (defined(_WIN32) || defined(_WIN64))

#include "DeviceInfo.hxx"
#include "../../include/NGenXXDeviceInfo.h"

#include <windows.h>
#include <stddef.h>
#include <stdint.h>

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
    //TODO
    return "";
}

std::string NGenXX::Device::DeviceInfo::osVersion()
{
    //TODO
    return "";
}

int NGenXX::Device::DeviceInfo::cpuArch()
{
    win::OSInfo::WindowsArchitecture arch = win::OSInfo::GetArchitecture();
    switch (arch)
    {
    case win::OSInfo::X86_ARCHITECTURE:
        return NGenXXDeviceCpuArchX86;
    case win::OSInfo::X64_ARCHITECTURE:
        return NGenXXDeviceCpuArchX86_64;
    case win::OSInfo::IA64_ARCHITECTURE:
        return NGenXXDeviceCpuArchIA64;
    case win::OSInfo::ARM64_ARCHITECTURE:
        return NGenXXDeviceCpuArchARM_64;
    default:
        return NGenXXDeviceCpuArchUnknown;
    }
}

#endif