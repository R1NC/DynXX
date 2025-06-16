#ifndef NGENXX_INCLUDE_H_
#define NGENXX_INCLUDE_H_

#ifndef NOMINMAX
#define NOMINMAX  // prevent windows redefining min/max
#endif

#include "NGenXXLog.h"
#include "NGenXXCrypto.h"
#include "NGenXXCoding.h"
#include "NGenXXDevice.h"
#include "NGenXXNetHttp.h"
#include "NGenXXStoreKV.h"
#include "NGenXXStoreSQLite.h"
#include "NGenXXJson.h"
#include "NGenXXZip.h"
#include "NGenXXLua.h"
#include "NGenXXJS.h"

EXTERN_C_BEGIN

/**
 * @brief Read version
 * @return version name
 */
const char *ngenxx_get_version();

/**
 * @brief Initialize SDK
 * @warning Not accessible in JS/Lua!
 * @param root Root path to store SDK inner files
 * @return Success or not
 */
bool ngenxx_init(const char *root);

/**
 * @brief Read the root path
 * @return the root path
 */
const char *ngenxx_root_path();

/**
 * @brief Release SDK
 * @warning Not accessible in JS/Lua!
 */
void ngenxx_release(void);

EXTERN_C_END

#endif // NGENXX_INCLUDE_H_
