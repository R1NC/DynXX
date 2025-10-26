#pragma once

#include <DynXX/CXX/Device.hxx>

namespace DynXX::Core::Device {
    DynXXDeviceTypeX deviceType();

    std::string deviceName();

    std::string deviceManufacturer();

    std::string deviceModel();

    std::string osVersion();

    DynXXDeviceCpuArchX cpuArch();
}  // namespace DynXX::Core::Device
