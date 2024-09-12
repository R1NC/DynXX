#ifndef NGENXX_DEVICE_INFO_H_
#define NGENXX_DEVICE_INFO_H_

/**
 * Device Type
 */
enum NGenXXDeviceType {
    NGenXXDeviceTypeUnknown,
    NGenXXDeviceTypeAndroid,
    NGenXXDeviceTypeApplePhone,
    NGenXXDeviceTypeApplePad,
    NGenXXDeviceTypeAppleMac,
    NGenXXDeviceTypeAppleWatch,
    NGenXXDeviceTypeAppleTV,
    NGenXXDeviceTypeHarmonyOS,
    NGenXXDeviceTypeWindows,
    NGenXXDeviceTypeLinux,
    NGenXXDeviceTypeWeb
};

/**
 * CPU Arch
 */
enum NGenXXDeviceCpuArch {
    NGenXXDeviceCpuArchUnknown,
    NGenXXDeviceCpuArchX86,
    NGenXXDeviceCpuArchX86_64,
    NGenXXDeviceCpuArchIA64,
    NGenXXDeviceCpuArchARM,
    NGenXXDeviceCpuArchARM_64
};

/**
 * @brief Read device type
 * @return device type
 */
int ngenxx_device_type();

/**
 * @brief Read device name
 * @return device name
 */
const char *ngenxx_device_name();

/**
 * @brief Read device manufacturer
 * @return device manufacturer
 */
const char *ngenxx_device_manufacturer();

/**
 * @brief Read OS version
 * @return OS VERSION
 */
const char *ngenxx_device_os_version();

/**
 * @brief Read CPU arch
 * @return CPU arch
 */
int ngenxx_device_cpu_arch();

#endif // NGENXX_DEVICE_INFO_H_