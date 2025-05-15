#if defined(__ANDROID__)
#include "DeviceInfo.hxx"

#include <cstddef>
#include <cstdint>

#include <sys/system_properties.h>
#include <android/api-level.h>

#include <NGenXXDeviceInfo.h>

namespace
{
    constexpr auto SYS_PROPERTY_VERSION_RELEASE = "ro.build.version.release";
    constexpr auto SYS_PROPERTY_VERSION_INCREMENTAL = "ro.build.version.incremental";
    constexpr auto SYS_PROPERTY_VERSION_CODENAME = "ro.build.version.codename";
    constexpr auto SYS_PROPERTY_VERSION_SDK = "ro.build.version.sdk";

    constexpr auto SYS_PROPERTY_MODEL = "ro.product.model";
    constexpr auto SYS_PROPERTY_MANUFACTURER = "ro.product.manufacturer";
    constexpr auto SYS_PROPERTY_BOARD = "ro.product.board";
    constexpr auto SYS_PROPERTY_BRAND = "ro.product.brand";
    constexpr auto SYS_PROPERTY_DEVICE = "ro.product.device";
    constexpr auto SYS_PROPERTY_PRODUCT = "ro.product.name";

    constexpr auto SYS_PROPERTY_HARDWARE = "ro.hardware";

    constexpr auto SYS_PROPERTY_CPU_ABI = "ro.product.cpu.abi";
    constexpr auto SYS_PROPERTY_CPU_ABI2 = "ro.product.cpu.abi2";

    constexpr auto SYS_PROPERTY_DISPLAY = "ro.build.display.id";
    constexpr auto SYS_PROPERTY_HOST = "ro.build.host";
    constexpr auto SYS_PROPERTY_USER = "ro.build.user";
    constexpr auto SYS_PROPERTY_ID = "ro.build.id";
    constexpr auto SYS_PROPERTY_TYPE = "ro.build.type";
    constexpr auto SYS_PROPERTY_TAGS = "ro.build.tags";
    constexpr auto SYS_PROPERTY_FINGERPRINT = "ro.build.fingerprint";

    std::string sysProperty(const std::string &k)
    {
        char v[PROP_VALUE_MAX];
        __system_property_get(k.c_str(), v);
        return v[0] ? v : "";
    }

    int apiLevel()
    {
        return android_get_device_api_level();
    }
}

int NGenXX::DeviceInfo::deviceType()
{
    return NGenXXDeviceTypeAndroid;
}

std::string NGenXX::DeviceInfo::deviceName()
{
    return sysProperty(SYS_PROPERTY_MODEL);
}

std::string NGenXX::DeviceInfo::deviceManufacturer()
{
    return sysProperty(SYS_PROPERTY_MANUFACTURER);
}

std::string NGenXX::DeviceInfo::deviceModel()
{
    return sysProperty(SYS_PROPERTY_MODEL);
}

std::string NGenXX::DeviceInfo::osVersion()
{
    return sysProperty(SYS_PROPERTY_VERSION_RELEASE);
}

int NGenXX::DeviceInfo::cpuArch()
{
#if defined(__aarch64__) || defined(_M_ARM64)
    return NGenXXDeviceCpuArchARM_64;
#endif
    return NGenXXDeviceCpuArchARM;
}

#endif