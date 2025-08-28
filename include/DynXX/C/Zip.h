#ifndef DYNXX_INCLUDE_ZIP_H_
#define DYNXX_INCLUDE_ZIP_H_

#include "Types.h"

#include <stdio.h>

EXTERN_C_BEGIN

typedef void* DynXXZipHandle;
typedef void* DynXXUnZipHandle;

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
DynXXZipHandle dynxx_z_zip_init(int mode, size_t bufferSize, int format);

/**
 * @brief input data to ZIP process
 * @param zip The ZIP handle
 * @param inBytes input data bytes
 * @param inLen input data length
 * @param inFinish Whether input is finished or not
 * @return The received data length, return `0` if error occurred
 */
size_t dynxx_z_zip_input(const DynXXZipHandle zip, const byte *inBytes, size_t inLen, bool inFinish);

/**
 * @brief process the ZIP input data
 * @param zip The ZIP handle
 * @param outLen a pointer to read the output data length
 * @return output data bytes, return `nullptr` if error occurred
 */
const byte *dynxx_z_zip_process_do(const DynXXZipHandle zip, size_t *outLen);

/**
 * @brief check whether all the ZIP data inputed before are processed
 * @param zip The ZIP handle
 * @return whether finished or not
 */
bool dynxx_z_zip_process_finished(const DynXXZipHandle zip);

/**
 * @brief release a ZIP process
 * @param zip The ZIP handle
 */
void dynxx_z_zip_release(const DynXXZipHandle zip);

/**
 * @brief initialize a UNZIP process
 * @param bufferSize buffer size，must be positive
 * @param format header type, see `DynXXZFormat`
 * @return a UNZIP handle
 */
DynXXUnZipHandle dynxx_z_unzip_init(size_t bufferSize, int header);

/**
 * @brief input data to UNZIP process
 * @param unzip The UNZIP handle
 * @param inBytes input data bytes
 * @param inLen input data length
 * @param inFinish Whether input is finished or not
 * @return The received data length, return `0` if error occurred
 */
size_t dynxx_z_unzip_input(const DynXXUnZipHandle unzip, const byte *inBytes, size_t inLen, bool inFinish);

/**
 * @brief process the UNZIP input data
 * @param unzip The UNZIP handle
 * @param outLen a pointer to read the output data length
 * @return output data bytes, return `nullptr` if error occurred
 */
const byte *dynxx_z_unzip_process_do(const DynXXUnZipHandle unzip, size_t *outLen);

/**
 * @brief check whether all the ZIP data inputed before are processed
 * @param unzip The UNZIP handle
 * @return whether finished or not
 */
bool dynxx_z_unzip_process_finished(const DynXXUnZipHandle unzip);

/**
 * @brief release a unzip process
 * @param unzip The unzip handle
 */
void dynxx_z_unzip_release(const DynXXUnZipHandle unzip);

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
bool dynxx_z_cfile_zip(int mode, size_t bufferSize, int format, FILE *cFILEIn, FILE *cFILEOut);

/**
 * @brief UNZIP for C FILE
 * @warning Not accessible in JS/Lua!
 * @param bufferSize buffer size，must be positive
 * @param format header type, see `DynXXZFormat`
 * @param cFILEIn Input C `FILE`
 * @param cFILEOut Output C `FILE`
 * @return whether finished or not
 */
bool dynxx_z_cfile_unzip(size_t bufferSize, int format, FILE *cFILEIn, FILE *cFILEOut);

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
