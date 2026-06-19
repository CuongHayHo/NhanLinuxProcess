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
#include <fcntl.h>
#include <sys/stat.h>
#include "shell_mgr.h"
#include "logger.h"
#include "menu.h"
#include <time.h>


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





static void shell_mgr_env_run(void) {
    int choice;
    char name_buf[128];
    char val_buf[256];

    while (1) {
        printf("\n========================================\n");
        printf("          Environment Manager\n");
        printf("========================================\n");
        printf("1. List Important Environment Variables\n");
        printf("2. Query Variable\n");
        printf("3. Set Temporary Variable\n");
        printf("4. Remove Temporary Variable\n");
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

        if (choice == 0 || choice == 5) {
            break;
        }

        if (choice == 1) {
            const char* vars[] = {"PATH", "HOME", "USER", "PWD", "SHELL", "LANG", "TERM"};
            printf("\n%-10s | Value\n", "Variable");
            printf("----------------------------------------\n");
            for (size_t i = 0; i < sizeof(vars)/sizeof(vars[0]); i++) {
                char* val = getenv(vars[i]);
                printf("%-10s | %s\n", vars[i], val ? val : "N/A");
            }
            shell_menu_pause();
        } else if (choice == 2) {
            printf("\nEnter variable name to query: ");
            fflush(stdout);
            if (fgets(name_buf, sizeof(name_buf), stdin) != NULL) {
                name_buf[strcspn(name_buf, "\n")] = '\0';
                if (strlen(name_buf) > 0) {
                    char* val = getenv(name_buf);
                    if (val) {
                        printf("\n%s = %s\n", name_buf, val);
                        log_info("SHELL", "Variable queried: '%s' = '%s'", name_buf, val);
                    } else {
                        printf("\nVariable '%s' is not set.\n", name_buf);
                        log_info("SHELL", "Variable queried: '%s' = N/A", name_buf);
                    }
                } else {
                    printf("Error: Empty variable name.\n");
                }
            }
            shell_menu_pause();
        } else if (choice == 3) {
            printf("\nEnter variable name: ");
            fflush(stdout);
            if (fgets(name_buf, sizeof(name_buf), stdin) != NULL) {
                name_buf[strcspn(name_buf, "\n")] = '\0';
                if (strlen(name_buf) > 0) {
                    printf("Enter variable value: ");
                    fflush(stdout);
                    if (fgets(val_buf, sizeof(val_buf), stdin) != NULL) {
                        val_buf[strcspn(val_buf, "\n")] = '\0';
                        if (setenv(name_buf, val_buf, 1) == 0) {
                            printf("\nSuccessfully set %s = %s\n", name_buf, val_buf);
                            log_info("SHELL", "Variable set: '%s' = '%s'", name_buf, val_buf);
                        } else {
                            printf("\nError setting variable '%s'.\n", name_buf);
                            log_error("SHELL", "Errors: failed to set variable '%s'", name_buf);
                        }
                    }
                } else {
                    printf("Error: Empty variable name.\n");
                }
            }
            shell_menu_pause();
        } else if (choice == 4) {
            printf("\nEnter variable name to remove: ");
            fflush(stdout);
            if (fgets(name_buf, sizeof(name_buf), stdin) != NULL) {
                name_buf[strcspn(name_buf, "\n")] = '\0';
                if (strlen(name_buf) > 0) {
                    if (getenv(name_buf) == NULL) {
                        printf("\nWarning: Variable '%s' is not set.\n", name_buf);
                    }
                    if (unsetenv(name_buf) == 0) {
                        printf("\nSuccessfully removed variable '%s'\n", name_buf);
                        log_info("SHELL", "Variable removed: '%s'", name_buf);
                    } else {
                        printf("\nError removing variable '%s'.\n", name_buf);
                        log_error("SHELL", "Errors: failed to remove variable '%s'", name_buf);
                    }
                } else {
                    printf("Error: Empty variable name.\n");
                }
            }
            shell_menu_pause();
        }
    }
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
        printf("2. Run Script\n");
        printf("3. Environment Manager\n");
        printf("4. File Management\n");
        printf("5. Task Scheduling\n");
        printf("6. System Time\n");
        printf("7. Package Manager\n");
        printf("0. Return\n");
        printf("========================================\n");
        printf("Select option: ");
        fflush(stdout);

        choice = read_shell_choice();

        if (choice < 0) {
            continue;
        }

        if (choice > 7) {
            printf("\nInvalid input. Please choose a number between 0 and 7.\n");
            shell_menu_pause();
            continue;
        }

        if (choice == 0) {
            log_info("SHELL", "Leaving Shell Manager");
            break;
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
        } else if (choice == 2) {
            printf("\nSelect script to run:\n");
            printf("1. backup.sh\n");
            printf("2. disk_usage.sh\n");
            printf("3. show_date.sh\n");
            printf("0. Cancel\n");
            printf("Select option: ");
            fflush(stdout);

            int script_choice = read_shell_choice();
            if (script_choice == 1) {
                shell_mgr_execute("/bin/bash scripts/backup.sh");
            } else if (script_choice == 2) {
                shell_mgr_execute("/bin/bash scripts/disk_usage.sh");
            } else if (script_choice == 3) {
                shell_mgr_execute("/bin/bash scripts/show_date.sh");
            } else if (script_choice == 0) {
                printf("Cancelled.\n");
            } else {
                printf("Invalid selection.\n");
            }
            shell_menu_pause();
        } else if (choice == 3) {
            shell_mgr_env_run();
        } else if (choice == 4) {
            log_info("SHELL", "Running shell/file.sh");
            shell_mgr_execute("/bin/bash shell/file.sh");
        } else if (choice == 5) {
            log_info("SHELL", "Running shell/crontab.sh");
            shell_mgr_execute("/bin/bash shell/crontab.sh");
        } else if (choice == 6) {
            log_info("SHELL", "Running shell/time.sh");
            shell_mgr_execute("/bin/bash shell/time.sh");
        } else if (choice == 7) {
            log_info("SHELL", "Running shell/program.sh");
            shell_mgr_execute("/bin/bash shell/program.sh");
        }
    }
}

int shell_mgr_execute(const char* command) {
    char* cmd_copy;
    char* args[64];
    int arg_count = 0;
    pid_t pid;
    int is_script = 0;
    const char* script_path = NULL;

    if (!command || strlen(command) == 0) {
        return -1;
    }

    if (strncmp(command, "/bin/bash ", 10) == 0) {
        is_script = 1;
        script_path = command + 10;
        log_info("SHELL", "Script selected: '%s'", script_path);
    } else {
        log_info("SHELL", "Command entered: '%s'", command);
    }

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
        if (is_script) {
            fprintf(stderr, "Error: Script '%s' not found or execution failed (errno %d)\n", script_path, errno);
        } else {
            fprintf(stderr, "Error: Command '%s' not found or execution failed (errno %d)\n", args[0], errno);
        }
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
            if (is_script) {
                log_info("SHELL", "Script executed");
            } else {
                log_info("SHELL", "Command executed");
            }
            log_info("SHELL", "Exit status: %d", exit_code);
            if (exit_code != 0) {
                log_error("SHELL", "Errors: execution failed with exit code %d", exit_code);
            }
            return exit_code;
        } else if (WIFSIGNALED(status)) {
            int sig = WTERMSIG(status);
            log_error("SHELL", "Errors: command terminated by signal %d", sig);
            return -1;
        }
    }

    return -1;
}
