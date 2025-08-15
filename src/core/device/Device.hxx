#ifndef DYNXX_SRC_CORE_DEVICE_HXX_
#define DYNXX_SRC_CORE_DEVICE_HXX_

#if defined(__cplusplus)

#include <string>

namespace DynXX::Core::Device {
    int deviceType();

    std::string deviceName();

    std::string deviceManufacturer();

    std::string deviceModel();

    std::string osVersion();

    int cpuArch();
}

#endif

#endif // DYNXX_SRC_CORE_DEVICE_HXX_
