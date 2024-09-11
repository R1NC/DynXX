#ifndef NGENXX_LOG_LOG_H_
#define NGENXX_LOG_LOG_H_

/**
 * 
 */
enum NGenXXLogLevel {
    NGenXXLogLevelInfo,
    NGenXXLogLevelDebug,
    NGenXXLogLevelWarn,
    NGenXXLogLevelError,
    NGenXXLogLevelNone
};

/**
 * @brief Set log level
 * @warning Not accessible in Lua!
 * @param level Log level
 */
void ngenxx_log_set_level(int level);

/**
 * @brief Set log callback
 * @warning Not accessible in Lua!
 * @param callback log callback
 */
void ngenxx_log_set_callback(void (*callback)(int level, const char *log));

/**
 * @brief Print log
 * @param level Log level
 * @param content Log content
 */
void ngenxx_log_print(int level, const char *content);

#endif // NGENXX_LOG_LOG_H_