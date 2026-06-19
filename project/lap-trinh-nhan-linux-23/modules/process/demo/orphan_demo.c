/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: modules/process/demo/orphan_demo.c
 * Purpose: Orphan process creation and adoption demonstration.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include "process_demo.h"
#include "logger.h"

void orphan_demo_run(void) {
    log_info("PROCESS", "Orphan demo started");

    printf("\n=== Orphan Process Demonstration ===\n");
    printf("Theory:\n");
    printf("  1. An Orphan Process is a running child process whose parent process\n");
    printf("     has terminated before the child itself exits.\n");
    printf("  2. In Linux, every process must belong to a parent process. When a parent exits,\n");
    printf("     the kernel automatically re-parents the orphan child.\n");
    printf("  3. The child is adopted by the system's initialization daemon (typically init\n");
    printf("     with PID 1, or systemd-user-services in modern user sessions).\n");
    printf("  4. The adopted process takes responsibility for reaping the child when it terminates.\n\n");

    printf("[Grandparent] Spawning the Parent process...\n");
    pid_t parent_pid = fork();

    if (parent_pid < 0) {
        log_error("PROCESS", "Fork failed for parent in Orphan demo (errno %d)", errno);
        printf("Error: Fork failed!\n");
        log_info("PROCESS", "Demo finished");
        return;
    }

    if (parent_pid == 0) {
        /* Parent Process ('P') */
        pid_t my_pid = getpid();
        printf("Parent PID:    %d\n", my_pid);
        
        printf("[Parent] Forking the Child process...\n");
        pid_t child_pid = fork();

        if (child_pid < 0) {
            log_error("PROCESS", "Fork failed for child in Orphan demo (errno %d)", errno);
            printf("[Parent] Error: Fork failed!\n");
            exit(1);
        }

        if (child_pid == 0) {
            /* Child Process ('C') */
            pid_t my_child_pid = getpid();
            pid_t original_ppid = getppid();
            printf("Child PID:     %d\n", my_child_pid);
            printf("Original PPID: %d\n", original_ppid);
            printf("  [Child] Sleeping for 4 seconds to wait for parent to exit...\n");
            
            /* Robust sleep loop handling signal interruptions */
            unsigned int seconds_left = 4;
            while (seconds_left > 0) {
                seconds_left = sleep(seconds_left);
            }
            
            pid_t new_ppid = getppid();
            printf("Current PID:   %d\n", getpid());
            printf("Current PPID:  %d\n", new_ppid);
            printf("New PPID:      %d\n", new_ppid);
            
            if (new_ppid != original_ppid) {
                log_info("PROCESS", "Child adopted");
                printf("Adoption detected\n");
                printf("  Explanation: The child has been adopted by init/systemd (PID %d).\n", new_ppid);
            } else {
                log_error("PROCESS", "Unexpected PPID in Orphan demo: PPID remains %d", new_ppid);
                printf("Error: Adoption not detected! PPID is still %d.\n", new_ppid);
            }
            
            exit(0);
        }

        /* Parent Process exits immediately to orphan the child */
        printf("[Parent] Exiting immediately to orphan the child...\n");
        log_info("PROCESS", "Parent exited");
        exit(0);
    }

    /* Grandparent Process ('G' - the test runner) */
    /* Wait for the Parent process 'P' to exit */
    int status;
    if (waitpid(parent_pid, &status, 0) < 0) {
        log_error("PROCESS", "waitpid() on Parent failed (errno %d)", errno);
    } else {
        printf("[Grandparent] Parent process (PID %d) exited.\n", parent_pid);
    }

    printf("[Grandparent] Waiting 6 seconds for Grandchild (Orphan) to finish printing...\n");
    
    /* Robust sleep loop for Grandparent */
    unsigned int seconds_left = 6;
    while (seconds_left > 0) {
        seconds_left = sleep(seconds_left);
    }

    log_info("PROCESS", "Demo finished");
    printf("====================================\n");
}
