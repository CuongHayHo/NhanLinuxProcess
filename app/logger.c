/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: app/logger.c
 * Purpose: Synchronous logging implementation using raw system calls.
 * 
 * Thread Safety Note:
 * This synchronous logger implementation is functionally thread-safe under Linux
 * for standard log message lengths. 
 * Since every log write opens the file descriptor with the 'O_APPEND' flag, performs 
 * a single atomic write() system call, and immediately invokes close(), the Linux kernel 
 * ensures that writes from different threads or processes are appended atomically 
 * without interleaving or corrupting the file offset.
 */

#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "logger.h"
#include "config.h"

/**
 * @brief Appends a formatted message to the log file using raw system calls.
 * 
 * System Calls Used:
 *   - open(): Opens the log file in write-only, append mode. Creates it if missing.
 *   - write(): Performs the write to the file descriptor.
 *   - close(): Closes the file descriptor.
 *
 * Error Handling:
 *   - If open() fails (returns < 0), the function returns immediately.
 *   - If write() fails, close() is called to prevent file descriptor leaks, and the function returns.
 */
static void append_log(const char* file_path, const char* prefix, const char* module, const char* message) {
    int fd;
    time_t raw_time;
    struct tm time_info;
    char time_buffer[32];
    char log_line[2048];
    ssize_t message_len;
    ssize_t written;

    /* Get local time safely using thread-safe localtime_r */
    if (time(&raw_time) == (time_t)-1) {
        return;
    }
    if (localtime_r(&raw_time, &time_info) == NULL) {
        return;
    }
    if (strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", &time_info) == 0) {
        return;
    }

    /* Format the log line exactly: YYYY-MM-DD HH:MM:SS [LEVEL] [MODULE] MESSAGE\n */
    message_len = snprintf(log_line, sizeof(log_line), "%s [%s] [%s] %s\n", 
                           time_buffer, prefix, module, message);
    if (message_len < 0 || (size_t)message_len >= sizeof(log_line)) {
        return; /* Formatting error or truncation */
    }

    /* Open the log file using POSIX system call (Permissions: rw-r--r--) */
    fd = open(file_path, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd < 0) {
        return; /* Open failed, exit immediately */
    }

    /* Write log line to the file descriptor */
    written = write(fd, log_line, message_len);
    if (written < 0) {
        close(fd); /* Write failed, close file descriptor to prevent leak */
        return;
    }

    /* Close the file descriptor */
    close(fd);
}

void log_info(const char* module, const char* format, ...) {
    char message[1024];
    va_list args;
    int len;
    
    va_start(args, format);
    len = vsnprintf(message, sizeof(message), format, args);
    va_end(args);

    if (len >= 0) {
        append_log(LOG_PATH_SYSTEM, "INFO", module, message);
    }
}

void log_warning(const char* module, const char* format, ...) {
    char message[1024];
    va_list args;
    int len;
    
    va_start(args, format);
    len = vsnprintf(message, sizeof(message), format, args);
    va_end(args);

    if (len >= 0) {
        append_log(LOG_PATH_SYSTEM, "WARN", module, message);
    }
}

void log_error(const char* module, const char* format, ...) {
    char message[1024];
    va_list args;
    int len;
    
    va_start(args, format);
    len = vsnprintf(message, sizeof(message), format, args);
    va_end(args);

    if (len >= 0) {
        append_log(LOG_PATH_SYSTEM, "ERROR", module, message);
    }
}
