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

/**
 * Checks if the system_monitor module is loaded by scanning /proc/modules.
 */
int is_module_loaded(void);

/**
 * Checks if the system_monitor module is loaded by parsing lsmod output.
 */
int is_module_loaded_via_lsmod(void);

/**
 * Builds and loads the system_monitor kernel module.
 * Returns 0 on success, or -1 on error.
 */
int kernel_mgr_load_module(void);

/**
 * Loads the system_monitor kernel module without sudo (for testing permission failure).
 * Returns 0 on success, or -1 on error.
 */
int kernel_mgr_load_module_no_sudo(void);

/**
 * Unloads the system_monitor kernel module.
 * Returns 0 on success, or -1 on error.
 */
int kernel_mgr_unload_module(void);

/**
 * Displays module status including loaded state, kernel version, module version,
 * and /proc/sysmgr availability.
 * Returns 0 on success, or -1 on error.
 */
int kernel_mgr_show_status(void);

/**
 * Captures dmesg log entries and displays the last 20 matching entries.
 * Returns 0 on success, or -1 on error.
 */
int kernel_mgr_show_log(void);

#endif /* KERNEL_MGR_H */

