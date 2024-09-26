#ifndef NGENXX_ZIP_H_
#define NGENXX_ZIP_H_

#include "NGenXXTypes.h"

#ifdef __cplusplus
extern "C"
{
#endif

    enum NGenXXZipCompressMode
    {
        NGenXXZipCompressModeDefault = -1,
        NGenXXZipCompressModePreferSpeed = 1,
        NGenXXZipCompressModePreferSize = 9,
    };

    /**
     * @brief initialize a zip process
     * @param mode Compress mode
     * @param bufferSize buffer size
     * @return a zip handle
     */
    void *ngenxx_z_zip_init(const int mode, const size bufferSize);

    /**
     * @brief input data to zip process
     * @param zip The zip handle
     * @param in input data bytes
     * @param inLen input data length
     * @param inFinish Whether input is finished or not
     * @return The received data length
     */
    const size ngenxx_z_zip_input(const void *zip, const byte *in, const size inLen, const bool inFinish);

    /**
     * @brief process the zip input data
     * @param zip The zip handle
     * @param outLen a pointer to read the output data length
     * @return output data bytes
     */
    const byte *ngenxx_z_zip_process_do(const void *zip, size *outLen);

    /**
     * @brief check whether zip process is finished
     * @param zip The zip handle
     * @return whether zip process is finished or not
     */
    const bool ngenxx_z_zip_process_finished(const void *zip);

    /**
     * @brief release a zip process
     * @param zip The zip handle
     */
    void ngenxx_z_zip_release(const void *zip);

    /**
     * @brief initialize a unzip process
     * @param mode Compress mode
     * @param bufferSize buffer size
     * @return a unzip handle
     */
    void *ngenxx_z_unzip_init(const size bufferSize);

    /**
     * @brief input data to unzip process
     * @param unzip The unzip handle
     * @param in input data bytes
     * @param inLen input data length
     * @param inFinish Whether input is finished or not
     * @return The received data length
     */
    const size ngenxx_z_unzip_input(const void *unzip, const byte *in, const size inLen, const bool inFinish);

    /**
     * @brief process the unzip input data
     * @param unzip The unzip handle
     * @param outLen a pointer to read the output data length
     * @return output data bytes
     */
    const byte *ngenxx_z_unzip_process_do(const void *unzip, size *outLen);

    /**
     * @brief check whether unzip process is finished
     * @param unzip The unzip handle
     * @return whether unzip process is finished or not
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