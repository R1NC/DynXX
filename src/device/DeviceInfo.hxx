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
        }
    }
}

#endif

#endif // NGENXX_DEVICE_INFO_HXX_