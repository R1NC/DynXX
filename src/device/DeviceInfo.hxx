#ifndef NGENXX_SRC_DEVICE_INFO_HXX_
#define NGENXX_SRC_DEVICE_INFO_HXX_

#if defined(__cplusplus)

#include <string>

namespace NGenXX
{
    namespace Device
    {
        namespace DeviceInfo
        {
            int deviceType();

            std::string deviceName();

            std::string deviceManufacturer();

            std::string osVersion();

            int cpuArch();

#if defined(__ANDROID__)
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

            std::string sysProperty(const std::string &k);

            int apiLevel();
#endif
        }
    }
}

#endif

#endif // NGENXX_SRC_DEVICE_INFO_HXX_