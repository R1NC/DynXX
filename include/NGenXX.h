#ifndef NGENXX_H_
#define NGENXX_H_

#include "NGenXXLog.h"
#include "NGenXXNet.h"
#include "NGenXXLua.h"

/**
 * Initialize SDK
 * @warning Not accessible in Lua!
 */
void* ngenxx_init(void);

/**
 * @brief Read version
 * @return version name
 */
const char *ngenxx_get_version(void);

/**
 * Release SDK
 * @warning Not accessible in Lua!
 */
void ngenxx_release(void);

#endif // NGENXX_H_