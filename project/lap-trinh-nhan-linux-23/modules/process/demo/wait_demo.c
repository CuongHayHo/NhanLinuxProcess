/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: modules/process/demo/wait_demo.c
 * Purpose: wait() and waitpid() system calls demonstration.
 *
 * Documentation:
 *   1. Why wait() blocks:
 *      wait() blocks the calling process (the parent) until one of its child processes 
 *      terminates. If a child is already dead (a zombie), wait() returns immediately 
 *      with the child's status information. Otherwise, the parent is placed in a blocked 
 *      state, yielding the CPU until a child state transition occurs.
 *
 *   2. Difference between wait() and waitpid():
 *      - wait() waits for ANY child process to exit. It takes a status pointer argument.
 *      - waitpid() allows the parent to wait for a SPECIFIC child process by passing its PID. 
 *        It also supports option flags, such as WNOHANG (to return immediately if no child 
 *        has exited, rather than blocking).
 *
 *   3. Why wait() prevents zombie processes:
 *      When a child process exits, its memory and other resources are freed, but its exit code 
 *      and PID are kept in the process table. This process remains as a "zombie". 
 *      Calling wait() retrieves this exit information, allowing the kernel to fully clean up 
 *      the process entry and free the PID.
 *
 * ASCII Diagram:
 *
 *       Parent Process (PID: X)
 *             |
 *          fork()
 *             |
 *             +-----------------------------+
 *             |                             |
 *       Parent Process (PID: X)       Child Process (PID: Y)
 *             |                             |
 *             |                             | sleep(3) [sleeping...]
 *             v wait()                      v
 *          BLOCKED                          v
 *             |                             v [exiting...]
 *             |                             v exit(42)
 *             |                             v (becomes Zombie)
 *             v <---------------------------+ (state change wakes parent)
 *       Parent resumes
 *             |
 *       Collects status (42)
 *       Parent finishes
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include "process_demo.h"
#include "logger.h"

void wait_demo_run(void) {
    pid_t pid;
    
    printf("\n=== Wait/Waitpid System Call Demonstration ===\n");
    
    /* Display Theory */
    printf("\n--- Theory of wait() and waitpid() ---\n");
    printf("1. wait() blocks the parent until any child terminates.\n");
    printf("2. waitpid() lets the parent block for a specific child PID, or run non-blockingly (WNOHANG).\n");
    printf("3. Waking up from wait() collects child resources, preventing Zombie accumulation.\n");
    
    printf("\n[PARENT PROCESS] Parent PID: %d\n", getpid());
    printf("[PARENT PROCESS] Forking child process...\n");
    fflush(stdout);
    
    pid = fork();
    
    if (pid < 0) {
        perror("fork failed");
        log_error("PROCESS", "wait demonstration: fork failed");
        return;
    }
    
    if (pid == 0) {
        /* Child Process */
        printf("\n[CHILD PROCESS] Child started. PID: %d, Parent PID (PPID): %d\n", getpid(), getppid());
        printf("[CHILD PROCESS] Child sleeping for 3 seconds...\n");
        fflush(stdout);
        
        log_info("PROCESS", "Child sleeping");
        sleep(3);
        
        printf("[CHILD PROCESS] Child exiting... status code 42.\n");
        fflush(stdout);
        
        log_info("PROCESS", "Child exited");
        exit(42);
    } else {
        /* Parent Process */
        printf("\n[PARENT PROCESS] Spawned child PID: %d\n", pid);
        printf("[PARENT PROCESS] Parent enters wait() (will block until child exits)...\n");
        fflush(stdout);
        
        log_info("PROCESS", "Parent waiting");
        
        int status;
        pid_t waited_pid = wait(&status);
        
        if (waited_pid < 0) {
            perror("wait failed");
            log_error("PROCESS", "wait demonstration: wait failed");
            return;
        }
        
        log_info("PROCESS", "Parent resumed");
        
        printf("\n[PARENT PROCESS] Parent resumed. Waited PID: %d\n", waited_pid);
        
        if (WIFEXITED(status)) {
            printf("[PARENT PROCESS] Child exit status: %d (collected successfully)\n", WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("[PARENT PROCESS] Child killed by signal: %d\n", WTERMSIG(status));
        } else {
            printf("[PARENT PROCESS] Child exited under other conditions.\n");
        }
        
        printf("[PARENT PROCESS] Parent finishes.\n");
        fflush(stdout);
    }
}
