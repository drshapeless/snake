#ifndef LOGGER_H
#define LOGGER_H

typedef enum log_level {

    LOG_LEVEL_FATAL = 0,
    LOG_LEVEL_ERROR = 1,
    LOG_LEVEL_WARN = 2,
    LOG_LEVEL_INFO = 3,
    LOG_LEVEL_DEBUG = 4,
    LOG_LEVEL_TRACE = 5
} log_level;

void log_output(log_level level, const char *message, ...);

#define FATAL(message, ...) log_output(LOG_LEVEL_FATAL, message, ##__VA_ARGS__);
#define ERROR(message, ...) log_output(LOG_LEVEL_ERROR, message, ##__VA_ARGS__);
#define WARN(message, ...) log_output(LOG_LEVEL_WARN, message, ##__VA_ARGS__);
#define INFO(message, ...) log_output(LOG_LEVEL_INFO, message, ##__VA_ARGS__);
#define DEBUG(message, ...) log_output(LOG_LEVEL_DEBUG, message, ##__VA_ARGS__);
#define TRACE(message, ...) log_output(LOG_LEVEL_TRACE, message, ##__VA_ARGS__);

#endif /* LOGGER_H */
