#ifndef DYNXX_INCLUDE_CXX_DEVICE_HXX_
#define DYNXX_INCLUDE_CXX_DEVICE_HXX_

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
    Linux
};

enum class DynXXDeviceCpuArchX: int {
    Unknown,
    X86,
    X86_64,
    ARM,
    ARM_64,
    IA64,
    MIPS,
    MIPS_64,
    RISCV,
    RISCV_64,
    PowerPC,
    PowerPC_64,
    S390,
    S390_64
};

DynXXDeviceTypeX dynxxDeviceType();

std::string dynxxDeviceName();

std::string dynxxDeviceManufacturer();

std::string dynxxDeviceModel();

std::string dynxxDeviceOsVersion();

DynXXDeviceCpuArchX dynxxDeviceCpuArch();

#endif // DYNXX_INCLUDE_CXX_DEVICE_HXX_
