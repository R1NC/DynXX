#ifndef NGENXX_DEVICE_INFO_HXX_
#define NGENXX_DEVICE_INFO_HXX_

#ifdef __cplusplus

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

#ifdef __ANDROID__
            char const *ANDROID_OS_BUILD_VERSION_RELEASE = "ro.build.version.release";
            char const *ANDROID_OS_BUILD_VERSION_INCREMENTAL = "ro.build.version.incremental";
            char const *ANDROID_OS_BUILD_VERSION_CODENAME = "ro.build.version.codename";
            char const *ANDROID_OS_BUILD_VERSION_SDK = "ro.build.version.sdk";

            char const *ANDROID_OS_BUILD_MODEL = "ro.product.model";
            char const *ANDROID_OS_BUILD_MANUFACTURER = "ro.product.manufacturer";
            char const *ANDROID_OS_BUILD_BOARD = "ro.product.board";
            char const *ANDROID_OS_BUILD_BRAND = "ro.product.brand";
            char const *ANDROID_OS_BUILD_DEVICE = "ro.product.device";
            char const *ANDROID_OS_BUILD_PRODUCT = "ro.product.name";

            char const *ANDROID_OS_BUILD_HARDWARE = "ro.hardware";

            char const *ANDROID_OS_BUILD_CPU_ABI = "ro.product.cpu.abi";
            char const *ANDROID_OS_BUILD_CPU_ABI2 = "ro.product.cpu.abi2";

            char const *ANDROID_OS_BUILD_DISPLAY = "ro.build.display.id";
            char const *ANDROID_OS_BUILD_HOST = "ro.build.host";
            char const *ANDROID_OS_BUILD_USER = "ro.build.user";
            char const *ANDROID_OS_BUILD_ID = "ro.build.id";
            char const *ANDROID_OS_BUILD_TYPE = "ro.build.type";
            char const *ANDROID_OS_BUILD_TAGS = "ro.build.tags";
            char const *ANDROID_OS_BUILD_FINGERPRINT = "ro.build.fingerprint";

            const std::string sysProperty(const std::string &k);

            const int apiLevel();
#endif
        }
    }
}

#endif

#endif // NGENXX_DEVICE_INFO_HXX_