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

            std::string deviceModel();

            std::string osVersion();

            int cpuArch();
        }
    }
}

#endif

#endif // NGENXX_SRC_DEVICE_INFO_HXX_