/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: app/main.c
 * Purpose: Application core entry point and dispatch control loop.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>
#include "menu.h"
#include "logger.h"
#include "file_mgr.h"
#include "process_mgr.h"
#include "network_mgr.h"
#include "package_mgr.h"
#include "socket_mgr.h"
#include "kernel_mgr.h"
#include "shell_mgr.h"
#include "repl.h"
#include "ui.h"

int main(int argc, char* argv[]) {
    /* Ensure working directory is set to the repository root for relative path resolution */
    if (argv[0]) {
        char exe_path[PATH_MAX];
        if (realpath(argv[0], exe_path)) {
            char *dir = strrchr(exe_path, '/');
            if (dir) {
                *dir = '\0';
                if (chdir(exe_path) != 0) {
                    /* If chdir fails, log it but continue anyway */
                    fprintf(stderr, "Warning: Failed to change to %s\n", exe_path);
                }
            }
        }
    }

    /* Ensure the log directory exists; logger uses open() which won't create it */
    mkdir("logs", 0755);

    int classic_mode = 0;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--classic") == 0) {
            classic_mode = 1;
            break;
        }
    }

    if (classic_mode) {
        int choice;
        /* Logger Initialization */
        log_info("SYSTEM", "Application started in classic mode");

        while (1) {
            if (is_interactive) {
                const char* main_options[] = {
                    "File Manager (Quản lý file)",
                    "Process Manager (Quản lý tiến trình)",
                    "Network Manager (Quản lý mạng)",
                    "Socket Manager (Quản lý Socket)",
                    "Package Manager (Quản lý gói phần mềm)",
                    "Shell Manager (Quản lý Shell)",
                    "Kernel Module (Quản lý Kernel)",
                    "Exit (Thoát)"
                };
                int sel = ui_select_menu("Linux System Manager", main_options, 8);
                if (sel == 7 || sel == -1) {
                    choice = 0;
                } else {
                    choice = sel + 1;
                }
            } else {
                menu_clear_screen();
                menu_display_main();
                choice = menu_read_choice();

                /* Input Validation: check for non-numeric, empty, or out-of-bounds inputs */
                if (choice < 0 || choice > 7) {
                    printf("\n%sInvalid input. Please choose a number between 0 and 7.%s\n", 
                           ANSI_COLOR_RED, ANSI_COLOR_RESET);
                    menu_pause();
                    continue;
                }
            }

            /* Dispatch Loop */
            if (choice == 1) {
                log_info("SYSTEM", "Entering File Manager");
                file_mgr_run();
                log_info("SYSTEM", "Leaving File Manager");
                menu_pause();
            } else if (choice == 2) {
                log_info("SYSTEM", "Entering Process Manager");
                process_mgr_run();
                log_info("SYSTEM", "Leaving Process Manager");
                menu_pause();
            } else if (choice == 3) {
                log_info("SYSTEM", "Entering Network Manager");
                network_mgr_run();
                log_info("SYSTEM", "Leaving Network Manager");
                menu_pause();
            } else if (choice == 4) {
                log_info("SYSTEM", "Entering Socket Manager");
                socket_mgr_run();
                log_info("SYSTEM", "Leaving Socket Manager");
                menu_pause();
            } else if (choice == 5) {
                log_info("SYSTEM", "Entering Package Manager");
                package_mgr_run();
                log_info("SYSTEM", "Leaving Package Manager");
                menu_pause();
            } else if (choice == 6) {
                log_info("SYSTEM", "Entering Shell Manager");
                shell_mgr_run();
                log_info("SYSTEM", "Leaving Shell Manager");
                menu_pause();
            } else if (choice == 7) {
                log_info("SYSTEM", "Entering Kernel Module Manager");
                kernel_mgr_run();
                log_info("SYSTEM", "Leaving Kernel Module Manager");
                menu_pause();
            } else if (choice == 0) {
                /* Clean Exit */
                log_info("SYSTEM", "Application shutting down");
                printf("\nExiting Linux System Manager. Goodbye!\n");
                break;
            }
        }
    } else if (argc > 1) {
        /* Direct command execution (GUI compatibility) */
        is_interactive = 0;
        // Calculate total length needed
        size_t cmd_len = 0;
        for (int i = 1; i < argc; i++) {
            cmd_len += strlen(argv[i]) + 1;
        }
        char* cmd_line = malloc(cmd_len + 1);
        cmd_line[0] = '\0';
        for (int i = 1; i < argc; i++) {
            strcat(cmd_line, argv[i]);
            if (i < argc - 1) {
                strcat(cmd_line, " ");
            }
        }
        log_info("SYSTEM", "Direct command invocation: '%s'", cmd_line);
        repl_execute_command(cmd_line);
        free(cmd_line);
    } else {
        /* Run new interactive REPL mode */
        log_info("SYSTEM", "Application started in modern interactive REPL mode");
        repl_run();
    }

    return 0;
}
