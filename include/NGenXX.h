#ifndef NGENXX_H_
#define NGENXX_H_

#include "NGenXXTypes.h"
#include "NGenXXLog.h"
#include "NGenXXCrypto.h"
#include "NGenXXCoding.h"
#include "NGenXXDeviceInfo.h"
#include "NGenXXNetHttp.h"
#include "NGenXXStoreKV.h"
#include "NGenXXStoreSQLite.h"
#include "NGenXXJsonDecoder.h"
#include "NGenXXZip.h"
#include "NGenXXLua.h"

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
 * @return Success or not
 */
bool ngenxx_init(const char *root);

/**
 * @brief Release SDK
 * @warning Not accessible in Lua!
 */
void ngenxx_release(void);

#ifdef  __cplusplus
}
#endif

#endif // NGENXX_H_