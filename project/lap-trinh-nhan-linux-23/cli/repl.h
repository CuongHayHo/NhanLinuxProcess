/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: cli/repl.h
 * Purpose: Interactive CLI / REPL interface entry point.
 */

#ifndef REPL_H
#define REPL_H

/**
 * Global flag indicating whether sysmgr is running in interactive mode.
 * 1 = interactive (menu or REPL), 0 = direct non-interactive CLI.
 */
extern int is_interactive;

/**
 * Initializes and runs the main interactive REPL CLI command loop.
 */
void repl_run(void);

/**
 * Executes a single command line directly without running the interactive loop.
 * Supported for future GUI launchers or script-like invocation.
 */
void repl_execute_command(const char* cmd_line);

#endif /* REPL_H */
