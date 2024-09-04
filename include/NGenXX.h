#ifndef NGENXX_H_
#define NGENXX_H_

#include "NGenXXLog.h"
#include "NGenXXNetHttp.h"
#include "NGenxxStoreDB.h"
#ifdef USE_LUA
#include "NGenXXLua.h"
#endif

/**
 * @brief Read version
 * @return version name
 */
const char *ngenxx_get_version(void);

/**
 * @brief Initialize SDK
 * @warning Not accessible in Lua!
 * @return SDK handle
 */
void *ngenxx_init(void);

/**
 * @brief Release SDK
 * @warning Not accessible in Lua!
 * @param handle SDK handle
 */
void ngenxx_release(void *handle);

#endif // NGENXX_H_