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
    std::string sysCtlByName(const char* key) {
        if (!key) return {};
        size_t size = 0;
        
        if (const auto ret = sysctlbyname(key, nullptr, &size, nullptr, 0); ret != 0 || size == 0) [[unlikely]] {
            return {};
        }
        
        std::string result(size, '\0');
        if (sysctlbyname(key, result.data(), &size, nullptr, 0) != 0) [[unlikely]] {
            return {};
        }

        if (!result.empty() && result.back() == '\0') {
            result.pop_back();
        }

        return result;
    }
}

namespace DynXX::Core::Device {

DynXXDeviceTypeX deviceType()
{
    return DynXXDeviceTypeX::ApplePhone;
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
    static const auto model = sysCtlByName("hw.machine");
    return model;
}

std::string osVersion()
{
    static const auto version = []() -> std::string {
        @autoreleasepool {
            NSString *nsVersion = UIDevice.currentDevice.systemVersion;
            if (!nsVersion) [[unlikely]] {
                return "Unknown";
            }
            const auto cstr = NSString2CharP(nsVersion);
            if (!cstr) [[unlikely]] {
                return "Unknown";
            }
            return {cstr};
        }
    }();
    return version;
}

DynXXDeviceCpuArchX cpuArch()
{
#if defined(__aarch64__) || defined(_M_ARM64)
    return DynXXDeviceCpuArchX::ARM_64;
#elif defined(__arm__)
    return DynXXDeviceCpuArchX::ARM;
#endif
    return DynXXDeviceCpuArchX::Unknown;
}

} // namespace DynXX::Core::Device

#endif