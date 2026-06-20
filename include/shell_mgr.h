/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: include/shell_mgr.h
 * Purpose: Safe program execution shell manager interfaces.
 */

#ifndef SHELL_MGR_H
#define SHELL_MGR_H

/**
 * Runs the interactive Shell Manager submenu loop.
 */
void shell_mgr_run(void);

/**
 * Executes a raw command string using fork() + execvp() + waitpid().
 * Returns the command's exit status, or -1 on error.
 */
int shell_mgr_execute(const char* command);

/**
 * Runs the interactive environment variable manager submenu.
 */
void shell_mgr_env_run(void);

/**
 * Execute cron task scheduling subcommand or TUI.
 */
void shell_mgr_cron_execute(int argc, char** argv);

/**
 * Execute system time configuration subcommand or TUI.
 */
void shell_mgr_time_execute(int argc, char** argv);

#endif /* SHELL_MGR_H */
