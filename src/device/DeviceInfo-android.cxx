#ifdef __ANDROID__

#include "DeviceInfo.hxx"
#include "../../include/NGenXXDeviceInfo.h"

#include <stddef.h>
#include <stdint.h>
#include <sys/system_properties.h>

char const *ANDROID_OS_BUILD_VERSION_RELEASE = "ro.build.version.release";
char const *ANDROID_OS_BUILD_VERSION_INCREMENTAL = "ro.build.version.incremental";
char const *ANDROID_OS_BUILD_VERSION_CODENAME = "ro.build.version.codename";
char const *ANDROID_OS_BUILD_VERSION_SDK = "ro.build.version.sdk";

char const *ANDROID_OS_BUILD_MODEL = "ro.product.model";
char const *ANDROID_OS_BUILD_MANUFACTURER = "ro.product.manufacturer";
char const *ANDROID_OS_BUILD_BOARD = "ro.product.board";
char const *ANDROID_OS_BUILD_BRAND = "ro.product.brand";
char const *ANDROID_OS_BUILD_DEVICE = "ro.product.device";
char const *ANDROID_OS_BUILD_PRODUCT = "ro.product.name";

char const *ANDROID_OS_BUILD_HARDWARE = "ro.hardware";

char const *ANDROID_OS_BUILD_CPU_ABI = "ro.product.cpu.abi";
char const *ANDROID_OS_BUILD_CPU_ABI2 = "ro.product.cpu.abi2";

char const *ANDROID_OS_BUILD_DISPLAY = "ro.build.display.id";
char const *ANDROID_OS_BUILD_HOST = "ro.build.host";
char const *ANDROID_OS_BUILD_USER = "ro.build.user";
char const *ANDROID_OS_BUILD_ID = "ro.build.id";
char const *ANDROID_OS_BUILD_TYPE = "ro.build.type";
char const *ANDROID_OS_BUILD_TAGS = "ro.build.tags";
char const *ANDROID_OS_BUILD_FINGERPRINT = "ro.build.fingerprint";

static inline std::string _ngenxx_android_sys_property(const std::string &k)
{
    char v[PROP_VALUE_MAX];
    __system_property_get(k.c_str(), v);
    return v[0] ? std::string(v) : "";
}

int NGenXX::Device::DeviceInfo::deviceType()
{
    return NGenXXDeviceTypeAndroid;
}

std::string NGenXX::Device::DeviceInfo::deviceName()
{
    return _ngenxx_android_sys_property(ANDROID_OS_BUILD_MODEL);
}

std::string NGenXX::Device::DeviceInfo::deviceManufacturer()
{
    return _ngenxx_android_sys_property(ANDROID_OS_BUILD_MANUFACTURER);
}

std::string NGenXX::Device::DeviceInfo::osVersion()
{
    return _ngenxx_android_sys_property(ANDROID_OS_BUILD_VERSION_RELEASE);
}

int NGenXX::Device::DeviceInfo::cpuArch()
{
#if defined(__aarch64__) || defined(_M_ARM64)
    return NGenXXDeviceCpuArchARM_64;
#endif
    return NGenXXDeviceCpuArchARM;
}

#endif