#include "../logger.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_LOGGERS 16

typedef struct {
    FILE* fp;
    log_level level;
} Logger;  // loggers instance

static struct {
    log_level level;
    Logger loggers[MAX_LOGGERS];
    size_t loggers_count;
    int log_IO_interaction;
} L = {LOG_INFO, {{NULL, LOG_INFO}}, 0, 0};

static const char* level_string[] = {"IO",   "TRACE", "DEBUG", "INFO",
                                     "WARN", "ERROR", "FATAL"};

static void log_to_stream(FILE* stream, log_level level, const char* file,
                          int line, const char* fmt, va_list ap) {
    char time_buf[16];
    time_t t = time(NULL);
    strftime(time_buf, sizeof(time_buf), "%H:%M:%S",
             localtime(&t));  // Format time
    fprintf(stream, "%s %-5s %s:%d: ", time_buf, level_string[level], file,
            line);  // Prints time and log state
    if (level == LOG_IO) {
        fprintf(stream, "\n\n");
    }
    vfprintf(stream, fmt, ap);  // Print user-defined data
    fprintf(stream, "\n");
    fflush(stream);
}

void log_set_level(log_level level) { L.level = level; }
void log_set_user_interaction(int enable) { L.log_IO_interaction = enable; }

err_t log_add_fp(FILE* fp, log_level level) {
    if (fp == NULL) {
        fprintf(stderr,
                "Error: Passed file pointer is NULL. "
                "Aborting adding new "
                "logger.\n");
        return DEREFERENCING_NULL_PTR;
    }

    if (L.loggers_count >= MAX_LOGGERS) {
        fprintf(stderr,
                "Error: Reached max logger count. "
                "Aborting adding new logger.\n");
        return ERROR_MAX_LOGGER_COUNT_REACHED;
    }

    L.loggers[L.loggers_count++] = (Logger){fp, level};
    return EXIT_SUCCESS;
}

void log_log(log_level level, const char* file, int line, const char* fmt,
             ...) {
    if (fmt == NULL) {
        fprintf(stderr, "Warning: fmt passed to log is NULL.\n");
        return;
    }

    if (level < L.level && level != LOG_IO) {
        return;  // Skip logs below the global level
    }

    va_list ap;
    va_list ap_cpy;
    va_start(ap, fmt);

    // Log to all logger instances
    for (size_t i = 0; i < L.loggers_count; ++i) {
        if ((level >= L.loggers[i].level) ||
            ((level == LOG_IO && L.log_IO_interaction != 0) &&
             (L.loggers[i].fp != stdout) && (L.loggers[i].fp != stderr))) {
            va_copy(ap_cpy, ap);
            log_to_stream(L.loggers[i].fp, level, file, line, fmt, ap_cpy);
            va_end(ap_cpy);  // Clean up copied va_list
        }
    }

    va_end(ap);
}

void vlog_log(log_level level, const char* file, int line, const char* fmt,
              va_list ap) {  // copy of prev. function but with va_list ap
                             // instead of "..." (for logprintf())
    if (fmt == NULL) {
        fprintf(stderr, "Warning: fmt passed to log is NULL.\n");
        return;
    }

    if (level < L.level && level != LOG_IO) {
        return;  // Skip logs below the global level
    }

    va_list ap_cpy;

    // Log to all logger instances
    for (size_t i = 0; i < L.loggers_count; ++i) {
        if ((level >= L.loggers[i].level) ||
            ((level == LOG_IO && L.log_IO_interaction != 0) &&
             (L.loggers[i].fp != stdout) && (L.loggers[i].fp != stderr))) {
            va_copy(ap_cpy, ap);
            log_to_stream(L.loggers[i].fp, level, file, line, fmt, ap_cpy);
            va_end(ap_cpy);  // Clean up copied va_list
        }
    }

    va_end(ap);
}

err_t logger_start() {
    err_t err = 0;
    err = log_add_fp(stdout, LOG_TRACE);
    if (err) {
        return err;
    }
    return EXIT_SUCCESS;
}
