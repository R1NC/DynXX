#if (!defined(__ANDROID__) && !defined(__APPLE__) && !defined(__OHOS__) && !defined(_WIN32) && !defined(__EMSCRIPTEN__))
#include "Device.hxx"

#include <DynXX/C/Device.h>

namespace DynXX::Core::Device {

DynXXDeviceTypeX deviceType()
{
    return DynXXDeviceTypeX::Linux;
}

std::string deviceName()
{
    //TODO
    return {};
}

std::string deviceManufacturer()
{
    //TODO
    return {};
}

std::string deviceModel()
{
    //TODO
    return {};
}

std::string osVersion()
{
    //TODO
    return {};
}

DynXXDeviceCpuArchX cpuArch()
{
#if defined(__x86_64__) || defined(__amd64__)
    return DynXXDeviceCpuArchX::X86_64;
#elif defined(__i386__) || defined(__i486__) || defined(__i586__) || defined(__i686__)
    return DynXXDeviceCpuArchX::X86;
#elif defined(__aarch64__) || defined(__arm64__)
    return DynXXDeviceCpuArchX::ARM_64;
#elif defined(__arm__) || defined(__ARMEL__)
    return DynXXDeviceCpuArchX::ARM;
#elif defined(__ia64__)
    return DynXXDeviceCpuArchX::IA64;
#elif defined(__mips64)
    return DynXXDeviceCpuArchX::MIPS_64;
#elif defined(__mips__)
    return DynXXDeviceCpuArchX::MIPS;
#elif defined(__powerpc64__)
    return DynXXDeviceCpuArchX::PowerPC_64;
#elif defined(__powerpc__)
    return DynXXDeviceCpuArchX::PowerPC;
#elif defined(__riscv) && __riscv_xlen == 64
    return DynXXDeviceCpuArchX::RISCV_64;
#elif defined(__riscv) && __riscv_xlen == 32
    return DynXXDeviceCpuArchX::RISCV;
#elif defined(__s390x__)
    return DynXXDeviceCpuArchX::S390_64;
#elif defined(__s390__)
    return DynXXDeviceCpuArchX::S390;
#else
    return DynXXDeviceCpuArchX::Unknown;
#endif
}

} // namespace DynXX::Core::Device

#endif