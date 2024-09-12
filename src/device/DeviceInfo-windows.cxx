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
    win::OSInfo *os_info = win::OSInfo::GetInstance();
    win::OSInfo::VersionNumber vn = os_info->version_number();
    win::OSInfo::ServicePack sp = os_info->service_pack();
    char osv[256];
    if (sp.major != 0)
    {
        sprintf(osv, "%d.%d.%d", vn.major, vn.minor, vn.build);
    }
    else
    {
        sprintf(osv, "%d.%d.%d SP%d.%d", vn.major, vn.minor, vn.build, sp.major, sp.minor);
    }
    return std::string(osv);
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