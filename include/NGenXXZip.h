#ifndef NGENXX_ZIP_H_
#define NGENXX_ZIP_H_

#include "NGenXXTypes.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * ZIP compress mode
 */
    enum NGenXXZipCompressMode
    {
        NGenXXZipCompressModeDefault = -1, //default
        NGenXXZipCompressModePreferSpeed = 1, //Best speed
        NGenXXZipCompressModePreferSize = 9, //Smallest output size
    };

    /**
     * @brief initialize a ZIP process
     * @param mode Compress mode
     * @param bufferSize buffer size
     * @return a ZIP handle
     */
    void *ngenxx_z_zip_init(const int mode, const size bufferSize);

    /**
     * @brief input data to ZIP process
     * @param zip The ZIP handle
     * @param in input data bytes
     * @param inLen input data length
     * @param inFinish Whether input is finished or not
     * @return The received data length
     */
    const size ngenxx_z_zip_input(const void *zip, const byte *in, const size inLen, const bool inFinish);

    /**
     * @brief process the ZIP input data
     * @param zip The ZIP handle
     * @param outLen a pointer to read the output data length
     * @return output data bytes
     */
    const byte *ngenxx_z_zip_process_do(const void *zip, size *outLen);

    /**
     * @brief check whether ZIP process is finished
     * @param zip The ZIP handle
     * @return whether ZIP process is finished or not
     */
    const bool ngenxx_z_zip_process_finished(const void *zip);

    /**
     * @brief release a ZIP process
     * @param zip The ZIP handle
     */
    void ngenxx_z_zip_release(const void *zip);

    /**
     * @brief initialize a UNZIP process
     * @param bufferSize buffer size
     * @return a UNZIP handle
     */
    void *ngenxx_z_unzip_init(const size bufferSize);

    /**
     * @brief input data to UNZIP process
     * @param unzip The UNZIP handle
     * @param in input data bytes
     * @param inLen input data length
     * @param inFinish Whether input is finished or not
     * @return The received data length
     */
    const size ngenxx_z_unzip_input(const void *unzip, const byte *in, const size inLen, const bool inFinish);

    /**
     * @brief process the UNZIP input data
     * @param unzip The UNZIP handle
     * @param outLen a pointer to read the output data length
     * @return output data bytes
     */
    const byte *ngenxx_z_unzip_process_do(const void *unzip, size *outLen);

    /**
     * @brief check whether UNZIP process is finished
     * @param unzip The UNZIP handle
     * @return whether UNZIP process is finished or not
     */
    const bool ngenxx_z_unzip_process_finished(const void *unzip);

    /**
     * @brief release a unzip process
     * @param unzip The unzip handle
     */
    void ngenxx_z_unzip_release(const void *unzip);

#ifdef __cplusplus
}
#endif

#endif // NGENXX_ZIP_H_