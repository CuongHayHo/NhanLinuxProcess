/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: modules/process/demo/fork_demo.c
 * Purpose: fork() system call demonstration.
 *
 * Documentation:
 *   1. What fork() does:
 *      fork() is a system call in Unix-like operating systems that creates a new process 
 *      (the child process) by duplicating the calling process (the parent process).
 *
 *   2. Why two execution flows exist:
 *      After fork() is called, the operating system creates an exact copy of the process's 
 *      address space, registers, file descriptors, and execution state. Execution resumes 
 *      for both processes at the instruction immediately following the fork() call.
 *      From that point on, both execution flows run concurrently and independently.
 *
 *   3. Meaning of the return value:
 *      - In the parent process, fork() returns the Process ID (PID) of the newly created child.
 *      - In the child process, fork() returns 0.
 *      - If fork() fails, it returns -1, and no child process is created.
 *
 *   4. Difference between parent and child:
 *      - Different PIDs and PPIDs (child's PPID is the parent's PID).
 *      - Different return values from fork().
 *      - Child has its own copy of the parent's memory and file descriptors.
 *
 * ASCII Diagram:
 *
 *       Parent Process
 *             |
 *             | getpid() = Parent PID
 *             |
 *          fork()
 *             |
 *             +-----------------------+
 *             |                       |
 *       Parent Process          Child Process
 *       (Returns Child PID)     (Returns 0)
 *             |                       |
 *             |                       | getpid() = Child PID
 *             v                       v getppid() = Parent PID
 *      Parent continues        Child continues
 *             |                       |
 *             |                       | sleep(2)
 *             |                       v
 *             |                  Child exits
 *             |                       |
 *             v waitpid()             v
 *      Parent resumes <---------------+ (Reaped)
 *             |
 *      Parent finishes
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "process_demo.h"
#include "logger.h"

void fork_demo_run(void) {
    pid_t pid;
    
    printf("\n=== Fork System Call Demonstration ===\n");
    
    /* Log request */
    log_info("PROCESS", "fork() requested");
    
    /* Before fork */
    printf("\n[BEFORE FORK]\n");
    printf("Current PID:  %d\n", getpid());
    printf("Current PPID: %d\n", getppid());
    printf("Creating child process...\n\n");
    fflush(stdout);
    
    pid = fork();
    
    if (pid < 0) {
        /* Error handling */
        perror("fork failed");
        log_error("PROCESS", "fork() failed");
        return;
    }
    
    if (pid == 0) {
        /* Child Process */
        log_info("PROCESS", "Child started");
        
        printf("[CHILD PROCESS]\n");
        printf("PID:               %d\n", getpid());
        printf("PPID (Parent PID): %d\n", getppid());
        printf("fork() return val: %d\n", pid);
        printf("Child continues running for 2 seconds...\n\n");
        fflush(stdout);
        
        sleep(2);
        
        printf("[CHILD PROCESS] Child exiting...\n");
        fflush(stdout);
        
        log_info("PROCESS", "Child exited");
        exit(0);
    } else {
        /* Parent Process */
        log_info("PROCESS", "Parent created child");
        
        printf("[PARENT PROCESS]\n");
        printf("PID:               %d\n", getpid());
        printf("PPID (Parent PID): %d\n", getppid());
        printf("fork() return val (Child PID): %d\n", pid);
        printf("Parent continues running...\n\n");
        fflush(stdout);
        
        /* Wait for child to exit to prevent zombie and clean up */
        int status;
        printf("[PARENT PROCESS] Waiting for child %d to exit...\n", pid);
        fflush(stdout);
        
        if (waitpid(pid, &status, 0) > 0) {
            log_info("PROCESS", "Parent resumed");
            printf("\n[PARENT PROCESS] Parent resumed after child exit.\n");
        } else {
            log_error("PROCESS", "Parent failed waiting for child");
        }
        
        printf("[PARENT PROCESS] Parent finishes.\n");
        fflush(stdout);
    }
}
