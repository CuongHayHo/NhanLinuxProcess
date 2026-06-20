/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: cli/ui.h
 * Purpose: ANSI Color outputs and CLI UI helpers.
 */

#ifndef UI_H
#define UI_H

#define ANSI_GREEN   "\033[0;32m"
#define ANSI_YELLOW  "\033[0;33m"
#define ANSI_RED     "\033[0;31m"
#define ANSI_BLUE    "\033[0;34m"
#define ANSI_CYAN    "\033[0;36m"
#define ANSI_MAGENTA "\033[0;35m"
#define ANSI_RESET   "\033[0m"
#define ANSI_BOLD    "\033[1m"

void ui_print_success(const char *format, ...);
void ui_print_warning(const char *format, ...);
void ui_print_error(const char *format, ...);
void ui_print_info(const char *format, ...);
void ui_print_title(const char *format, ...);
void ui_print_bold(const char *format, ...);

void ui_clear_screen(void);
void ui_show_banner(const char *version);
void ui_show_help(void);

#endif /* UI_H */
