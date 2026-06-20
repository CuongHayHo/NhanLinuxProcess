/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: cli/ui.c
 * Purpose: ANSI Color formatting and layout output definitions.
 */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include "ui.h"

void ui_print_success(const char *format, ...) {
    va_list args;
    printf("%s", ANSI_GREEN);
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf("%s", ANSI_RESET);
    fflush(stdout);
}

void ui_print_warning(const char *format, ...) {
    va_list args;
    printf("%s", ANSI_YELLOW);
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf("%s", ANSI_RESET);
    fflush(stdout);
}

void ui_print_error(const char *format, ...) {
    va_list args;
    printf("%s", ANSI_RED);
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf("%s", ANSI_RESET);
    fflush(stdout);
}

void ui_print_info(const char *format, ...) {
    va_list args;
    printf("%s", ANSI_BLUE);
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf("%s", ANSI_RESET);
    fflush(stdout);
}

void ui_print_title(const char *format, ...) {
    va_list args;
    printf("%s%s", ANSI_BOLD, ANSI_CYAN);
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf("%s", ANSI_RESET);
    fflush(stdout);
}

void ui_print_bold(const char *format, ...) {
    va_list args;
    printf("%s", ANSI_BOLD);
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf("%s", ANSI_RESET);
    fflush(stdout);
}

void ui_clear_screen(void) {
    printf("\033[H\033[2J");
    fflush(stdout);
}

void ui_show_banner(const char *version) {
    printf("\n%s---------------------------------------------------------%s\n", ANSI_CYAN, ANSI_RESET);
    ui_print_title("Linux System Manager (v%s)\n\n", version);
    ui_print_bold("Concepts:\n");
    printf("  - Shell Programming (File & Package launcher scripts)\n");
    printf("  - Linux Programming (Process & POSIX Network control APIs)\n");
    printf("  - Kernel Module (procfs sequence files & system parameters)\n\n");
    ui_print_info("Type / to browse commands\n");
    ui_print_info("Type help for help documentation\n");
    ui_print_warning("Press Ctrl+C to exit\n");
    printf("%s---------------------------------------------------------%s\n", ANSI_CYAN, ANSI_RESET);
    fflush(stdout);
}

void ui_show_help(void) {
    printf("\n%s=================== Help Documentation ===================%s\n", ANSI_CYAN, ANSI_RESET);
    ui_print_bold("Navigation:\n");
    printf("  - Type the name of a module (e.g. 'file', 'package') to change context.\n");
    printf("  - Prompt changes to match: 'sysmgr/package > '.\n");
    printf("  - Type 'back' or '..' or press Escape to return to the parent context.\n");
    printf("  - Type 'exit' at the main prompt to close the application.\n\n");

    ui_print_bold("Commands:\n");
    printf("  - Type '/' to show all available commands in your current context.\n");
    printf("  - Type '/<search>' to filter and display matching commands (e.g. '/pa').\n");
    printf("  - You can also execute commands directly with parameters (e.g. 'package search hello').\n\n");

    ui_print_bold("Keyboard Shortcuts:\n");
    printf("  - %sTAB%s          : Autocomplete commands / cycle parameters.\n", ANSI_GREEN, ANSI_RESET);
    printf("  - %sUp/Down Arrow%s: Navigate persistent execution history.\n", ANSI_GREEN, ANSI_RESET);
    printf("  - %sCtrl+L%s       : Clear screen.\n", ANSI_GREEN, ANSI_RESET);
    printf("  - %sCtrl+C%s       : Force exit the application.\n", ANSI_GREEN, ANSI_RESET);
    printf("========================================================\n\n");
    fflush(stdout);
}
