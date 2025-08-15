#ifndef DYNXX_INCLUDE_LOG_LOG_H_
#define DYNXX_INCLUDE_LOG_LOG_H_

#include "Types.h"

EXTERN_C_BEGIN

/**
 *
 */
enum DynXXLogLevel {
    DynXXLogLevelDebug = 3,
    DynXXLogLevelInfo,
    DynXXLogLevelWarn,
    DynXXLogLevelError,
    DynXXLogLevelFatal,
    DynXXLogLevelNone
};

/**
 * @brief Set log level
 * @warning Not accessible in JS/Lua!
 * @param level Log level
 */
void dynxx_log_set_level(int level);

/**
 * @brief Set log callback
 * @warning Not accessible in JS/Lua!
 * @param callback log callback
 */
void dynxx_log_set_callback(void (*const callback)(int level, const char *content));

/**
 * @brief Print log
 * @param level Log level
 * @param content Log content
 */
void dynxx_log_print(int level, const char *content);

EXTERN_C_END

#endif // DYNXX_INCLUDE_LOG_LOG_H_
