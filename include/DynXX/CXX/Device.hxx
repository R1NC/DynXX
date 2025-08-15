#ifndef DYNXX_INCLUDE_DEVICE_HXX_
#define DYNXX_INCLUDE_DEVICE_HXX_

#include "Types.hxx"

enum class DynXXDeviceTypeX: int {
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

enum class DynXXDeviceCpuArchX: int {
    Unknown,
    X86,
    X86_64,
    IA64,
    ARM,
    ARM_64
};

DynXXDeviceTypeX dynxxDeviceType();

std::string dynxxDeviceName();

std::string dynxxDeviceManufacturer();

std::string dynxxDeviceModel();

std::string dynxxDeviceOsVersion();

DynXXDeviceCpuArchX dynxxDeviceCpuArch();

#endif // DYNXX_INCLUDE_DEVICE_HXX_
