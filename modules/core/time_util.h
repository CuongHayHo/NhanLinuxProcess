/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: modules/core/time_util.h
 * Purpose: API declarations for common time utility helper functions.
 */

#ifndef TIME_UTIL_H
#define TIME_UTIL_H

#include "common.h"
#include "error.h"

/**
 * @brief Get the current local system time formatted as a string (e.g. "YYYY-MM-DD HH:MM:SS").
 *
 * @param buffer The destination buffer to store the formatted time.
 * @param buf_size The size of the destination buffer.
 * @return err_code_t ERR_SUCCESS on success, or an appropriate error code.
 */
err_code_t time_get_current_str(char* buffer, size_t buf_size);

/**
 * @brief Calculate the difference in milliseconds between two timestamps.
 *
 * @param start_ms Start timestamp in milliseconds.
 * @param end_ms End timestamp in milliseconds.
 * @param out_duration Output pointer containing the duration difference.
 * @return err_code_t ERR_SUCCESS on success, or an appropriate error code.
 */
err_code_t time_calc_duration(uint64_t start_ms, uint64_t end_ms, uint64_t* out_duration);

/**
 * @brief Fetch the high-resolution system timestamp in milliseconds.
 *
 * @param out_ms Output pointer to receive the current timestamp.
 * @return err_code_t ERR_SUCCESS on success, or an appropriate error code.
 */
err_code_t time_get_timestamp_ms(uint64_t* out_ms);

#endif /* TIME_UTIL_H */
