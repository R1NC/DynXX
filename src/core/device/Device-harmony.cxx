#if defined(__OHOS__)
#include "Device.hxx"

#include <deviceinfo.h>

#include <NGenXXDevice.h>

int NGenXX::Core::Device::deviceType()
{
    return NGenXXDeviceTypeHarmonyOS;
}

std::string NGenXX::Core::Device::deviceName()
{
    return OH_GetHardwareModel();
}

std::string NGenXX::Core::Device::deviceManufacturer()
{
    return OH_GetManufacture();
}

std::string NGenXX::Core::Device::deviceModel()
{
    return OH_GetHardwareModel();
}

std::string NGenXX::Core::Device::osVersion()
{
    return OH_GetDistributionOSVersion();
}

int NGenXX::Core::Device::cpuArch()
{
#if defined(__aarch64__) || defined(_M_ARM64)
    return NGenXXDeviceCpuArchARM_64;
#endif
    return NGenXXDeviceCpuArchARM;
}

#endif