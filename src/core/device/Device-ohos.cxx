#if defined(__OHOS__)
#include "Device.hxx"

#include <deviceinfo.h>

#include <DynXX/C/Device.h>

namespace DynXX::Core::Device {


DynXXDeviceTypeX deviceType()
{
    return DynXXDeviceTypeX::HarmonyOS;
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

DynXXDeviceCpuArchX cpuArch()
{
#if defined(__aarch64__) || defined(_M_ARM64)
    return DynXXDeviceCpuArchX::ARM_64;
#endif
    return DynXXDeviceCpuArchX::ARM;
}

} // namespace DynXX::Core::Device

#endif
