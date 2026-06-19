/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: app/menu.c
 * Purpose: Terminal User Interface (TUI) display rendering and robust input collection.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "menu.h"

void menu_clear_screen(void) {
    /* Send ANSI escape sequences to clear screen and home cursor */
    printf("\033[H\033[2J");
    fflush(stdout);
}

void menu_display_main(void) {
    printf("%s========================================\n", ANSI_COLOR_CYAN);
    printf("           Linux System Manager\n");
    printf("========================================%s\n", ANSI_COLOR_RESET);
    printf("1. File Manager\n");
    printf("2. Process Manager\n");
    printf("3. Signal Manager\n");
    printf("4. Network Manager\n");
    printf("5. Socket Chat\n");
    printf("6. Package Manager\n");
    printf("7. Scheduler\n");
    printf("8. System Information\n");
    printf("9. Log Viewer\n");
    printf("10. Kernel Module\n");
    printf("11. Exit\n");
    printf("%s----------------------------------------%s\n", ANSI_COLOR_CYAN, ANSI_COLOR_RESET);
    printf("Select option: ");
    fflush(stdout);
}

int menu_read_choice(void) {
    char input_buf[128];
    char* endptr;
    long val;

    if (fgets(input_buf, sizeof(input_buf), stdin) == NULL) {
        return -1;
    }

    /* Remove trailing newline character */
    input_buf[strcspn(input_buf, "\n")] = '\0';

    /* Check for empty input */
    if (strlen(input_buf) == 0) {
        return -1;
    }

    /* Convert string to long integer */
    val = strtol(input_buf, &endptr, 10);

    /* If conversion failed or extra non-numeric characters exist, return -1 */
    if (*endptr != '\0') {
        return -1;
    }

    return (int)val;
}

void menu_pause(void) {
    int c;
    printf("\nPress ENTER to continue...");
    fflush(stdout);
    
    /* Consume characters until a newline or EOF is reached */
    while ((c = getchar()) != '\n' && c != EOF);
}
