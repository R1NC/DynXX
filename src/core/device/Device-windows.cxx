#if (defined(_WIN32) || defined(_WIN64))
#include "Device.hxx"

#include <cstddef>
#include <cstdint>

#include <Windows.h>

#include <DynXX/C/Device.h>

namespace DynXX::Core::Device {

DynXXDeviceTypeX deviceType()
{
    return DynXXDeviceTypeX::Windows;
}

std::string deviceName()
{
    static const auto name = []() -> std::string {
        DWORD size = 0;
        if (!GetComputerNameA(nullptr, &size)) [[unlikely]] {
            if (GetLastError() != ERROR_BUFFER_OVERFLOW)  [[unlikely]] {
                return {};
            }
        }
        std::string result(size, '\0');
        if (!GetComputerNameA(result.data(), &size)) [[unlikely]] {
            return {};
        }
        result.resize(size);
        return result;
    }();
    return name;
}

std::string deviceManufacturer()
{
    return "Microsoft";
}

std::string deviceModel()
{
    //TODO
    return {};
}

std::string osVersion()
{
    //TODO
    return "Windows Unknown";
}

DynXXDeviceCpuArchX cpuArch()
{
#if defined(_M_X64) || defined(__x86_64__)
    return DynXXDeviceCpuArchX::X86_64;
#elif defined(_M_ARM64) || defined(__aarch64__)
    return DynXXDeviceCpuArchX::ARM_64;
#elif defined(_M_IX86) || defined(__i386__)
    return DynXXDeviceCpuArchX::X86;
#elif defined(_M_ARM) || defined(__arm__)
    return DynXXDeviceCpuArchX::ARM;
#else
    return DynXXDeviceCpuArchX::Unknown;
#endif
}

} // namespace DynXX::Core::Device

#endif