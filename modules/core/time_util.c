/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: modules/core/time_util.c
 * Purpose: Empty stub implementations for time utility helper functions.
 */

#include "time_util.h"

err_code_t time_get_current_str(char* buffer, size_t buf_size) {
    (void)buffer;
    (void)buf_size;
    /* Stub: Implement system time retrieval and formatting */
    return ERR_SUCCESS;
}

err_code_t time_calc_duration(uint64_t start_ms, uint64_t end_ms, uint64_t* out_duration) {
    (void)start_ms;
    (void)end_ms;
    if (out_duration) {
        *out_duration = 0;
    }
    /* Stub: Implement millisecond duration arithmetic */
    return ERR_SUCCESS;
}

err_code_t time_get_timestamp_ms(uint64_t* out_ms) {
    if (out_ms) {
        *out_ms = 0;
    }
    /* Stub: Implement clock_gettime clock source wrapper */
    return ERR_SUCCESS;
}
