#if defined(__OHOS__)
#include "DeviceInfo.hxx"

#include <deviceinfo.h>

#include <NGenXXDeviceInfo.h>

int NGenXX::DeviceInfo::deviceType()
{
    return NGenXXDeviceTypeHarmonyOS;
}

std::string NGenXX::DeviceInfo::deviceName()
{
    return OH_GetHardwareModel();
}

std::string NGenXX::DeviceInfo::deviceManufacturer()
{
    return OH_GetManufacture();
}

std::string NGenXX::DeviceInfo::deviceModel()
{
    return OH_GetHardwareModel();
}

std::string NGenXX::DeviceInfo::osVersion()
{
    return OH_GetDistributionOSVersion();
}

int NGenXX::DeviceInfo::cpuArch()
{
#if defined(__aarch64__) || defined(_M_ARM64)
    return NGenXXDeviceCpuArchARM_64;
#endif
    return NGenXXDeviceCpuArchARM;
}

#endif