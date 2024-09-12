#ifdef __EMSCRIPTEN__

#include "DeviceInfo.hxx"
#include "../../include/NGenXXDeviceInfo.h"

int NGenXX::Device::DeviceInfo::deviceType()
{
    return NGenXXDeviceTypeWeb;
}

std::string NGenXX::Device::DeviceInfo::deviceName()
{
    //TODO
}

std::string NGenXX::Device::DeviceInfo::deviceManufacturer()
{
    //TODO
}

std::string NGenXX::Device::DeviceInfo::osVersion()
{
    //TODO
}

int NGenXX::Device::DeviceInfo::cpuArch()
{
    //TODO
}

#endif