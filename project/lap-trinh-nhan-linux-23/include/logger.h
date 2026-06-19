/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: include/logger.h
 * Purpose: Synchronous log writer interface.
 */

#ifndef LOGGER_H
#define LOGGER_H

/**
 * @brief Log level classifications.
 */
typedef enum {
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARNING,
    LOG_LEVEL_ERROR
} log_level_t;

/**
 * @brief Writes an informational log entry to the log file.
 * 
 * Purpose: Logs general informational events about the system status.
 *
 * Parameters:
 *   - module: The name of the calling module (e.g. "FILE", "PROCESS").
 *   - format: Printf-style format string.
 *   - ...: Variadic list of arguments matching the format string.
 *
 * Return value: None.
 *
 * System calls used:
 *   - open(): Opens or creates the target log file logs/system.log.
 *   - write(): Writes the formatted log line to the file descriptor.
 *   - close(): Closes the file descriptor.
 */
void log_info(const char* module, const char* format, ...);

/**
 * @brief Writes a warning log entry to the log file.
 * 
 * Purpose: Logs unexpected conditions or non-fatal warning messages.
 *
 * Parameters:
 *   - module: The name of the calling module.
 *   - format: Printf-style format string.
 *   - ...: Variadic list of arguments matching the format string.
 *
 * Return value: None.
 *
 * System calls used:
 *   - open(): Opens or creates the target log file logs/system.log.
 *   - write(): Writes the formatted log line to the file descriptor.
 *   - close(): Closes the file descriptor.
 */
void log_warning(const char* module, const char* format, ...);

/**
 * @brief Writes an error log entry to the log file.
 * 
 * Purpose: Logs critical system errors or failures.
 *
 * Parameters:
 *   - module: The name of the calling module.
 *   - format: Printf-style format string.
 *   - ...: Variadic list of arguments matching the format string.
 *
 * Return value: None.
 *
 * System calls used:
 *   - open(): Opens or creates the target log file logs/system.log.
 *   - write(): Writes the formatted log line to the file descriptor.
 *   - close(): Closes the file descriptor.
 */
void log_error(const char* module, const char* format, ...);

#endif /* LOGGER_H */
