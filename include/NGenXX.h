#ifndef NGENXX_H_
#define NGENXX_H_

#include "NGenXXLog.h"
#include "NGenXXNetHttp.h"
#include "NGenXXStoreSQLite.h"
#include "NGenXXLua.h"

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
 * @param sdk SDK handle
 */
void ngenxx_release(void *sdk);

#endif // NGENXX_H_