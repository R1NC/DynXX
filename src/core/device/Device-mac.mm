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

#include <NGenXXDevice.h>
#include <NGenXXTypes.hxx>

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

int NGenXX::Core::Device::deviceType()
{
    return NGenXXDeviceTypeAppleMac;
}

std::string NGenXX::Core::Device::deviceName()
{
    struct utsname systemInfo{};
    uname(&systemInfo);
    return wrapStr(systemInfo.machine);
}

std::string NGenXX::Core::Device::deviceManufacturer()
{
    return "Apple";
}

std::string NGenXX::Core::Device::deviceModel()
{
    //TODO
    return {};
}

std::string NGenXX::Core::Device::osVersion()
{
    NSOperatingSystemVersion version = NSProcessInfo.processInfo.operatingSystemVersion;
    int major = static_cast<int32_t>(version.majorVersion);
    int minor = static_cast<int32_t>(version.minorVersion);
    int bugfix = static_cast<int32_t>(version.patchVersion);
    std::ostringstream ss;
    ss << major << "." << minor << "." << bugfix;  
    return ss.str();
}

int NGenXX::Core::Device::cpuArch()
{
#if defined(ARCH_CPU_X86_64)
    if (!ProcessIsTranslated())
    {
        return NGenXXDeviceCpuArchX86_64;
    }
#else
    return NGenXXDeviceCpuArchARM_64;
#endif
}

#endif