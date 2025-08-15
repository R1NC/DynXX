#if defined(__OHOS__)
#include "Device.hxx"

#include <deviceinfo.h>

#include <DynXX/C/Device.h>

int DynXX::Core::Device::deviceType()
{
    return DynXXDeviceTypeHarmonyOS;
}

std::string DynXX::Core::Device::deviceName()
{
    return OH_GetHardwareModel();
}

std::string DynXX::Core::Device::deviceManufacturer()
{
    return OH_GetManufacture();
}

std::string DynXX::Core::Device::deviceModel()
{
    return OH_GetHardwareModel();
}

std::string DynXX::Core::Device::osVersion()
{
    return OH_GetDistributionOSVersion();
}

int DynXX::Core::Device::cpuArch()
{
#if defined(__aarch64__) || defined(_M_ARM64)
    return DynXXDeviceCpuArchARM_64;
#endif
    return DynXXDeviceCpuArchARM;
}

#endif