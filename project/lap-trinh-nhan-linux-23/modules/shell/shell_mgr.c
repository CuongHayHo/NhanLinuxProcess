/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: modules/shell/shell_mgr.c
 * Purpose: Fork-exec command execution shell manager.
 */

#define _GNU_SOURCE /* Required for strdup */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include "shell_mgr.h"
#include "logger.h"
#include "menu.h"

static int read_shell_choice(void) {
    char input_buf[128];
    char* endptr;
    long val;

    if (fgets(input_buf, sizeof(input_buf), stdin) == NULL) {
        return -1;
    }

    input_buf[strcspn(input_buf, "\n")] = '\0';
    if (strlen(input_buf) == 0) {
        return -1;
    }

    val = strtol(input_buf, &endptr, 10);
    if (*endptr != '\0') {
        return -1;
    }
    return (int)val;
}

static void shell_menu_pause(void) {
    int c;
    printf("\nPress ENTER to continue...");
    fflush(stdout);
    while ((c = getchar()) != '\n' && c != EOF);
}

void shell_mgr_run(void) {
    int choice;
    char cmd_buf[512];
    log_info("SHELL", "Entering Shell Manager");

    while (1) {
        printf("\n========================================\n");
        printf("            Shell Manager\n");
        printf("========================================\n");
        printf("1. Execute Command\n");
        printf("2. Run Script (Future)\n");
        printf("3. Environment Variables (Future)\n");
        printf("4. Automation (Future)\n");
        printf("5. Return\n");
        printf("========================================\n");
        printf("Select option: ");
        fflush(stdout);

        choice = read_shell_choice();

        if (choice < 0) {
            continue;
        }

        if (choice > 5) {
            printf("\nInvalid input. Please choose a number between 0 and 5.\n");
            shell_menu_pause();
            continue;
        }

        /* Support both 0 and 5 for Return as per acceptance criteria and UI standards */
        if (choice == 0 || choice == 5) {
            log_info("SHELL", "Leaving Shell Manager");
            break;
        }

        if (choice >= 2 && choice <= 4) {
            printf("\nComing in a future sprint.\n");
            shell_menu_pause();
            continue;
        }

        if (choice == 1) {
            printf("\nEnter command to execute (e.g. 'ls -l', 'pwd', 'whoami'):\n");
            printf("Command: ");
            fflush(stdout);

            if (fgets(cmd_buf, sizeof(cmd_buf), stdin) != NULL) {
                /* Remove trailing newline */
                cmd_buf[strcspn(cmd_buf, "\n")] = '\0';

                if (strlen(cmd_buf) > 0) {
                    shell_mgr_execute(cmd_buf);
                } else {
                    printf("Error: Empty command string.\n");
                }
            }
            shell_menu_pause();
        }
    }
}

int shell_mgr_execute(const char* command) {
    char* cmd_copy;
    char* args[64];
    int arg_count = 0;
    pid_t pid;

    if (!command || strlen(command) == 0) {
        return -1;
    }

    log_info("SHELL", "Command entered: '%s'", command);

    /* Tokenize command safely */
    cmd_copy = strdup(command);
    if (!cmd_copy) {
        log_error("SHELL", "Errors: memory allocation failed (strdup)");
        return -1;
    }

    char* token = strtok(cmd_copy, " \t");
    while (token != NULL && arg_count < 63) {
        args[arg_count++] = token;
        token = strtok(NULL, " \t");
    }
    args[arg_count] = NULL;

    if (arg_count == 0) {
        free(cmd_copy);
        return -1;
    }

    /* Fork child process */
    pid = fork();
    if (pid < 0) {
        log_error("SHELL", "Errors: fork failed (errno %d)", errno);
        free(cmd_copy);
        return -1;
    }

    if (pid == 0) {
        /* Child process execution */
        execvp(args[0], args);

        /* If execvp returns, it means execution failed */
        fprintf(stderr, "Error: Command '%s' not found or execution failed (errno %d)\n", args[0], errno);
        free(cmd_copy);
        exit(127);
    } else {
        /* Parent process waiting */
        int status;
        log_info("SHELL", "Child process created (PID: %d)", pid);

        if (waitpid(pid, &status, 0) == -1) {
            log_error("SHELL", "Errors: waitpid failed (errno %d)", errno);
            free(cmd_copy);
            return -1;
        }

        free(cmd_copy);

        if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);
            log_info("SHELL", "Command executed");
            log_info("SHELL", "Exit status: %d", exit_code);
            return exit_code;
        } else if (WIFSIGNALED(status)) {
            int sig = WTERMSIG(status);
            log_error("SHELL", "Errors: command terminated by signal %d", sig);
            return -1;
        }
    }

    return -1;
}
