#if defined(__OHOS__)
#include "Device.hxx"

#include <deviceinfo.h>

#include <DynXX/C/Device.h>

namespace DynXX::Core::Device {


int deviceType()
{
    return DynXXDeviceTypeHarmonyOS;
}

std::string deviceName()
{
    return OH_GetHardwareModel();
}

std::string deviceManufacturer()
{
    return OH_GetManufacture();
}

std::string deviceModel()
{
    return OH_GetHardwareModel();
}

std::string osVersion()
{
    return OH_GetDistributionOSVersion();
}

int cpuArch()
{
#if defined(__aarch64__) || defined(_M_ARM64)
    return DynXXDeviceCpuArchARM_64;
#endif
    return DynXXDeviceCpuArchARM;
}

} // namespace DynXX::Core::Device

#endif