#pragma once

#include "Types.h"

DYNXX_EXTERN_C_BEGIN

/**
 * @brief transfer string to upper case
 * @param str string
 * @return upper case string
 */
const char *dynxx_coding_case_upper(const char *str);

/**
 * @brief transfer string to lower case
 * @param str string
 * @return lower case string
 */
const char *dynxx_coding_case_lower(const char *str);

/**
 * @brief transfer byte array to hex string
 * @param inBytes byte array data
 * @param inLen byte array length
 * @return hex string
 */
const char *dynxx_coding_hex_bytes2str(const byte *inBytes, size_t inLen);

/**
 * @brief transfer hex string to byte array
 * @param str hex string
 * @param outLen A pointer to read length of the byte array
 * @return byte array
 */
const byte *dynxx_coding_hex_str2bytes(const char *str, DYNXX_OUT size_t *outLen);

/**
 * @brief transfer byte array to string
 * @param inBytes byte array data
 * @param inLen byte array length
 * @return hex string
 */
const char *dynxx_coding_bytes2str(const byte *inBytes, size_t inLen);

/**
 * @brief transfer string to byte array
 * @param str hex string
 * @param outLen A pointer to read length of the byte array
 * @return byte array
 */
const byte *dynxx_coding_str2bytes(const char *str, DYNXX_OUT size_t *outLen);

/**
 * @brief trim string
 * @param str string
 * @return trimmed string
 */
const char *dynxx_coding_str_trim(const char *str);

DYNXX_EXTERN_C_END
