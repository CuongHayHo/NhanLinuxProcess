/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: include/system_info.h
 * Purpose: Host machine diagnostics statistics.
 */

#ifndef SYSTEM_INFO_H
#define SYSTEM_INFO_H

#include "common.h"

void system_info_run(void);
int system_info_fetch_uname(void);
int system_info_fetch_stats(void);

int system_info_kernel(void);
int system_info_hostname(void);
int system_info_os(void);
int system_info_architecture(void);

#endif /* SYSTEM_INFO_H */
