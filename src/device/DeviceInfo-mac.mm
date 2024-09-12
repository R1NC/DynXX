#ifdef __APPLE__
#ifdef __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__

#include "DeviceInfo.hxx"
#include "../../include/NGenXXDeviceInfo.h"

#import <Foundation/Foundation.h>
#include <mach/mach_host.h>
#include <mach/mach_init.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/sysctl.h>
#include <sys/types.h>
 #import <sys/utsname.h>

#include <stdlib.h>

#if defined(ARCH_CPU_X86_64)
// https://developer.apple.com/documentation/apple_silicon/about_the_rosetta_translation_environment#3616845
bool ProcessIsTranslated() {
  int ret = 0;
  size_t size = sizeof(ret);
  if (sysctlbyname("sysctl.proc_translated", &ret, &size, nullptr, 0) == -1) {
    return false;
  }
  return ret;
}
#endif

int NGenXX::Device::DeviceInfo::deviceType()
{
    return NGenXXDeviceTypeAppleMac;
}

std::string NGenXX::Device::DeviceInfo::deviceName()
{
    struct utsname systemInfo;
    uname(&systemInfo);
    return std::string(systemInfo.machine);
}

std::string NGenXX::Device::DeviceInfo::deviceManufacturer()
{
    return "Apple";
}

std::string NGenXX::Device::DeviceInfo::osVersion()
{
    NSOperatingSystemVersion version = NSProcessInfo.processInfo.operatingSystemVersion;
    int major = (int32_t)(version.majorVersion);
    int minor = (int32_t)(version.minorVersion);
    int bugfix = (int32_t)(version.patchVersion);
    char osv[256];                                                                                                 \
    sprintf(osv, "%d.%d.%d", major, minor, bugfix);     
    return std::string(osv);
}

int NGenXX::Device::DeviceInfo::cpuArch()
{
#if defined(ARCH_CPU_X86_64)
    if (!ProcessIsTranslated())
        return NGenXXDeviceCpuArchX86_64;
    }
#else
    return NGenXXDeviceCpuArchARM_64;
#endif
}

#endif
#endif