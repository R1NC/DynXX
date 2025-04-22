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
    NSString *systemsInfoByName(const char *typeName) 
    {
        size_t size;
        sysctlbyname(typeName, NULL, &size, NULL, 0);
        char *answer = reinterpret_cast<char*>(malloc(size * sizeof(char)));
        sysctlbyname(typeName, answer, &size, NULL, 0);
        NSString *results = CharP2NSString(answer);
        free(answer);
        return results;
    }
}

int NGenXX::Device::DeviceInfo::deviceType()
{
    return NGenXXDeviceTypeApplePhone;
}

std::string NGenXX::Device::DeviceInfo::deviceName()
{
    struct utsname systemInfo;
    uname(&systemInfo);
    return systemInfo.machine;
}

std::string NGenXX::Device::DeviceInfo::deviceManufacturer()
{
    return "Apple";
}

std::string NGenXX::Device::DeviceInfo::deviceModel()
{
    static dispatch_once_t get_system_model_once;
    static std::string *model;
    dispatch_once(&get_system_model_once, ^{
        @autoreleasepool {
            model = new std::string(NSString2CharP(systemsInfoByName("hw.machine")));
        }
    });
    return *model;
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