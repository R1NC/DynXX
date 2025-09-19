#if defined(__APPLE__) && defined(__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__)
#include "Device.hxx"

#include <mach/mach_host.h>
#include <mach/mach_init.h>
#include <sys/sysctl.h>
#include <sys/types.h>
#include <sys/utsname.h>

#include <cstddef>
#include <cstdint>
#include <cstdlib>

#import <Foundation/Foundation.h>

#include <sstream>

#include <DynXX/C/Device.h>
#include <DynXX/CXX/Types.hxx>

namespace
{
    #if defined(ARCH_CPU_X86_64)
    // https://developer.apple.com/documentation/apple_silicon/about_the_rosetta_translation_environment#3616845
    bool ProcessIsTranslated() 
    {
        int ret = 0;
        size_t size = sizeof(ret);
        if (sysctlbyname("sysctl.proc_translated", &ret, &size, nullptr, 0) == -1) 
        {
            return false;
        }
        return ret;
    }
#endif
}

namespace DynXX::Core::Device {

DynXXDeviceTypeX deviceType()
{
    return DynXXDeviceTypeX::AppleMac;
}

std::string deviceName()
{
    struct utsname systemInfo{};
    uname(&systemInfo);
    return makeStr(systemInfo.machine);
}

std::string deviceManufacturer()
{
    return "Apple";
}

std::string deviceModel()
{
    //TODO
    return {};
}

std::string osVersion()
{
    static const auto version = []() -> std::string {
        NSOperatingSystemVersion ver = NSProcessInfo.processInfo.operatingSystemVersion;
        int major = static_cast<int32_t>(ver.majorVersion);
        int minor = static_cast<int32_t>(ver.minorVersion);
        int patch = static_cast<int32_t>(ver.patchVersion);
        std::string result;
        result.reserve(16);
        result += std::to_string(major);
        result += '.';
        result += std::to_string(minor);
        result += '.';
        result += std::to_string(patch);
        return result;
    }();
    return version;
}

DynXXDeviceCpuArchX cpuArch()
{
#if defined(ARCH_CPU_X86_64)
    if (!ProcessIsTranslated())
    {
        return DynXXDeviceCpuArchX::X86_64;
    }
#else
    return DynXXDeviceCpuArchX::ARM_64;
#endif
}

} // namespace DynXX::Core::Device

#endif