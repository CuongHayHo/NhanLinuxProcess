/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: modules/process/demo/zombie_demo.c
 * Purpose: Zombie process creation and reaping demonstration.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include "process_demo.h"
#include "logger.h"

/* Static helper to parse /proc/<pid>/status using low-level POSIX calls */
static int parse_proc_status(pid_t pid, char* name_out, char* state_out, pid_t* pid_out, pid_t* ppid_out) {
    char path[128];
    char buf[2048];
    int fd;
    ssize_t n;

    snprintf(path, sizeof(path), "/proc/%d/status", pid);
    fd = open(path, O_RDONLY);
    if (fd < 0) {
        return -1;
    }

    n = read(fd, buf, sizeof(buf) - 1);
    close(fd);
    if (n <= 0) {
        return -1;
    }
    buf[n] = '\0';

    name_out[0] = '\0';
    state_out[0] = '\0';
    *pid_out = 0;
    *ppid_out = 0;

    char* line = buf;
    char* next_line;
    while (line && *line != '\0') {
        next_line = strchr(line, '\n');
        if (next_line) {
            *next_line = '\0';
            next_line++;
        }

        char key[32];
        char val[256];
        /* Parse key and value, supporting whitespace delimiters */
        if (sscanf(line, "%31s %255s", key, val) >= 2) {
            if (strcmp(key, "Name:") == 0) {
                strncpy(name_out, val, 63);
                name_out[63] = '\0';
            } else if (strcmp(key, "State:") == 0) {
                strncpy(state_out, val, 15);
                state_out[15] = '\0';
            } else if (strcmp(key, "Pid:") == 0) {
                *pid_out = (pid_t)atoi(val);
            } else if (strcmp(key, "PPid:") == 0) {
                *ppid_out = (pid_t)atoi(val);
            }
        }
        line = next_line;
    }
    return 0;
}

void zombie_demo_run(void) {
    log_info("PROCESS", "Zombie demo started");

    printf("\n=== Zombie Process Demonstration ===\n");
    printf("Theory:\n");
    printf("  1. A Zombie process is a terminated child process whose parent has not yet\n");
    printf("     read its exit status via wait() or waitpid().\n");
    printf("  2. Since the parent might want to query the status later, the kernel keeps\n");
    printf("     a minimal entry in the process table (including PID, PPID, exit status).\n");
    printf("  3. The zombie remains until the parent explicitly reaps it via wait() or waitpid(),\n");
    printf("     or until the parent process terminates (in which case the init process adopts\n");
    printf("     and reaps it).\n\n");

    pid_t parent_pid = getpid();
    printf("Parent PID: %d\n", parent_pid);

    printf("Forking a child process...\n");
    pid_t child_pid = fork();

    if (child_pid < 0) {
        log_error("PROCESS", "fork() failed in Zombie demo (errno %d)", errno);
        printf("Error: Fork failed!\n");
        log_info("PROCESS", "Demo finished");
        return;
    }

    if (child_pid == 0) {
        /* Child Process: exit immediately to become a zombie */
        printf("  [Child] PID: %d, PPID: %d\n", getpid(), getppid());
        printf("  [Child] Exiting immediately to trigger zombie state...\n");
        log_info("PROCESS", "Child exited");
        exit(42);
    }

    /* Parent Process */
    printf("Child PID:  %d\n", child_pid);
    printf("Parent intentionally sleeping for 3 seconds WITHOUT calling waitpid()...\n");
    sleep(3);

    /* Verify zombie state from /proc/<child_pid>/status */
    char name[64];
    char state[16];
    pid_t p_pid, pp_pid;
    
    printf("Reading /proc/%d/status...\n", child_pid);
    if (parse_proc_status(child_pid, name, state, &p_pid, &pp_pid) == 0) {
        printf("Extracted /proc/%d/status:\n", child_pid);
        printf("  Name:   %s\n", name);
        printf("  State:  %s\n", state);
        printf("  Pid:    %d\n", p_pid);
        printf("  PPid:   %d\n", pp_pid);

        if (state[0] == 'Z') {
            log_info("PROCESS", "Zombie detected");
            printf("Zombie detected: State is indeed Z (Zombie).\n");
        } else {
            printf("Warning: Child state is '%s' (not Zombie).\n", state);
        }
    } else {
        log_error("PROCESS", "Missing /proc entry or permission denied for PID %d", child_pid);
        printf("Error: Failed to read /proc/%d/status\n", child_pid);
    }

    /* Parent reaps child */
    printf("Parent now calling waitpid() to reap child...\n");
    log_info("PROCESS", "waitpid called");
    
    int status;
    if (waitpid(child_pid, &status, 0) < 0) {
        log_error("PROCESS", "waitpid() failed (errno %d)", errno);
        printf("Error: waitpid() failed!\n");
    } else {
        printf("Parent reaped child.\n");
        if (WIFEXITED(status)) {
            printf("  Child exit status code: %d\n", WEXITSTATUS(status));
        }
    }

    /* Verify zombie has disappeared */
    printf("Reading /proc/%d/status again to verify clean up...\n", child_pid);
    if (parse_proc_status(child_pid, name, state, &p_pid, &pp_pid) != 0) {
        log_info("PROCESS", "Zombie removed");
        printf("Zombie removed: /proc/%d/status no longer exists.\n", child_pid);
    } else {
        printf("Error: /proc/%d/status still exists! Child state: %s\n", child_pid, state);
    }

    printf("Explanation:\n");
    printf("  Calling waitpid() successfully read the child's exit status.\n");
    printf("  This informed the kernel that the parent has completed queries on this child,\n");
    printf("  allowing the kernel to safely release the child's PID and process descriptor.\n");

    log_info("PROCESS", "Demo finished");
    printf("====================================\n");
}
