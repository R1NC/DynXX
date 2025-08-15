#ifndef DYNXX_INCLUDE_DEVICE_H_
#define DYNXX_INCLUDE_DEVICE_H_

#include "Types.h"

EXTERN_C_BEGIN

/**
 * Device Type
 */
enum DynXXDeviceType {
    DynXXDeviceTypeUnknown,
    DynXXDeviceTypeAndroid,
    DynXXDeviceTypeApplePhone,
    DynXXDeviceTypeApplePad,
    DynXXDeviceTypeAppleMac,
    DynXXDeviceTypeAppleWatch,
    DynXXDeviceTypeAppleTV,
    DynXXDeviceTypeHarmonyOS,
    DynXXDeviceTypeWindows,
    DynXXDeviceTypeLinux,
    DynXXDeviceTypeWeb
};

/**
 * CPU Arch
 */
enum DynXXDeviceCpuArch {
    DynXXDeviceCpuArchUnknown,
    DynXXDeviceCpuArchX86,
    DynXXDeviceCpuArchX86_64,
    DynXXDeviceCpuArchIA64,
    DynXXDeviceCpuArchARM,
    DynXXDeviceCpuArchARM_64
};

/**
 * @brief Read device type
 * @return device type
 */
int dynxx_device_type();

/**
 * @brief Read device name
 * @return device name
 */
const char *dynxx_device_name();

/**
 * @brief Read device manufacturer
 * @return device manufacturer
 */
const char *dynxx_device_manufacturer();

/**
* @brief Read device model
* @return device model
*/
const char *dynxx_device_model();

/**
 * @brief Read OS version
 * @return OS VERSION
 */
const char *dynxx_device_os_version();

/**
 * @brief Read CPU arch
 * @return CPU arch
 */
int dynxx_device_cpu_arch();

EXTERN_C_END

#endif // DYNXX_INCLUDE_DEVICE_H_
