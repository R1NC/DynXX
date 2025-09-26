#pragma once

#include "Log.h"
#include "Crypto.h"
#include "Coding.h"
#include "Device.h"
#include "Net.h"
#include "KV.h"
#include "SQLite.h"
#include "Json.h"
#include "Zip.h"
#include "Lua.h"
#include "JS.h"

DYNXX_EXTERN_C_BEGIN

/**
 * @brief Read version
 * @return version name
 */
const char *dynxx_get_version();

/**
 * @brief Initialize SDK
 * @warning Not accessible in JS/Lua!
 * @param root Root path to store SDK inner files
 * @return Success or not
 */
bool dynxx_init(const char *root);

/**
 * @brief Read the root path
 * @return the root path
 */
const char *dynxx_root_path();

/**
 * @brief Release SDK
 * @warning Not accessible in JS/Lua!
 */
void dynxx_release(void);

DYNXX_EXTERN_C_END
