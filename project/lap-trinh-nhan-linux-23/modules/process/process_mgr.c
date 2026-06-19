/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: modules/process/process_mgr.c
 * Purpose: Process management and process lifecycle demonstrations.
 */

#include <stdio.h>
#include <stdlib.h>
#include "process_mgr.h"
#include "logger.h"

void process_mgr_run(void) {
    printf("--- Process Manager (Stub) ---\n");
    printf("1. List Processes\n");
    printf("2. Zombie Process Demo\n");
    printf("3. Orphan Process Demo\n");
    printf("0. Return\n");
    log_info("PROCESS", "Process manager interactive loop started.");
    
    /* TODO: Implement process choice handler mapping to functions */
}

int process_mgr_list(void) {
    log_info("PROCESS", "process_mgr_list called (stub).");
    /* TODO: Traverse /proc directory and print processes */
    return 0;
}

int process_mgr_send_signal(pid_t pid, int sig) {
    (void)pid;
    (void)sig;
    log_info("PROCESS", "process_mgr_send_signal called (stub).");
    /* TODO: Send signal using kill() system call */
    return 0;
}

int process_mgr_adjust_nice(pid_t pid, int increment) {
    (void)pid;
    (void)increment;
    log_info("PROCESS", "process_mgr_adjust_nice called (stub).");
    /* TODO: Set nice value using setpriority() system call */
    return 0;
}

void process_mgr_demo_zombie(void) {
    log_info("PROCESS", "process_mgr_demo_zombie called (stub).");
    /* TODO: Create zombie process using fork() and quick exit child */
    printf("Zombie Process Demo: Stub. (TODO)\n");
}

void process_mgr_demo_orphan(void) {
    log_info("PROCESS", "process_mgr_demo_orphan called (stub).");
    /* TODO: Create orphan process by exiting parent immediately */
    printf("Orphan Process Demo: Stub. (TODO)\n");
}
