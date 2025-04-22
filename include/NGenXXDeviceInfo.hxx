#ifndef NGENXX_INCLUDE_DEVICE_INFO_HXX_
#define NGENXX_INCLUDE_DEVICE_INFO_HXX_

#include <string>

enum class NGenXXDeviceTypeX: int {
    Unknown,
    Android,
    ApplePhone,
    ApplePad,
    AppleMac,
    AppleWatch,
    AppleTV,
    HarmonyOS,
    Windows,
    Linux,
    Web
};

enum class NGenXXDeviceCpuArchX: int {
    Unknown,
    X86,
    X86_64,
    IA64,
    ARM,
    ARM_64
};

NGenXXDeviceTypeX ngenxxDeviceType();

std::string ngenxxDeviceName();

std::string ngenxxDeviceManufacturer();

std::string ngenxxDeviceModel();

std::string ngenxxDeviceOsVersion();

NGenXXDeviceCpuArchX ngenxxDeviceCpuArch();

#endif // NGENXX_INCLUDE_DEVICE_INFO_HXX_