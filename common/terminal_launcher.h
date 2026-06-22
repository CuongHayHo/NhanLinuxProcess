/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: common/terminal_launcher.h
 * Purpose: Terminal detection and external window execution helper.
 */

#ifndef TERMINAL_LAUNCHER_H
#define TERMINAL_LAUNCHER_H

/**
 * Launches the given command and arguments in a new external terminal window.
 * Returns 0 on success, or -1 on failure.
 */
int terminal_open(const char* title, const char* command);
void print_ssh_tunnel_guide(int port);

#endif /* TERMINAL_LAUNCHER_H */
