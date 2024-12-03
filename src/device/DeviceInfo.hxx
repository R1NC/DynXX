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
            char const *const SYS_PROPERTY_VERSION_RELEASE = "ro.build.version.release";
            char const *const SYS_PROPERTY_VERSION_INCREMENTAL = "ro.build.version.incremental";
            char const *const SYS_PROPERTY_VERSION_CODENAME = "ro.build.version.codename";
            char const *const SYS_PROPERTY_VERSION_SDK = "ro.build.version.sdk";

            char const *const SYS_PROPERTY_MODEL = "ro.product.model";
            char const *const SYS_PROPERTY_MANUFACTURER = "ro.product.manufacturer";
            char const *const SYS_PROPERTY_BOARD = "ro.product.board";
            char const *const SYS_PROPERTY_BRAND = "ro.product.brand";
            char const *const SYS_PROPERTY_DEVICE = "ro.product.device";
            char const *const SYS_PROPERTY_PRODUCT = "ro.product.name";

            char const *const SYS_PROPERTY_HARDWARE = "ro.hardware";

            char const *const SYS_PROPERTY_CPU_ABI = "ro.product.cpu.abi";
            char const *const SYS_PROPERTY_CPU_ABI2 = "ro.product.cpu.abi2";

            char const *const SYS_PROPERTY_DISPLAY = "ro.build.display.id";
            char const *const SYS_PROPERTY_HOST = "ro.build.host";
            char const *const SYS_PROPERTY_USER = "ro.build.user";
            char const *const SYS_PROPERTY_ID = "ro.build.id";
            char const *const SYS_PROPERTY_TYPE = "ro.build.type";
            char const *const SYS_PROPERTY_TAGS = "ro.build.tags";
            char const *const SYS_PROPERTY_FINGERPRINT = "ro.build.fingerprint";

            const std::string sysProperty(const std::string &k);

            const int apiLevel();
#endif
        }
    }
}

#endif

#endif // NGENXX_SRC_DEVICE_INFO_HXX_