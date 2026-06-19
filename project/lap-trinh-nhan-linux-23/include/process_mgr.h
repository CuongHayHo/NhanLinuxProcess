/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: include/process_mgr.h
 * Purpose: Process diagnostics and lifecycle demos.
 */

#ifndef PROCESS_MGR_H
#define PROCESS_MGR_H

#include "common.h"

typedef struct {
    pid_t pid;
    pid_t ppid;
    char name[256];
    char state;
    long priority;
    unsigned long memory_size;
} proc_info_t;

void process_mgr_run(void);
int process_mgr_list(void);
int process_mgr_search(const char* query);
int process_mgr_send_signal(pid_t pid, int sig);
int process_mgr_set_priority(pid_t pid, int nice_val);


void signal_mgr_init(void);
void signal_mgr_restore_defaults(void);

#endif /* PROCESS_MGR_H */
