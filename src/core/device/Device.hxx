#ifndef DYNXX_SRC_CORE_DEVICE_HXX_
#define DYNXX_SRC_CORE_DEVICE_HXX_

#if defined(__cplusplus)

#include <string>

#include <DynXX/CXX/Device.hxx>

namespace DynXX::Core::Device {
    DynXXDeviceTypeX deviceType();

    std::string deviceName();

    std::string deviceManufacturer();

    std::string deviceModel();

    std::string osVersion();

    DynXXDeviceCpuArchX cpuArch();
}

#endif

#endif // DYNXX_SRC_CORE_DEVICE_HXX_
