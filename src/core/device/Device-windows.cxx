#if (defined(_WIN32) || defined(_WIN64))
#include "Device.hxx"

#include <cstddef>
#include <cstdint>

#include <Windows.h>

#include <DynXX/C/Device.h>

namespace DynXX::Core::Device {

int deviceType()
{
    return DynXXDeviceTypeWindows;
}

std::string deviceName()
{
    //TODO
    return {};
}

std::string deviceManufacturer()
{
    return "Microsoft";
}

std::string deviceModel()
{
    //TODO
    return {};
}

std::string osVersion()
{
    //TODO
    return {};
}

int cpuArch()
{
    //TODO
    return DynXXDeviceCpuArchUnknown;
}

} // namespace DynXX::Core::Device

#endif