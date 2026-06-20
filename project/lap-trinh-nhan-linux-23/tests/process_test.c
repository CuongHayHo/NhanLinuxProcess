/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: tests/process_test.c
 * Purpose: Complete Diagnostic tests and demonstrations for Process Manager - Sprints 1, 2, & 3.
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "process_mgr.h"

/* Dummy definitions for CLI and TUI subsystems to satisfy links in process_mgr.o */
int is_interactive = 0;
int ui_select_menu(const char* title, const char* options[], int options_count) {
    (void)title; (void)options; (void)options_count;
    return -1;
}
void print_prompt_explanation(const char* prompt_msg) {
    (void)prompt_msg;
}

int main(void) {
    char current_pid_str[32];
    
    printf("Starting Process Manager - Sprint 3 (Process Control) test program...\n");
    printf("My PID: %d, My Parent PID: %d\n\n", getpid(), getppid());

    /* 1. Test Process List (Sprint 1) */
    printf("--- Test 1: Listing All Processes ---\n");
    process_mgr_list();

    /* 2. Test Process Search by Name: "process_test" (This application) */
    printf("\n--- Test 2: Searching Process by Name ('process_test') ---\n");
    process_mgr_search("process_test");

    /* 3. Test Process Search by PID: current PID */
    printf("\n--- Test 3: Searching Process by current PID (%d) ---\n", getpid());
    snprintf(current_pid_str, sizeof(current_pid_str), "%d", getpid());
    process_mgr_search(current_pid_str);

    /* 4. Process Control Tests (Sprint 3) using a child process */
    printf("\n--- Test 4: Process Control (Signal & Priority) with Background Process ---\n");
    pid_t child_pid = fork();
    if (child_pid < 0) {
        perror("fork failed");
        return 1;
    }

    if (child_pid == 0) {
        /* Child process: just sleep in a loop */
        while (1) {
            sleep(1);
        }
        exit(0);
    }

    printf("Spawned background child process with PID: %d\n\n", child_pid);
    sleep(1); /* Allow child process to start and register in proc */

    /* 4a. Verify priority decrease (should succeed) */
    printf("=> Attempting to decrease priority (nice value +5):\n");
    process_mgr_set_priority(child_pid, 5);
    
    /* 4b. Verify priority increase (should fail with EPERM for non-root) */
    printf("\n=> Attempting to increase priority (nice value -5, expected to fail/EPERM):\n");
    process_mgr_set_priority(child_pid, -5);

    /* 4c. Verify SIGSTOP */
    printf("\n=> Sending SIGSTOP to child:\n");
    process_mgr_send_signal(child_pid, SIGSTOP);
    sleep(1);

    /* 4d. Verify SIGCONT */
    printf("\n=> Sending SIGCONT to child:\n");
    process_mgr_send_signal(child_pid, SIGCONT);
    sleep(1);

    /* 4e. Verify SIGTERM */
    printf("\n=> Sending SIGTERM to child:\n");
    process_mgr_send_signal(child_pid, SIGTERM);
    
    /* Reap child */
    int status;
    waitpid(child_pid, &status, 0);
    printf("Child process %d reaped.\n", child_pid);

    /* 5. Error Handling Tests */
    printf("\n--- Test 5: Error Handling (Invalid and Non-Existent PIDs) ---\n");
    
    printf("=> Signal to negative PID (-999):\n");
    process_mgr_send_signal(-999, SIGTERM);

    printf("\n=> Signal to non-existent PID (999999):\n");
    process_mgr_send_signal(999999, SIGTERM);

    printf("\n=> Priority change to negative PID (-999):\n");
    process_mgr_set_priority(-999, 10);

    printf("\n=> Priority change to non-existent PID (999999):\n");
    process_mgr_set_priority(999999, 10);

    /* 6. Test Fork Demonstration (Sprint 4A) */
    printf("\n--- Test 6: Running Fork Demonstration ---\n");
    void fork_demo_run(void);
    fork_demo_run();

    /* 7. Test Exec Demonstration (Sprint 4B) */
    printf("\n--- Test 7: Running Exec Demonstration ---\n");
    void exec_demo_run(void);
    exec_demo_run();

    /* 8. Test Wait Demonstration (Sprint 4C) */
    printf("\n--- Test 8: Running Wait/Waitpid Demonstration ---\n");
    void wait_demo_run(void);
    wait_demo_run();

    /* 9. Test Zombie Demonstration (Sprint 4D) */
    printf("\n--- Test 9: Running Zombie Process Demonstration ---\n");
    void zombie_demo_run(void);
    zombie_demo_run();

    /* 10. Test Orphan Demonstration (Sprint 4E) */
    printf("\n--- Test 10: Running Orphan Process Demonstration ---\n");
    void orphan_demo_run(void);
    orphan_demo_run();

    /* 11. Test Daemon Demonstration (Sprint 4F) */
    printf("\n--- Test 11: Running Daemon Process Demonstration ---\n");
    void daemon_demo_run(void);
    daemon_demo_run();

    printf("\nProcess Manager tests completed successfully.\n");
    return 0;
}
