#ifdef __OHOS__

#include "DeviceInfo.hxx"
#include "../../include/NGenXXDeviceInfo.h"

int NGenXX::Device::DeviceInfo::deviceType()
{
    return NGenXXDeviceTypeHarmonyOS;
}

std::string NGenXX::Device::DeviceInfo::deviceName()
{
    //TODO
}

std::string NGenXX::Device::DeviceInfo::osVersion()
{
    //TODO
}

int NGenXX::Device::DeviceInfo::cpuArch()
{
#if defined(__aarch64__) || defined(_M_ARM64)
    return NGenXXDeviceCpuArchARM_64;
#endif
    return NGenXXDeviceCpuArchARM;
}

#endif