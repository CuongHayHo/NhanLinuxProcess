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
#include <string.h>
#include <termios.h>
#include <unistd.h>
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
    printf("\033[38;2;255;0;255m█\033[39m\033[38;2;243;12;255m█\033[39m\033[38;2;231;24;255m╗\033[39m\033[38;2;219;36;255m \033[39m\033[38;2;206;49;255m \033[39m\033[38;2;194;61;255m█\033[39m\033[38;2;182;73;255m█\033[39m\033[38;2;170;85;255m╗\033[39m\033[38;2;158;97;255m█\033[39m\033[38;2;146;109;255m█\033[39m\033[38;2;134;121;255m█\033[39m\033[38;2;121;134;255m█\033[39m\033[38;2;109;146;255m█\033[39m\033[38;2;97;158;255m█\033[39m\033[38;2;85;170;255m█\033[39m\033[38;2;73;182;255m╗\033[39m\033[38;2;61;194;255m█\033[39m\033[38;2;49;206;255m█\033[39m\033[38;2;36;219;255m╗\033[39m\033[38;2;24;231;255m \033[39m\033[38;2;12;243;255m \033[39m\033[38;2;0;255;255m \033[39m\033[38;2;12;243;255m \033[39m\033[38;2;23;232;255m \033[39m\033[38;2;35;220;255m█\033[39m\033[38;2;46;209;255m█\033[39m\033[38;2;58;197;255m╗\033[39m\033[38;2;70;185;255m \033[39m\033[38;2;81;174;255m \033[39m\033[38;2;93;162;255m \033[39m\033[38;2;104;151;255m \033[39m\033[38;2;116;139;255m \033[39m\033[38;2;128;127;255m \033[39m\033[38;2;139;116;255m█\033[39m\033[38;2;151;104;255m█\033[39m\033[38;2;162;93;255m█\033[39m\033[38;2;174;81;255m█\033[39m\033[38;2;185;70;255m█\033[39m\033[38;2;197;58;255m█\033[39m\033[38;2;209;46;255m╗\033[39m\033[38;2;220;35;255m \033[39m\033[38;2;232;23;255m█\033[39m\033[38;2;243;12;255m█\033[39m\033[38;2;255;0;255m╗\033[39m\n");
    printf("\033[38;2;255;0;255m█\033[39m\033[38;2;243;12;255m█\033[39m\033[38;2;231;24;255m║\033[39m\033[38;2;219;36;255m \033[39m\033[38;2;206;49;255m \033[39m\033[38;2;194;61;255m█\033[39m\033[38;2;182;73;255m█\033[39m\033[38;2;170;85;255m║\033[39m\033[38;2;158;97;255m█\033[39m\033[38;2;146;109;255m█\033[39m\033[38;2;134;121;255m╔\033[39m\033[38;2;121;134;255m═\033[39m\033[38;2;109;146;255m═\033[39m\033[38;2;97;158;255m═\033[39m\033[38;2;85;170;255m═\033[39m\033[38;2;73;182;255m╝\033[39m\033[38;2;61;194;255m█\033[39m\033[38;2;49;206;255m█\033[39m\033[38;2;36;219;255m║\033[39m\033[38;2;24;231;255m \033[39m\033[38;2;12;243;255m \033[39m\033[38;2;0;255;255m \033[39m\033[38;2;12;243;255m \033[39m\033[38;2;23;232;255m \033[39m\033[38;2;35;220;255m█\033[39m\033[38;2;46;209;255m█\033[39m\033[38;2;58;197;255m║\033[39m\033[38;2;70;185;255m \033[39m\033[38;2;81;174;255m \033[39m\033[38;2;93;162;255m \033[39m\033[38;2;104;151;255m \033[39m\033[38;2;116;139;255m \033[39m\033[38;2;128;127;255m█\033[39m\033[38;2;139;116;255m█\033[39m\033[38;2;151;104;255m╔\033[39m\033[38;2;162;93;255m═\033[39m\033[38;2;174;81;255m═\033[39m\033[38;2;185;70;255m═\033[39m\033[38;2;197;58;255m█\033[39m\033[38;2;209;46;255m█\033[39m\033[38;2;220;35;255m╗\033[39m\033[38;2;232;23;255m█\033[39m\033[38;2;243;12;255m█\033[39m\033[38;2;255;0;255m║\033[39m\n");
    printf("\033[38;2;255;0;255m█\033[39m\033[38;2;243;12;255m█\033[39m\033[38;2;231;24;255m█\033[39m\033[38;2;219;36;255m█\033[39m\033[38;2;206;49;255m█\033[39m\033[38;2;194;61;255m█\033[39m\033[38;2;182;73;255m█\033[39m\033[38;2;170;85;255m║\033[39m\033[38;2;158;97;255m█\033[39m\033[38;2;146;109;255m█\033[39m\033[38;2;134;121;255m█\033[39m\033[38;2;121;134;255m█\033[39m\033[38;2;109;146;255m█\033[39m\033[38;2;97;158;255m╗\033[39m\033[38;2;85;170;255m \033[39m\033[38;2;73;182;255m \033[39m\033[38;2;61;194;255m█\033[39m\033[38;2;49;206;255m█\033[39m\033[38;2;36;219;255m║\033[39m\033[38;2;24;231;255m \033[39m\033[38;2;12;243;255m \033[39m\033[38;2;0;255;255m \033[39m\033[38;2;12;243;255m \033[39m\033[38;2;23;232;255m \033[39m\033[38;2;35;220;255m█\033[39m\033[38;2;46;209;255m█\033[39m\033[38;2;58;197;255m║\033[39m\033[38;2;70;185;255m \033[39m\033[38;2;81;174;255m \033[39m\033[38;2;93;162;255m \033[39m\033[38;2;104;151;255m \033[39m\033[38;2;116;139;255m \033[39m\033[38;2;128;127;255m█\033[39m\033[38;2;139;116;255m█\033[39m\033[38;2;151;104;255m║\033[39m\033[38;2;162;93;255m \033[39m\033[38;2;174;81;255m \033[39m\033[38;2;185;70;255m \033[39m\033[38;2;197;58;255m█\033[39m\033[38;2;209;46;255m█\033[39m\033[38;2;220;35;255m║\033[39m\033[38;2;232;23;255m█\033[39m\033[38;2;243;12;255m█\033[39m\033[38;2;255;0;255m║\033[39m\n");
    printf("\033[38;2;255;0;255m█\033[39m\033[38;2;243;12;255m█\033[39m\033[38;2;231;24;255m╔\033[39m\033[38;2;219;36;255m═\033[39m\033[38;2;206;49;255m═\033[39m\033[38;2;194;61;255m█\033[39m\033[38;2;182;73;255m█\033[39m\033[38;2;170;85;255m║\033[39m\033[38;2;158;97;255m█\033[39m\033[38;2;146;109;255m█\033[39m\033[38;2;134;121;255m╔\033[39m\033[38;2;121;134;255m═\033[39m\033[38;2;109;146;255m═\033[39m\033[38;2;97;158;255m╝\033[39m\033[38;2;85;170;255m \033[39m\033[38;2;73;182;255m \033[39m\033[38;2;61;194;255m█\033[39m\033[38;2;49;206;255m█\033[39m\033[38;2;36;219;255m║\033[39m\033[38;2;24;231;255m \033[39m\033[38;2;12;243;255m \033[39m\033[38;2;0;255;255m \033[39m\033[38;2;12;243;255m \033[39m\033[38;2;23;232;255m \033[39m\033[38;2;35;220;255m█\033[39m\033[38;2;46;209;255m█\033[39m\033[38;2;58;197;255m║\033[39m\033[38;2;70;185;255m \033[39m\033[38;2;81;174;255m \033[39m\033[38;2;93;162;255m \033[39m\033[38;2;104;151;255m \033[39m\033[38;2;116;139;255m \033[39m\033[38;2;128;127;255m█\033[39m\033[38;2;139;116;255m█\033[39m\033[38;2;151;104;255m║\033[39m\033[38;2;162;93;255m \033[39m\033[38;2;174;81;255m \033[39m\033[38;2;185;70;255m \033[39m\033[38;2;197;58;255m█\033[39m\033[38;2;209;46;255m█\033[39m\033[38;2;220;35;255m║\033[39m\033[38;2;232;23;255m╚\033[39m\033[38;2;243;12;255m═\033[39m\033[38;2;255;0;255m╝\033[39m\n");
    printf("\033[38;2;255;0;255m█\033[39m\033[38;2;243;12;255m█\033[39m\033[38;2;231;24;255m║\033[39m\033[38;2;219;36;255m \033[39m\033[38;2;206;49;255m \033[39m\033[38;2;194;61;255m█\033[39m\033[38;2;182;73;255m█\033[39m\033[38;2;170;85;255m║\033[39m\033[38;2;158;97;255m█\033[39m\033[38;2;146;109;255m█\033[39m\033[38;2;134;121;255m█\033[39m\033[38;2;121;134;255m█\033[39m\033[38;2;109;146;255m█\033[39m\033[38;2;97;158;255m█\033[39m\033[38;2;85;170;255m█\033[39m\033[38;2;73;182;255m╗\033[39m\033[38;2;61;194;255m█\033[39m\033[38;2;49;206;255m█\033[39m\033[38;2;36;219;255m█\033[39m\033[38;2;24;231;255m█\033[39m\033[38;2;12;243;255m█\033[39m\033[38;2;0;255;255m█\033[39m\033[38;2;12;243;255m█\033[39m\033[38;2;23;232;255m╗\033[39m\033[38;2;35;220;255m█\033[39m\033[38;2;46;209;255m█\033[39m\033[38;2;58;197;255m█\033[39m\033[38;2;70;185;255m█\033[39m\033[38;2;81;174;255m█\033[39m\033[38;2;93;162;255m█\033[39m\033[38;2;104;151;255m█\033[39m\033[38;2;116;139;255m╗\033[39m\033[38;2;128;127;255m╚\033[39m\033[38;2;139;116;255m█\033[39m\033[38;2;151;104;255m█\033[39m\033[38;2;162;93;255m█\033[39m\033[38;2;174;81;255m█\033[39m\033[38;2;185;70;255m█\033[39m\033[38;2;197;58;255m█\033[39m\033[38;2;209;46;255m╔\033[39m\033[38;2;220;35;255m╝\033[39m\033[38;2;232;23;255m█\033[39m\033[38;2;243;12;255m█\033[39m\033[38;2;255;0;255m╗\033[39m\n");
    printf("\033[38;2;255;0;255m╚\033[39m\033[38;2;243;12;255m═\033[39m\033[38;2;231;24;255m╝\033[39m\033[38;2;219;36;255m \033[39m\033[38;2;206;49;255m \033[39m\033[38;2;194;61;255m╚\033[39m\033[38;2;182;73;255m═\033[39m\033[38;2;170;85;255m╝\033[39m\033[38;2;158;97;255m╚\033[39m\033[38;2;146;109;255m═\033[39m\033[38;2;134;121;255m═\033[39m\033[38;2;121;134;255m═\033[39m\033[38;2;109;146;255m═\033[39m\033[38;2;97;158;255m═\033[39m\033[38;2;85;170;255m═\033[39m\033[38;2;73;182;255m╝\033[39m\033[38;2;61;194;255m╚\033[39m\033[38;2;49;206;255m═\033[39m\033[38;2;36;219;255m═\033[39m\033[38;2;24;231;255m═\033[39m\033[38;2;12;243;255m═\033[39m\033[38;2;0;255;255m═\033[39m\033[38;2;12;243;255m═\033[39m\033[38;2;23;232;255m╝\033[39m\033[38;2;35;220;255m╚\033[39m\033[38;2;46;209;255m═\033[39m\033[38;2;58;197;255m═\033[39m\033[38;2;70;185;255m═\033[39m\033[38;2;81;174;255m═\033[39m\033[38;2;93;162;255m═\033[39m\033[38;2;104;151;255m═\033[39m\033[38;2;116;139;255m╝\033[39m\033[38;2;128;127;255m \033[39m\033[38;2;139;116;255m╚\033[39m\033[38;2;151;104;255m═\033[39m\033[38;2;162;93;255m═\033[39m\033[38;2;174;81;255m═\033[39m\033[38;2;185;70;255m═\033[39m\033[38;2;197;58;255m═\033[39m\033[38;2;209;46;255m╝\033[39m\033[38;2;220;35;255m \033[39m\033[38;2;232;23;255m╚\033[39m\033[38;2;243;12;255m═\033[39m\033[38;2;255;0;255m╝\033[39m\n");
    printf("%s Welcome,%s\n\n", ANSI_CYAN, ANSI_RESET);
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

int ui_select_menu(const char* title, const char* options[], int options_count) {
    if (options_count <= 0) return -1;

    extern int is_interactive;
    if (!is_interactive) {
        // Non-interactive fallback
        printf("\n--- %s ---\n", title);
        for (int i = 0; i < options_count; i++) {
            printf("%d. %s\n", i + 1, options[i]);
        }
        printf("Select option [1-%d]: ", options_count);
        fflush(stdout);
        char input[64];
        if (fgets(input, sizeof(input), stdin) == NULL) return -1;
        int val = atoi(input) - 1;
        if (val >= 0 && val < options_count) return val;
        return -1;
    }

    // Save original terminal settings
    struct termios orig_t;
    if (tcgetattr(STDIN_FILENO, &orig_t) == -1) {
        return -1; 
    }

    struct termios raw = orig_t;
    raw.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);

    int selected = 0;
    int key_pressed = 0;

    // Hide cursor
    printf("\033[?25l");
    fflush(stdout);

    int first_draw = 1;

    while (!key_pressed) {
        // Clear previous output if not first draw
        if (!first_draw) {
            // Move up by (options_count + 4) lines
            printf("\033[%dA", options_count + 4);
        }
        first_draw = 0;

        printf("%s========================================%s\n", ANSI_CYAN, ANSI_RESET);
        printf("  %s%s%s\n", ANSI_BOLD, title, ANSI_RESET);
        printf("%s========================================%s\n", ANSI_CYAN, ANSI_RESET);

        for (int i = 0; i < options_count; i++) {
            if (i == selected) {
                printf("  %s-> %s%s\n", ANSI_GREEN, options[i], ANSI_RESET);
            } else {
                printf("     %s\n", options[i]);
            }
        }
        printf("%s========================================%s\n", ANSI_CYAN, ANSI_RESET);
        fflush(stdout);

        char c;
        int nread = read(STDIN_FILENO, &c, 1);
        if (nread <= 0) continue;

        if (c == '\033') {
            char seq[3];
            // Read arrow keys
            if (read(STDIN_FILENO, &seq[0], 1) > 0 && read(STDIN_FILENO, &seq[1], 1) > 0) {
                if (seq[0] == '[') {
                    if (seq[1] == 'A') { // Up Arrow
                        selected = (selected - 1 + options_count) % options_count;
                    } else if (seq[1] == 'B') { // Down Arrow
                        selected = (selected + 1) % options_count;
                    }
                }
            } else {
                // ESC key
                selected = -1;
                key_pressed = 1;
            }
        } else if (c == '\r' || c == '\n') {
            key_pressed = 1;
        } else if (c == 'q' || c == 'Q') {
            selected = -1;
            key_pressed = 1;
        }
    }

    // Restore cursor and terminal mode
    printf("\033[?25h");
    fflush(stdout);
    tcsetattr(STDIN_FILENO, TCSANOW, &orig_t);

    return selected;
}

