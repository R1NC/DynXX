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
        NGenXXZipCompressModeDefault = -1,    // default
        NGenXXZipCompressModePreferSpeed = 1, // Best speed
        NGenXXZipCompressModePreferSize = 9,  // Smallest output size
    };

    /**
     * Z header type
     */
    enum NGenXXZFormat
    {
        NGenXXZFormatZLib = 0,  // Default zlib header
        NGenXXZFormatGZip = 1, // GZip header
        NGenXXZFormatRaw = 2,  // No header
    };

    /**
     * @brief initialize a ZIP process
     * @param mode ZIP mode, see `NGenXXZipCompressMode`
     * @param bufferSize buffer size，must be positive
     * @param format header type, see `NGenXXZFormat`
     * @return a ZIP handle
     */
    void *const ngenxx_z_zip_init(const int mode, const size bufferSize, const int format);

    /**
     * @brief input data to ZIP process
     * @param zip The ZIP handle
     * @param inBytes input data bytes
     * @param inLen input data length
     * @param inFinish Whether input is finished or not
     * @return The received data length, return `0` if error occurred
     */
    const size ngenxx_z_zip_input(void *const zip, const byte *inBytes, const size inLen, const bool inFinish);

    /**
     * @brief process the ZIP input data
     * @param zip The ZIP handle
     * @param outLen a pointer to read the output data length
     * @return output data bytes, return `NULL` if error occurred
     */
    const byte *ngenxx_z_zip_process_do(void *const zip, size *outLen);

    /**
     * @brief check whether all the ZIP data inputed before are processed
     * @param zip The ZIP handle
     * @return whether finished or not
     */
    const bool ngenxx_z_zip_process_finished(void *const zip);

    /**
     * @brief release a ZIP process
     * @param zip The ZIP handle
     */
    void ngenxx_z_zip_release(void *const zip);

    /**
     * @brief initialize a UNZIP process
     * @param bufferSize buffer size，must be positive
     * @param format header type, see `NGenXXZFormat`
     * @return a UNZIP handle
     */
    void *const ngenxx_z_unzip_init(const size bufferSize, const int header);

    /**
     * @brief input data to UNZIP process
     * @param unzip The UNZIP handle
     * @param inBytes input data bytes
     * @param inLen input data length
     * @param inFinish Whether input is finished or not
     * @return The received data length, return `0` if error occurred
     */
    const size ngenxx_z_unzip_input(void *const unzip, const byte *inBytes, const size inLen, const bool inFinish);

    /**
     * @brief process the UNZIP input data
     * @param unzip The UNZIP handle
     * @param outLen a pointer to read the output data length
     * @return output data bytes, return `NULL` if error occurred
     */
    const byte *ngenxx_z_unzip_process_do(void *const unzip, size *outLen);

    /**
     * @brief check whether all the ZIP data inputed before are processed
     * @param unzip The UNZIP handle
     * @return whether finished or not
     */
    const bool ngenxx_z_unzip_process_finished(void *const unzip);

    /**
     * @brief release a unzip process
     * @param unzip The unzip handle
     */
    void ngenxx_z_unzip_release(void *const unzip);

    /**
     * @brief ZIP for C FILE
     * @warning Not accessible in JS/Lua!
     * @param mode ZIP mode, see `NGenXXZipCompressMode`
     * @param bufferSize buffer size，must be positive
     * @param format header type, see `NGenXXZFormat`
     * @param cFILEIn Input C `FILE`
     * @param cFILEOut Output C `FILE`
     * @return whether finished or not
     */
    bool ngenxx_z_cfile_zip(const int mode, const size bufferSize, const int format, void *const cFILEIn, void *const cFILEOut);

    /**
     * @brief UNZIP for C FILE
     * @warning Not accessible in JS/Lua!
     * @param bufferSize buffer size，must be positive
     * @param format header type, see `NGenXXZFormat`
     * @param cFILEIn Input C `FILE`
     * @param cFILEOut Output C `FILE`
     * @return whether finished or not
     */
    bool ngenxx_z_cfile_unzip(const size bufferSize, const int format, void *const cFILEIn, void *const cFILEOut);

    /**
     * @brief ZIP for C++ Stream
     * @warning Not accessible in JS/Lua!
     * @param mode ZIP mode, see `NGenXXZipCompressMode`
     * @param bufferSize buffer size，must be positive
     * @param format header type, see `NGenXXZFormat`
     * @param cxxStreamIn Input C++ Stream(`istream`)
     * @param cxxStreamOut Output C++ Stream(`ostream`)
     * @return whether finished or not
     */
    bool ngenxx_z_cxxstream_zip(const int mode, const size bufferSize, const int format, void *const cxxStreamIn, void *const cxxStreamOut);

    /**
     * @brief UNZIP for C++ Stream
     * @warning Not accessible in JS/Lua!
     * @param bufferSize buffer size，must be positive
     * @param format header type, see `NGenXXZFormat`
     * @param cxxStreamIn Input C++ Stream(`istream`)
     * @param cxxStreamOut Output C++ Stream(`ostream`)
     * @return whether finished or not
     */
    bool ngenxx_z_cxxstream_unzip(const size bufferSize, const int format, void *const cxxStreamIn, void *const cxxStreamOut);

    /**
     * @brief ZIP for bytes
     * @param mode ZIP mode, see `NGenXXZipCompressMode`
     * @param bufferSize buffer size，must be positive
     * @param format header type, see `NGenXXZFormat`
     * @param inBytes Input bytes data
     * @param inLen Input bytes length
     * @param outLen A pointer to read output bytes length
     * @return output bytes data
     */
    const byte *ngenxx_z_bytes_zip(const int mode, const size bufferSize, const int format, const byte *inBytes, const size inLen, size *outLen);

    /**
     * @brief UNZIP for bytes
     * @param bufferSize buffer size，must be positive
     * @param format header type, see `NGenXXZFormat`
     * @param inBytes Input bytes data
     * @param inLen Input bytes length
     * @param outLen A pointer to read output bytes length
     * @return output bytes data
     */
    const byte *ngenxx_z_bytes_unzip(const size bufferSize, const int format, const byte *inBytes, const size inLen, size *outLen);

#ifdef __cplusplus
}
#endif

#endif // NGENXX_ZIP_H_