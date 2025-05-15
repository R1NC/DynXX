#if defined(__APPLE__) && defined(__ENVIRONMENT_IPHONE_OS_VERSION_MIN_REQUIRED__)
#include "DeviceInfo.hxx"

#import <UIKit/UIKit.h>

#import <sys/utsname.h>
#import <sys/sysctl.h>

#include <NGenXXDeviceInfo.h>

#define NSString2CharP(nsstr) [nsstr cStringUsingEncoding:NSUTF8StringEncoding]
#define CharP2NSString(cstr) [NSString stringWithCString:cstr encoding:NSUTF8StringEncoding]

namespace
{
    char *sysCtlByName(const char *key) 
    {
        size_t size;
        sysctlbyname(key, NULL, &size, NULL, 0);
        auto value = reinterpret_cast<char*>(malloc(size * sizeof(char)));
        sysctlbyname(key, value, &size, NULL, 0);
        return value;
    }
}

int NGenXX::DeviceInfo::deviceType()
{
    return NGenXXDeviceTypeApplePhone;
}

std::string NGenXX::DeviceInfo::deviceName()
{
    struct utsname systemInfo;
    uname(&systemInfo);
    return systemInfo.machine;
}

std::string NGenXX::DeviceInfo::deviceManufacturer()
{
    return "Apple";
}

std::string NGenXX::DeviceInfo::deviceModel()
{
    static dispatch_once_t get_system_model_once;
    static std::string *model;
    dispatch_once(&get_system_model_once, ^{
        @autoreleasepool {
            model = new std::string(sysCtlByName("hw.machine"));
        }
    });
    return *model;
}

std::string NGenXX::DeviceInfo::osVersion()
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

int NGenXX::DeviceInfo::cpuArch()
{
#if defined(__aarch64__) || defined(_M_ARM64)
    return NGenXXDeviceCpuArchARM_64;
#elif defined(__arm__)
    return NGenXXDeviceCpuArchARM;
#endif
    return NGenXXDeviceCpuArchUnknown;
}

#endif