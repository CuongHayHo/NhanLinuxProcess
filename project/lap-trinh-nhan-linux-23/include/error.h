/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: include/error.h
 * Purpose: Unified project error code definitions and helper functions.
 */

#ifndef ERROR_H
#define ERROR_H

typedef enum {
    ERR_SUCCESS = 0,       /* Success / no error */
    ERR_FAILURE = -1,      /* Generic failure */
    ERR_INVALID_ARG = -2,  /* Invalid argument provided */
    ERR_NULL_PTR = -3,     /* Null pointer encountered */
    ERR_OUT_OF_MEM = -4,   /* Memory allocation failed */
    ERR_PERMISSION = -5,   /* Permission denied */
    ERR_NOT_FOUND = -6,    /* File or resource not found */
    ERR_TIMEOUT = -7,      /* Operation timeout */
    ERR_BUSY = -8,         /* Resource busy */
    ERR_IO = -9,           /* Low-level Input/Output error */
    ERR_OVERFLOW = -10,    /* Buffer or numeric overflow */
    ERR_NOT_SUPPORTED = -11 /* Operation not supported */
} err_code_t;

/* Helper to convert error code to human-readable string representation */
static inline const char* err_to_string(err_code_t code) {
    switch (code) {
        case ERR_SUCCESS:       return "Success";
        case ERR_FAILURE:       return "Generic failure";
        case ERR_INVALID_ARG:   return "Invalid argument";
        case ERR_NULL_PTR:      return "Null pointer";
        case ERR_OUT_OF_MEM:    return "Out of memory";
        case ERR_PERMISSION:    return "Permission denied";
        case ERR_NOT_FOUND:     return "Not found";
        case ERR_TIMEOUT:       return "Timeout";
        case ERR_BUSY:          return "Busy";
        case ERR_IO:            return "I/O error";
        case ERR_OVERFLOW:      return "Overflow";
        case ERR_NOT_SUPPORTED: return "Not supported";
        default:                return "Unknown error";
    }
}

#endif /* ERROR_H */
