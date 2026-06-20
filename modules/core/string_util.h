/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: modules/core/string_util.h
 * Purpose: API declarations for common string utility helper functions.
 */

#ifndef STRING_UTIL_H
#define STRING_UTIL_H

#include "common.h"
#include "error.h"

/**
 * @brief Trim leading and trailing whitespace characters from a string in-place.
 *
 * @param str The null-terminated input string to modify.
 * @return err_code_t ERR_SUCCESS on success, or an appropriate error code.
 */
err_code_t str_trim(char* str);

/**
 * @brief Split a string into multiple tokens based on a delimiter.
 *
 * @param str The input string to split.
 * @param delim The delimiter string.
 * @param tokens Output array of string buffers.
 * @param max_tokens Maximum number of tokens allowed in the output array.
 * @param out_tokens Output pointer containing the number of tokens actually extracted.
 * @return err_code_t ERR_SUCCESS on success, or an appropriate error code.
 */
err_code_t str_split(const char* str, const char* delim, char** tokens, int max_tokens, int* out_tokens);

/**
 * @brief Safely copy a string with boundary checking, guaranteeing null-termination.
 *
 * @param dest The destination buffer.
 * @param src The source null-terminated string.
 * @param dest_size The size of the destination buffer.
 * @return err_code_t ERR_SUCCESS on success, or an appropriate error code.
 */
err_code_t str_safe_copy(char* dest, const char* src, size_t dest_size);

#endif /* STRING_UTIL_H */
