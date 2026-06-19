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
int process_mgr_send_signal(pid_t pid, int sig);
int process_mgr_adjust_nice(pid_t pid, int increment);
void process_mgr_demo_zombie(void);
void process_mgr_demo_orphan(void);

#endif /* PROCESS_MGR_H */
