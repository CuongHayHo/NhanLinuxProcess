/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: include/package_mgr.h
 * Purpose: Package manager enumeration and metadata diagnostics.
 */

#ifndef PACKAGE_MGR_H
#define PACKAGE_MGR_H

/**
 * Runs the interactive Package Manager submenu loop.
 */
void package_mgr_run(void);

/**
 * Enumerate installed software packages on the host system.
 * Returns 0 on success, -1 on failure.
 */
int package_mgr_list_installed(void);

/**
 * Detect the supported package manager (dpkg or rpm).
 * Returns name string, or NULL if not supported.
 */
const char* package_mgr_detect(void);

/**
 * Search installed packages by substring name (case-insensitive).
 * Returns 0 on success, -1 on failure.
 */
int package_mgr_search(const char* query);

/**
 * Query and display detailed metadata for a single installed package.
 * Returns 0 on success, -1 on failure.
 */
int package_mgr_info(const char* pkg_name);

/**
 * Check if a package is installed.
 * Returns 1 if installed, 0 if not installed or error.
 */
int is_package_installed(const char* pkg_name);

/**
 * Install a package on the host system.
 * Returns 0 on success, -1 on failure.
 */
int package_mgr_install(const char* pkg_name);

/**
 * Remove/Uninstall a package from the host system.
 * Returns 0 on success, -1 on failure.
 */
int package_mgr_remove(const char* pkg_name);

/**
 * Runs the safe demonstration process.
 * Returns 0 on success, -1 on failure.
 */
int package_mgr_demo(void);

#endif /* PACKAGE_MGR_H */

