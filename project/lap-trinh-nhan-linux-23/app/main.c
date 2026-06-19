/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: app/main.c
 * Purpose: Application core entry point and dispatch control loop.
 */

#include <stdio.h>
#include <stdlib.h>
#include "menu.h"
#include "logger.h"
#include "file_mgr.h"
#include "process_mgr.h"
#include "network_mgr.h"
#include "package_mgr.h"
#include "socket_mgr.h"
#include "kernel_mgr.h"
#include "shell_mgr.h"

int main(void) {
    int choice;

    /* Logger Initialization */
    log_info("SYSTEM", "Application started");

    while (1) {
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

    return 0;
}
