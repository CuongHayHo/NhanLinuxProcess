/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: include/kernel_mgr.h
 * Purpose: Kernel module user space integration manager.
 */

#ifndef KERNEL_MGR_H
#define KERNEL_MGR_H

/**
 * Runs the interactive Kernel Module Manager submenu loop.
 */
void kernel_mgr_run(void);

/**
 * Reads module information from /proc/sysmgr and displays it to the terminal.
 * Returns 0 on success, or -1 on error.
 */
int kernel_mgr_show_info(void);

#endif /* KERNEL_MGR_H */
