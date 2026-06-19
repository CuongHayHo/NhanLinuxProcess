/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: modules/process/process_mgr_internal.h
 * Purpose: Internal helper declarations for Process Manager.
 */

#ifndef PROCESS_MGR_INTERNAL_H
#define PROCESS_MGR_INTERNAL_H

#include "process_mgr.h"

/* Static helper prototypes */
static int parse_proc_stat(pid_t pid, proc_info_t* info) __attribute__((unused));
static unsigned long parse_proc_status_memory(pid_t pid) __attribute__((unused));
static void get_proc_cmdline(pid_t pid, char* cmd_out, size_t max_len) __attribute__((unused));

#endif /* PROCESS_MGR_INTERNAL_H */
