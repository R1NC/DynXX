#ifndef NGENXX_SRC_CORE_DEVICE_HXX_
#define NGENXX_SRC_CORE_DEVICE_HXX_

#if defined(__cplusplus)

#include <string>

namespace NGenXX::Core::Device
        {
            int deviceType();

            std::string deviceName();

            std::string deviceManufacturer();

            std::string deviceModel();

            std::string osVersion();

            int cpuArch();
}

#endif

#endif // NGENXX_SRC_CORE_DEVICE_HXX_