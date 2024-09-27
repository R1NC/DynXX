#ifndef NGENXX_CODING_H_
#define NGENXX_CODING_H_

#include "NGenXXTypes.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief transfer byte array to hex string
 * @param inBytes byte array data
 * @param inLen byte array length
 * @return hex string
 */
const char *ngenxx_coding_hex_bytes2str(const byte *inBytes, const size inLen);

/**
 * @brief transfer hex string to byte array
 * @param str hex string
 * @param outLen A pointer to read length of the byte array
 * @return byte array
 */
const byte *ngenxx_coding_hex_str2bytes(const char *str, size *outLen);

#ifdef __cplusplus
}
#endif

#endif // NGENXX_CODING_H_