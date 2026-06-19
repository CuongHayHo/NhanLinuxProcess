/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: include/time_mgr.h
 * Purpose: Time manager submenu and inspection declarations.
 */

#ifndef TIME_MGR_H
#define TIME_MGR_H

/**
 * Runs the interactive Time Manager submenu loop.
 */
void time_mgr_run(void);

/**
 * Retrieves and prints the system time, UTC time, Unix epoch, timezone, uptime, and boot time.
 * Returns 0 on success, or -1 on error.
 */
int time_mgr_show_current(void);

#endif /* TIME_MGR_H */
