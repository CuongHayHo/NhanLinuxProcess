/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: modules/kernel/kernel_mgr.c
 * Purpose: Kernel module user space integration manager and submenu.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "kernel_mgr.h"
#include "logger.h"
#include "menu.h"

static int read_kernel_choice(void) {
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

static void kernel_menu_pause(void) {
    int c;
    printf("\nPress ENTER to continue...");
    fflush(stdout);
    while ((c = getchar()) != '\n' && c != EOF);
}

void kernel_mgr_run(void) {
    int choice;
    log_info("KERNEL", "Entering Kernel Module Manager");

    while (1) {
        printf("\n========================================\n");
        printf("          Kernel Module Menu\n");
        printf("========================================\n");
        printf("1. Show Module Information\n");
        printf("0. Return\n");
        printf("========================================\n");
        printf("Select option: ");
        fflush(stdout);

        choice = read_kernel_choice();

        if (choice < 0) {
            continue;
        }

        if (choice > 2) {
            printf("\nInvalid input. Please choose a number between 0 and 2.\n");
            kernel_menu_pause();
            continue;
        }

        /* Support both 0 and 2 for Return as per acceptance criteria and UI standards */
        if (choice == 0 || choice == 2) {
            log_info("KERNEL", "Leaving Kernel Module Manager");
            break;
        }

        if (choice == 1) {
            kernel_mgr_show_info();
            kernel_menu_pause();
        }
    }
}

int kernel_mgr_show_info(void) {
    FILE *fp;
    char buffer[256];
    int line_count = 0;

    log_info("KERNEL", "Kernel module information requested");

    fp = fopen("/proc/sysmgr", "r");
    if (!fp) {
        log_error("KERNEL", "Read failure: failed to open /proc/sysmgr (errno %d)", errno);
        printf("\nError: Kernel module is not loaded or /proc/sysmgr is unavailable.\n");
        return -1;
    }

    printf("\n========================================\n");
    printf("Kernel Module Information (/proc/sysmgr)\n");
    printf("----------------------------------------\n");

    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        printf("%s", buffer);
        line_count++;
    }

    printf("========================================\n");

    fclose(fp);

    if (line_count > 0) {
        log_info("KERNEL", "Read success");
        return 0;
    } else {
        log_error("KERNEL", "Read failure: read 0 lines from /proc/sysmgr");
        return -1;
    }
}
