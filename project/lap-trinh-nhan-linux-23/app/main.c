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

int main(void) {
    int choice;

    /* Logger Initialization */
    log_info("SYSTEM", "Application started");

    while (1) {
        menu_clear_screen();
        menu_display_main();
        choice = menu_read_choice();

        /* Input Validation: check for non-numeric, empty, or out-of-bounds inputs */
        if (choice < 0 || choice > 11) {
            printf("\n%sInvalid input. Please choose a number between 1 and 11.%s\n", 
                   ANSI_COLOR_RED, ANSI_COLOR_RESET);
            menu_pause();
            continue;
        }

        /* Dispatch Loop */
        if (choice >= 1 && choice <= 10) {
            printf("\n---\n");
            printf("%sTODO%s\n", ANSI_COLOR_YELLOW, ANSI_COLOR_RESET);
            printf("This module is not implemented yet.\n");
            printf("---\n");
            
            menu_pause();
            printf("\nReturn to menu.\n");
        } else if (choice == 11 || choice == 0) {
            /* Clean Exit */
            log_info("SYSTEM", "Application shutting down");
            printf("\nExiting Linux System Manager. Goodbye!\n");
            break;
        }
    }

    return 0;
}
