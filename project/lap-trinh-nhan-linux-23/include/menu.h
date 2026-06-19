/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: include/menu.h
 * Purpose: Terminal User Interface (TUI) menu functions.
 */

#ifndef MENU_H
#define MENU_H

/* ANSI Escape Codes for Terminal Styling */
#define ANSI_COLOR_RESET   "\x1b[0m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"

/**
 * @brief Clears the terminal screen.
 *
 * Purpose: Provides a clean terminal display before rendering menus.
 * Parameters: None.
 * Return value: None.
 */
void menu_clear_screen(void);

/**
 * @brief Displays the application main menu to the user.
 *
 * Purpose: Renders the structured selection menu using ANSI coloring.
 * Parameters: None.
 * Return value: None.
 */
void menu_display_main(void);

/**
 * @brief Reads and validates the user's menu choice from stdin.
 *
 * Purpose: Extracts an integer selection, validating against bad inputs 
 *          (e.g., characters, out of bounds, empty buffers) to prevent infinite loops.
 * Parameters: None.
 * Return value: The validated option integer (0-11) or -1 on error.
 */
int menu_read_choice(void);

/**
 * @brief Pauses execution and prompts the user to press Enter.
 *
 * Purpose: Blocks execution until the user presses Enter, allowing them 
 *          to read status messages before the menu redraws.
 * Parameters: None.
 * Return value: None.
 */
void menu_pause(void);

#endif /* MENU_H */
