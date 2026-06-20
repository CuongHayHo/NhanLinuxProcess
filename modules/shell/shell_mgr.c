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
#include "ui.h"
#include "linenoise.h"
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





void shell_mgr_env_run(void) {
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
    log_info("SHELL", "Entering Shell Manager");
    extern int is_interactive;

    while (1) {
        if (is_interactive) {
            const char* shell_options[] = {
                "Execute Command (Chạy lệnh)",
                "Run Script (Chạy kịch bản)",
                "Environment Manager (Quản lý môi trường)",
                "File Management (Quản lý tệp)",
                "Task Scheduling (Lập lịch tác vụ)",
                "System Time (Thời gian hệ thống)",
                "Package Manager (Trình quản lý gói)",
                "Return (Trở về)"
            };
            int sel = ui_select_menu("Shell Manager", shell_options, 8);
            if (sel == 7 || sel == -1) {
                log_info("SHELL", "Leaving Shell Manager");
                break;
            }
            if (sel == 0) {
                char cmd_buf[512];
                printf("\nEnter command to execute (e.g. 'ls -l', 'pwd', 'whoami'):\n");
                printf("Command: ");
                fflush(stdout);
                if (fgets(cmd_buf, sizeof(cmd_buf), stdin) != NULL) {
                    cmd_buf[strcspn(cmd_buf, "\n")] = '\0';
                    if (strlen(cmd_buf) > 0) {
                        shell_mgr_execute(cmd_buf);
                    } else {
                        printf("Error: Empty command string.\n");
                    }
                }
                shell_menu_pause();
            } else if (sel == 1) {
                const char* script_options[] = {
                    "backup.sh",
                    "disk_usage.sh",
                    "show_date.sh",
                    "Cancel"
                };
                int script_choice = ui_select_menu("Select Script to Run", script_options, 4);
                if (script_choice == 0) {
                    shell_mgr_execute("/bin/bash scripts/backup.sh");
                } else if (script_choice == 1) {
                    shell_mgr_execute("/bin/bash scripts/disk_usage.sh");
                } else if (script_choice == 2) {
                    shell_mgr_execute("/bin/bash scripts/show_date.sh");
                }
                if (script_choice >= 0 && script_choice <= 2) {
                    shell_menu_pause();
                }
            } else if (sel == 2) {
                shell_mgr_env_run();
            } else if (sel == 3) {
                log_info("SHELL", "Running shell/file.sh");
                shell_mgr_execute("/bin/bash shell/file.sh");
                shell_menu_pause();
            } else if (sel == 4) {
                log_info("SHELL", "Running task scheduling (cron)");
                shell_mgr_cron_execute(1, (char*[]){"cron"});
                shell_menu_pause();
            } else if (sel == 5) {
                log_info("SHELL", "Running system time");
                shell_mgr_time_execute(1, (char*[]){"time"});
                shell_menu_pause();
            } else if (sel == 6) {
                log_info("SHELL", "Running package manager");
                shell_mgr_execute("/bin/bash shell/program.sh");
                shell_menu_pause();
            }
        } else {
            // Non-interactive classic mode menu
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
            int choice = read_shell_choice();
            if (choice < 0 || choice == 0) {
                log_info("SHELL", "Leaving Shell Manager");
                break;
            }
            if (choice == 1) {
                char cmd_buf[512];
                if (fgets(cmd_buf, sizeof(cmd_buf), stdin) != NULL) {
                    cmd_buf[strcspn(cmd_buf, "\n")] = '\0';
                    if (strlen(cmd_buf) > 0) {
                        shell_mgr_execute(cmd_buf);
                    }
                }
            } else if (choice == 2) {
                int script_choice = read_shell_choice();
                if (script_choice == 1) {
                    shell_mgr_execute("/bin/bash scripts/backup.sh");
                } else if (script_choice == 2) {
                    shell_mgr_execute("/bin/bash scripts/disk_usage.sh");
                } else if (script_choice == 3) {
                    shell_mgr_execute("/bin/bash scripts/show_date.sh");
                }
            } else if (choice == 3) {
                shell_mgr_env_run();
            } else if (choice == 4) {
                shell_mgr_execute("/bin/bash shell/file.sh");
            } else if (choice == 5) {
                shell_mgr_cron_execute(1, (char*[]){"cron"});
            } else if (choice == 6) {
                shell_mgr_time_execute(1, (char*[]){"time"});
            } else if (choice == 7) {
                shell_mgr_execute("/bin/bash shell/program.sh");
            }
        }
    }
}

void shell_mgr_cron_execute(int argc, char** argv) {
    extern int is_interactive;
    if (argc > 1) {
        const char* sub = argv[1];
        if (strcmp(sub, "list") == 0 || strcmp(sub, "show") == 0) {
            printf("\n--- Task Scheduling List ---\n");
            shell_mgr_execute("crontab -l");
        } else if (strcmp(sub, "delete") == 0 || strcmp(sub, "clear") == 0) {
            shell_mgr_execute("crontab -r");
            printf("All cron tasks deleted.\n");
        } else if (strcmp(sub, "create") == 0) {
            char path[256] = "";
            char expr[128] = "";
            int has_path = (argc > 2);
            int has_expr = (argc > 3);

            if (has_path) {
                strncpy(path, argv[2], sizeof(path)-1);
            }
            if (has_expr) {
                strncpy(expr, argv[3], sizeof(expr)-1);
            }

            if (!has_path) {
                if (is_interactive) {
                    char* input = linenoise("Enter file path to script: ");
                    if (input) {
                        strncpy(path, input, sizeof(path)-1);
                        linenoiseFree(input);
                    }
                } else {
                    printf("Enter file path to script: ");
                    fflush(stdout);
                    if (fgets(path, sizeof(path), stdin)) {
                        path[strcspn(path, "\n")] = '\0';
                    }
                }
            }

            if (strlen(path) > 0 && !has_expr) {
                if (is_interactive) {
                    char* input = linenoise("Enter cron expression (e.g. '* * * * *'): ");
                    if (input) {
                        strncpy(expr, input, sizeof(expr)-1);
                        linenoiseFree(input);
                    }
                } else {
                    printf("Enter cron expression (e.g. '* * * * *'): ");
                    fflush(stdout);
                    if (fgets(expr, sizeof(expr), stdin)) {
                        expr[strcspn(expr, "\n")] = '\0';
                    }
                }
            }

            if (strlen(path) > 0 && strlen(expr) > 0) {
                char chmod_cmd[512];
                snprintf(chmod_cmd, sizeof(chmod_cmd), "chmod +x %s", path);
                shell_mgr_execute(chmod_cmd);

                char cron_cmd[1024];
                snprintf(cron_cmd, sizeof(cron_cmd), "crontab -l 2>/dev/null | { cat; echo \"%s %s\"; } | crontab -", expr, path);
                if (shell_mgr_execute(cron_cmd) == 0) {
                    printf("Successfully scheduled task '%s' with expression '%s'\n", path, expr);
                } else {
                    printf("Failed to schedule task.\n");
                }
            } else {
                printf("Error: Missing file path or cron expression.\n");
            }
        } else {
            printf("Unknown cron subcommand '%s'. Use: list, create, delete.\n", sub);
        }
    } else {
        if (is_interactive) {
            const char* cron_options[] = {
                "Danh sách tác vụ (List scheduled tasks)",
                "Tạo tác vụ (Create new scheduled task)",
                "Xoá tất cả tác vụ (Delete all scheduled tasks)",
                "Cancel"
            };
            int sel = ui_select_menu("Task Scheduling Manager (Cron)", cron_options, 4);
            if (sel == 0) {
                printf("\n--- Task Scheduling List ---\n");
                shell_mgr_execute("crontab -l");
            } else if (sel == 1) {
                char* path_input = linenoise("Enter file path to script: ");
                if (!path_input) return;
                char* expr_input = linenoise("Enter cron expression (e.g. '* * * * *'): ");
                if (!expr_input) {
                    linenoiseFree(path_input);
                    return;
                }

                char chmod_cmd[512];
                snprintf(chmod_cmd, sizeof(chmod_cmd), "chmod +x %s", path_input);
                shell_mgr_execute(chmod_cmd);

                char cron_cmd[1024];
                snprintf(cron_cmd, sizeof(cron_cmd), "crontab -l 2>/dev/null | { cat; echo \"%s %s\"; } | crontab -", expr_input, path_input);
                if (shell_mgr_execute(cron_cmd) == 0) {
                    printf("Successfully scheduled task '%s' with expression '%s'\n", path_input, expr_input);
                } else {
                    printf("Failed to schedule task.\n");
                }
                linenoiseFree(path_input);
                linenoiseFree(expr_input);
            } else if (sel == 2) {
                shell_mgr_execute("crontab -r");
                printf("All cron tasks deleted.\n");
            }
        } else {
            printf("Usage: cron {list|create|delete} [args...]\n");
        }
    }
}

void shell_mgr_time_execute(int argc, char** argv) {
    extern int is_interactive;
    if (argc > 1) {
        const char* sub = argv[1];
        if (strcmp(sub, "show") == 0) {
            shell_mgr_execute("date");
        } else if (strcmp(sub, "zone") == 0) {
            if (access("/usr/bin/timedatectl", F_OK) == 0) {
                shell_mgr_execute("timedatectl | grep 'Time zone'");
            } else {
                printf("Time zone: ");
                fflush(stdout);
                shell_mgr_execute("cat /etc/timezone 2>/dev/null || date +%Z");
            }
        } else if (strcmp(sub, "set") == 0) {
            char datetime[128] = "";
            int has_dt = (argc > 2);
            if (has_dt) {
                strncpy(datetime, argv[2], sizeof(datetime)-1);
            } else {
                if (is_interactive) {
                    char* input = linenoise("Enter YYYY-MM-DD HH:MM:SS: ");
                    if (input) {
                        strncpy(datetime, input, sizeof(datetime)-1);
                        linenoiseFree(input);
                    }
                } else {
                    printf("Enter YYYY-MM-DD HH:MM:SS: ");
                    fflush(stdout);
                    if (fgets(datetime, sizeof(datetime), stdin)) {
                        datetime[strcspn(datetime, "\n")] = '\0';
                    }
                }
            }

            if (strlen(datetime) > 0) {
                char date_cmd[256];
                snprintf(date_cmd, sizeof(date_cmd), "sudo date -s \"%s\"", datetime);
                if (shell_mgr_execute(date_cmd) == 0) {
                    printf("System time updated successfully.\n");
                } else {
                    printf("Failed to set system time (requires root/sudo).\n");
                }
            }
        } else if (strcmp(sub, "sync") == 0) {
            printf("Performing Internet time sync...\n");
            if (access("/usr/sbin/chronyc", F_OK) == 0 || access("/usr/bin/chronyc", F_OK) == 0) {
                shell_mgr_execute("sudo chronyc makestep");
            } else if (access("/usr/sbin/ntpdate", F_OK) == 0 || access("/usr/bin/ntpdate", F_OK) == 0) {
                shell_mgr_execute("sudo ntpdate pool.ntp.org");
            } else {
                printf("Sync backend not found (install chrony or ntpdate).\n");
            }
        } else if (strcmp(sub, "autosync") == 0) {
            if (access("/usr/bin/timedatectl", F_OK) == 0) {
                shell_mgr_execute("sudo timedatectl set-ntp true");
            } else {
                printf("timedatectl not available on this system.\n");
            }
        } else {
            printf("Unknown time subcommand '%s'. Use: show, zone, set, sync, autosync.\n", sub);
        }
    } else {
        if (is_interactive) {
            const char* time_options[] = {
                "Xem thời gian hiện tại (Show current time)",
                "Xem múi giờ (Show timezone)",
                "Thiết lập thời gian thủ công (Set time manually)",
                "Đồng bộ thời gian qua Internet (Internet time sync)",
                "Bật tự động đồng bộ (NTP) (Enable auto sync)",
                "Cancel"
            };
            int sel = ui_select_menu("Time Configuration Manager", time_options, 6);
            if (sel == 0) {
                shell_mgr_execute("date");
            } else if (sel == 1) {
                if (access("/usr/bin/timedatectl", F_OK) == 0) {
                    shell_mgr_execute("timedatectl | grep 'Time zone'");
                } else {
                    printf("Time zone: ");
                    fflush(stdout);
                    shell_mgr_execute("cat /etc/timezone 2>/dev/null || date +%Z");
                }
            } else if (sel == 2) {
                char* input = linenoise("Enter YYYY-MM-DD HH:MM:SS: ");
                if (!input) return;
                char date_cmd[256];
                snprintf(date_cmd, sizeof(date_cmd), "sudo date -s \"%s\"", input);
                if (shell_mgr_execute(date_cmd) == 0) {
                    printf("System time updated successfully.\n");
                } else {
                    printf("Failed to set system time (requires root/sudo).\n");
                }
                linenoiseFree(input);
            } else if (sel == 3) {
                printf("Performing Internet time sync...\n");
                if (access("/usr/sbin/chronyc", F_OK) == 0 || access("/usr/bin/chronyc", F_OK) == 0) {
                    shell_mgr_execute("sudo chronyc makestep");
                } else if (access("/usr/sbin/ntpdate", F_OK) == 0 || access("/usr/bin/ntpdate", F_OK) == 0) {
                    shell_mgr_execute("sudo ntpdate pool.ntp.org");
                } else {
                    printf("Sync backend not found (install chrony or ntpdate).\n");
                }
            } else if (sel == 4) {
                if (access("/usr/bin/timedatectl", F_OK) == 0) {
                    shell_mgr_execute("sudo timedatectl set-ntp true");
                } else {
                    printf("timedatectl not available on this system.\n");
                }
            }
        } else {
            printf("Usage: time {show|zone|set|sync|autosync} [args...]\n");
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
