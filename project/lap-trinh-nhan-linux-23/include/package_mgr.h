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

#endif /* PACKAGE_MGR_H */
