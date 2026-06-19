/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: modules/process/demo/exec_demo.c
 * Purpose: exec() system call family demonstration.
 *
 * Documentation:
 *   1. What exec() does:
 *      The exec() family of functions replaces the current process image with a new process image.
 *      It loads a binary executable into the process's address space, overwriting its segments
 *      (code, data, heap, and stack) and starts execution at the entry point of the new program.
 *
 *   2. Why code after successful exec() is never executed:
 *      Since the entire memory image of the process is completely overwritten by the new program,
 *      none of the old program's code remains. Therefore, if exec() succeeds, it never returns.
 *      Any code written after an exec() call will only run if the exec() call fails.
 *
 *   3. Difference between fork() and exec():
 *      - fork() creates a duplicate of the calling process, resulting in TWO concurrent processes
 *        running the same code.
 *      - exec() does NOT create a new process; it merely changes the program that the CURRENT process
 *        is running. It replaces the execution image of the existing process.
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
 *             |                             | (Runs current code)
 *             |                             v
 *             |                          execvp()
 *             |                             |
 *             |                             v (Memory Overwritten!)
 *             |                        New Program (/bin/ls)
 *             |                             |
 *             v waitpid()                   v (Runs ls code)
 *       Parent waits                   New Program exits
 *             |                             |
 *       Parent resumes <--------------------+ (Reaped)
 *             |
 *       Parent resumes & finishes
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include "process_demo.h"
#include "logger.h"

void exec_demo_run(void) {
    pid_t pid;
    
    printf("\n=== Exec System Call Demonstration ===\n");
    
    /* Display Theory */
    printf("\n--- Theory of exec() ---\n");
    printf("1. exec() replaces the current process memory image with a new executable.\n");
    printf("2. It does NOT create a new PID; the child PID remains the same.\n");
    printf("3. A successful exec() never returns because the original program memory is overwritten.\n");
    
    log_info("PROCESS", "exec demonstration started");
    
    printf("\n[PARENT PROCESS] Parent PID: %d\n", getpid());
    printf("[PARENT PROCESS] Forking child process...\n");
    fflush(stdout);
    
    pid = fork();
    
    if (pid < 0) {
        perror("fork failed");
        log_error("PROCESS", "exec demonstration: fork failed");
        return;
    }
    
    if (pid == 0) {
        /* Child Process */
        printf("\n[CHILD PROCESS] Child started. PID: %d, Parent PID (PPID): %d\n", getpid(), getppid());
        printf("[CHILD PROCESS] Current program: ./tests/process_test (our TUI program)\n");
        printf("[CHILD PROCESS] Calling execvp() to run '/bin/ls -l tests'...\n");
        fflush(stdout);
        
        log_info("PROCESS", "child executing external program");
        
        char* args[] = {"ls", "-l", "tests", NULL};
        execvp(args[0], args);
        
        /* If we get here, execvp failed! */
        perror("execvp failed");
        log_error("PROCESS", "execvp failed in child");
        exit(EXIT_FAILURE);
    } else {
        /* Parent Process */
        log_info("PROCESS", "parent waiting");
        
        printf("\n[PARENT PROCESS] Spawned child PID: %d\n", pid);
        printf("[PARENT PROCESS] Waiting for child to terminate...\n");
        fflush(stdout);
        
        int status;
        pid_t waited_pid = waitpid(pid, &status, 0);
        
        if (waited_pid < 0) {
            perror("waitpid failed");
            log_error("PROCESS", "exec demonstration: waitpid failed");
            return;
        }
        
        log_info("PROCESS", "child exited");
        
        printf("\n[PARENT PROCESS] Child process (PID %d) terminated.\n", waited_pid);
        
        if (WIFEXITED(status)) {
            printf("[PARENT PROCESS] Child exited normally with status: %d\n", WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("[PARENT PROCESS] Child terminated abnormally by signal: %d\n", WTERMSIG(status));
        } else {
            printf("[PARENT PROCESS] Child exited under other status.\n");
        }
        
        printf("[PARENT PROCESS] Parent resumes and finishes.\n");
        fflush(stdout);
    }
}
