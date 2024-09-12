#ifndef NGENXX_H_
#define NGENXX_H_

#include "NGenXXLog.h"
#include "NGenXXDeviceInfo.h"
#include "NGenXXNetHttp.h"
#include "NGenXXStoreKV.h"
#include "NGenXXStoreSQLite.h"
#include "NGenXXLua.h"
#include "NGenXXJs.h"

#ifdef  __cplusplus
extern "C" {
#endif

/**
 * @brief Read version
 * @return version name
 */
const char *ngenxx_get_version(void);

/**
 * @brief Initialize SDK
 * @warning Not accessible in Lua!
 * @param root Root path to store SDK inner files
 * @return SDK handle
 */
void *ngenxx_init(const char *root);

/**
 * @brief Release SDK
 * @warning Not accessible in Lua!
 * @param sdk SDK handle
 */
void ngenxx_release(void *sdk);

#ifdef  __cplusplus
}
#endif

#endif // NGENXX_H_