#if defined(__ANDROID__)
#include "Device.hxx"

#include <cstddef>

#include <sys/system_properties.h>
#include <android/api-level.h>

#include <DynXX/C/Device.h>

namespace
{
    constexpr auto SYS_PROPERTY_VERSION_RELEASE = "ro.build.version.release";
    constexpr auto SYS_PROPERTY_VERSION_INCREMENTAL = "ro.build.version.incremental";
    constexpr auto SYS_PROPERTY_VERSION_CODENAME = "ro.build.version.codename";
    constexpr auto SYS_PROPERTY_VERSION_SDK = "ro.build.version.sdk";

    constexpr auto SYS_PROPERTY_MODEL = "ro.product.model";
    constexpr auto SYS_PROPERTY_MANUFACTURER = "ro.product.manufacturer";
    constexpr auto SYS_PROPERTY_BOARD = "ro.product.board";
    constexpr auto SYS_PROPERTY_BRAND = "ro.product.brand";
    constexpr auto SYS_PROPERTY_DEVICE = "ro.product.device";
    constexpr auto SYS_PROPERTY_PRODUCT = "ro.product.name";

    constexpr auto SYS_PROPERTY_HARDWARE = "ro.hardware";

    constexpr auto SYS_PROPERTY_CPU_ABI = "ro.product.cpu.abi";
    constexpr auto SYS_PROPERTY_CPU_ABI2 = "ro.product.cpu.abi2";

    constexpr auto SYS_PROPERTY_DISPLAY = "ro.build.display.id";
    constexpr auto SYS_PROPERTY_HOST = "ro.build.host";
    constexpr auto SYS_PROPERTY_USER = "ro.build.user";
    constexpr auto SYS_PROPERTY_ID = "ro.build.id";
    constexpr auto SYS_PROPERTY_TYPE = "ro.build.type";
    constexpr auto SYS_PROPERTY_TAGS = "ro.build.tags";
    constexpr auto SYS_PROPERTY_FINGERPRINT = "ro.build.fingerprint";

    std::string sysProperty(const char *k)
    {
        if (k == nullptr) [[unlikely]] {
            return {};
        }
        std::string buffer(PROP_VALUE_MAX - 1, '\0');
        const auto len = __system_property_get(k, buffer.data());
        if (len <= 0) [[unlikely]] {
            return {};
        }
        return {buffer.data(), static_cast<size_t>(len)};
    }

    [[maybe_unused]] int apiLevel()
    {
        return android_get_device_api_level();
    }
}

namespace DynXX::Core::Device {

DynXXDeviceTypeX deviceType()
{
    return DynXXDeviceTypeX::Android;
}

std::string deviceName()
{
    return sysProperty(SYS_PROPERTY_MODEL);
}

std::string deviceManufacturer()
{
    return sysProperty(SYS_PROPERTY_MANUFACTURER);
}

std::string deviceModel()
{
    return sysProperty(SYS_PROPERTY_MODEL);
}

std::string osVersion()
{
    return sysProperty(SYS_PROPERTY_VERSION_RELEASE);
}

DynXXDeviceCpuArchX cpuArch()
{
#if defined(__aarch64__) || defined(_M_ARM64)
    return DynXXDeviceCpuArchX::ARM_64;
#endif
    return DynXXDeviceCpuArchX::ARM;
}

} // namespace DynXX::Core::Device

#endif
