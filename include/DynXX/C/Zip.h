#ifndef DYNXX_INCLUDE_ZIP_H_
#define DYNXX_INCLUDE_ZIP_H_

#include "Types.h"

EXTERN_C_BEGIN

/**
 * ZIP compress mode
 */
enum DynXXZipCompressMode {
    DynXXZipCompressModeDefault = -1, // default
    DynXXZipCompressModePreferSpeed = 1, // Best speed
    DynXXZipCompressModePreferSize = 9, // Smallest output size
};

/**
 * Z header type
 */
enum DynXXZFormat {
    DynXXZFormatZLib = 0, // Default zlib header
    DynXXZFormatGZip = 1, // GZip header
    DynXXZFormatRaw = 2, // No header
};

/**
 * @brief initialize a ZIP process
 * @param mode ZIP mode, see `DynXXZipCompressMode`
 * @param bufferSize buffer size，must be positive
 * @param format header type, see `DynXXZFormat`
 * @return a ZIP handle
 */
void *dynxx_z_zip_init(int mode, size_t bufferSize, int format);

/**
 * @brief input data to ZIP process
 * @param zip The ZIP handle
 * @param inBytes input data bytes
 * @param inLen input data length
 * @param inFinish Whether input is finished or not
 * @return The received data length, return `0` if error occurred
 */
size_t dynxx_z_zip_input(void *const zip, const byte *inBytes, size_t inLen, bool inFinish);

/**
 * @brief process the ZIP input data
 * @param zip The ZIP handle
 * @param outLen a pointer to read the output data length
 * @return output data bytes, return `nullptr` if error occurred
 */
const byte *dynxx_z_zip_process_do(void *const zip, size_t *outLen);

/**
 * @brief check whether all the ZIP data inputed before are processed
 * @param zip The ZIP handle
 * @return whether finished or not
 */
bool dynxx_z_zip_process_finished(void *const zip);

/**
 * @brief release a ZIP process
 * @param zip The ZIP handle
 */
void dynxx_z_zip_release(void *const zip);

/**
 * @brief initialize a UNZIP process
 * @param bufferSize buffer size，must be positive
 * @param format header type, see `DynXXZFormat`
 * @return a UNZIP handle
 */
void *dynxx_z_unzip_init(size_t bufferSize, int header);

/**
 * @brief input data to UNZIP process
 * @param unzip The UNZIP handle
 * @param inBytes input data bytes
 * @param inLen input data length
 * @param inFinish Whether input is finished or not
 * @return The received data length, return `0` if error occurred
 */
size_t dynxx_z_unzip_input(void *const unzip, const byte *inBytes, size_t inLen, bool inFinish);

/**
 * @brief process the UNZIP input data
 * @param unzip The UNZIP handle
 * @param outLen a pointer to read the output data length
 * @return output data bytes, return `nullptr` if error occurred
 */
const byte *dynxx_z_unzip_process_do(void *const unzip, size_t *outLen);

/**
 * @brief check whether all the ZIP data inputed before are processed
 * @param unzip The UNZIP handle
 * @return whether finished or not
 */
bool dynxx_z_unzip_process_finished(void *const unzip);

/**
 * @brief release a unzip process
 * @param unzip The unzip handle
 */
void dynxx_z_unzip_release(void *const unzip);

/**
 * @brief ZIP for C FILE
 * @warning Not accessible in JS/Lua!
 * @param mode ZIP mode, see `DynXXZipCompressMode`
 * @param bufferSize buffer size，must be positive
 * @param format header type, see `DynXXZFormat`
 * @param cFILEIn Input C `FILE`
 * @param cFILEOut Output C `FILE`
 * @return whether finished or not
 */
bool dynxx_z_cfile_zip(int mode, size_t bufferSize, int format, void *const cFILEIn, void *const cFILEOut);

/**
 * @brief UNZIP for C FILE
 * @warning Not accessible in JS/Lua!
 * @param bufferSize buffer size，must be positive
 * @param format header type, see `DynXXZFormat`
 * @param cFILEIn Input C `FILE`
 * @param cFILEOut Output C `FILE`
 * @return whether finished or not
 */
bool dynxx_z_cfile_unzip(size_t bufferSize, int format, void *const cFILEIn, void *const cFILEOut);

/**
 * @brief ZIP for C++ Stream
 * @warning Not accessible in JS/Lua!
 * @param mode ZIP mode, see `DynXXZipCompressMode`
 * @param bufferSize buffer size，must be positive
 * @param format header type, see `DynXXZFormat`
 * @param cxxStreamIn Input C++ Stream(`istream`)
 * @param cxxStreamOut Output C++ Stream(`ostream`)
 * @return whether finished or not
 */
bool dynxx_z_cxxstream_zip(int mode, size_t bufferSize, int format, void *const cxxStreamIn, void *const cxxStreamOut);

/**
 * @brief UNZIP for C++ Stream
 * @warning Not accessible in JS/Lua!
 * @param bufferSize buffer size，must be positive
 * @param format header type, see `DynXXZFormat`
 * @param cxxStreamIn Input C++ Stream(`istream`)
 * @param cxxStreamOut Output C++ Stream(`ostream`)
 * @return whether finished or not
 */
bool dynxx_z_cxxstream_unzip(size_t bufferSize, int format, void *const cxxStreamIn, void *const cxxStreamOut);

/**
 * @brief ZIP for bytes
 * @param mode ZIP mode, see `DynXXZipCompressMode`
 * @param bufferSize buffer size，must be positive
 * @param format header type, see `DynXXZFormat`
 * @param inBytes Input bytes data
 * @param inLen Input bytes length
 * @param outLen A pointer to read output bytes length
 * @return output bytes data
 */
const byte *dynxx_z_bytes_zip(int mode, size_t bufferSize, int format, const byte *inBytes, size_t inLen,
                               size_t *outLen);

/**
 * @brief UNZIP for bytes
 * @param bufferSize buffer size，must be positive
 * @param format header type, see `DynXXZFormat`
 * @param inBytes Input bytes data
 * @param inLen Input bytes length
 * @param outLen A pointer to read output bytes length
 * @return output bytes data
 */
const byte *dynxx_z_bytes_unzip(size_t bufferSize, int format, const byte *inBytes, size_t inLen, size_t *outLen);

EXTERN_C_END

#endif // DYNXX_INCLUDE_ZIP_H_
