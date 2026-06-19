/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: include/common.h
 * Purpose: Common macros, shared systems libraries, typedefs, and constants.
 */

#ifndef COMMON_H
#define COMMON_H

#include <sys/types.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

/* Boolean helpers */
#ifndef TRUE
#define TRUE  true
#endif
#ifndef FALSE
#define FALSE false
#endif

/* Shared typedefs */
typedef uint8_t byte_t;
typedef int status_t;

/* Common module identifier enum */
typedef enum {
    MODULE_SYSTEM,
    MODULE_FILE,
    MODULE_PROCESS,
    MODULE_SIGNAL,
    MODULE_NETWORK,
    MODULE_SOCKET,
    MODULE_PACKAGE,
    MODULE_SCHEDULER,
    MODULE_SYSTEMINFO,
    MODULE_LOGVIEWER,
    MODULE_CORE,
    MODULE_MAX
} module_id_t;

/* Global constants */
#define MAX_PATH_LEN    256
#define MAX_BUF_LEN     1024
#define MAX_LINE_LEN    512

#endif /* COMMON_H */
