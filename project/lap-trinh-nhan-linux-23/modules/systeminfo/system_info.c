/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: modules/systeminfo/system_info.c
 * Purpose: Diagnostic host metrics systeminfo module.
 */

#include <stdio.h>
#include <stdlib.h>
#include "system_info.h"
#include "logger.h"

void system_info_run(void) {
    printf("--- System Diagnostic Metrics (Stub) ---\n");
    system_info_fetch_uname();
    system_info_fetch_stats();
    log_info("SYSTEM_INFO", "System diagnostics printed (stub).");
}

int system_info_fetch_uname(void) {
    log_info("SYSTEM_INFO", "system_info_fetch_uname called (stub).");
    /* TODO: Call uname() and output kernel version, host, and arch */
    printf("OS Kernel: Stub Linux (TODO)\n");
    return 0;
}

int system_info_fetch_stats(void) {
    log_info("SYSTEM_INFO", "system_info_fetch_stats called (stub).");
    /* TODO: Call sysinfo() and query CPU details from /proc/cpuinfo */
    printf("RAM/Uptime Metrics: Stub (TODO)\n");
    return 0;
}
