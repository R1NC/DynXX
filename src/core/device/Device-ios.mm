#if defined(__APPLE__) && defined(__ENVIRONMENT_IPHONE_OS_VERSION_MIN_REQUIRED__)
#include "Device.hxx"

#import <UIKit/UIKit.h>

#import <sys/utsname.h>
#import <sys/sysctl.h>

#include <DynXX/C/Device.h>

#define NSString2CharP(nsstr) [nsstr cStringUsingEncoding:NSUTF8StringEncoding]
#define CharP2NSString(cstr) [NSString stringWithCString:cstr encoding:NSUTF8StringEncoding]

namespace
{
    char *sysCtlByName(const char *key) 
    {
        size_t size;
        sysctlbyname(key, nullptr, &size, nullptr, 0);
        auto value = reinterpret_cast<char*>(malloc(size * sizeof(char)));
        sysctlbyname(key, value, &size, nullptr, 0);
        return value;
    }
}

namespace DynXX::Core::Device {

int deviceType()
{
    return DynXXDeviceTypeApplePhone;
}

std::string deviceName()
{
    struct utsname systemInfo;
    uname(&systemInfo);
    return systemInfo.machine;
}

std::string deviceManufacturer()
{
    return "Apple";
}

std::string deviceModel()
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

std::string osVersion()
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

int cpuArch()
{
#if defined(__aarch64__) || defined(_M_ARM64)
    return DynXXDeviceCpuArchARM_64;
#elif defined(__arm__)
    return DynXXDeviceCpuArchARM;
#endif
    return DynXXDeviceCpuArchUnknown;
}

} // namespace DynXX::Core::Device

#endif