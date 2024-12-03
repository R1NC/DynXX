#if defined(__APPLE__) && defined(__ENVIRONMENT_IPHONE_OS_VERSION_MIN_REQUIRED__)

#include "DeviceInfo.hxx"
#include "../../include/NGenXXDeviceInfo.h"

#import <UIKit/UIKit.h>
#import <sys/utsname.h>

#define NSString2CharP(nsstr) [nsstr cStringUsingEncoding:NSUTF8StringEncoding]

int NGenXX::Device::DeviceInfo::deviceType()
{
    return NGenXXDeviceTypeApplePhone;
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
    static dispatch_once_t get_system_version_once;
    static std::string *osv;
    dispatch_once(&get_system_version_once, ^{
        @autoreleasepool {
            osv = new std::string(NSString2CharP(UIDevice.currentDevice.systemVersion));
        }
    });
    return *osv;
}

int NGenXX::Device::DeviceInfo::cpuArch()
{
#if defined(__aarch64__) || defined(_M_ARM64)
    return NGenXXDeviceCpuArchARM_64;
#elif defined(__arm__)
    return NGenXXDeviceCpuArchARM;
#endif
    return NGenXXDeviceCpuArchUnknown;
}

#endif