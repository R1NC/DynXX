#ifdef __ANDROID__

#include "DeviceInfo.hxx"
#include "../../include/NGenXXDeviceInfo.h"

#include <stddef.h>
#include <stdint.h>
#include <sys/system_properties.h>

int NGenXX::Device::DeviceInfo::deviceType()
{
    return NGenXXDeviceTypeAndroid;
}

std::string NGenXX::Device::DeviceInfo::deviceName()
{
    char model[PROP_VALUE_MAX];
    __system_property_get("ro.product.model", model);
    return std::string(model);
}

std::string NGenXX::Device::DeviceInfo::deviceManufacturer()
{
    char manufacturer[PROP_VALUE_MAX];
    __system_property_get("ro.product.manufacturer", manufacturer);
    return std::string(manufacturer);
}

std::string NGenXX::Device::DeviceInfo::osVersion()
{
    char osv[PROP_VALUE_MAX];
    __system_property_get("ro.build.version.release", osv);
    if (osv[0])
    {
        return std::string(osv);
    }
    return "1.0.0";
}

int NGenXX::Device::DeviceInfo::cpuArch()
{
#if defined(__aarch64__) || defined(_M_ARM64)
    return NGenXXDeviceCpuArchARM_64;
#endif
    return NGenXXDeviceCpuArchARM;
}

#endif