#ifndef ENGINEXX_LOG_LOG_H_
#define ENGINEXX_LOG_LOG_H_

/**
 * 
 */
enum EngineXXLogLevel {
    Info,
    Debug,
    Warn,
    Error
};

/**
 * @brief Set log level
 * @warning Not accessible in Lua!
 * @param level Log level
 */
void enginexx_log_set_level(int level);

/**
 * @brief Set log callback
 * @warning Not accessible in Lua!
 * @param callback log callback
 */
void enginexx_log_set_callback(void (*callback)(int level, const char *log));

/**
 * @brief Print log
 * @param level Log level
 * @param content Log content
 */
void enginexx_log_print(int level, const char *content);

#endif // ENGINEXX_LOG_LOG_H_