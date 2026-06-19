/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: modules/package/package_mgr.c
 * Purpose: Package installer wrapper around apt/dpkg.
 */

#include <stdio.h>
#include <stdlib.h>
#include "package_mgr.h"
#include "logger.h"

void package_mgr_run(void) {
    printf("--- Package Installer (Stub) ---\n");
    printf("1. Install Package Stub\n");
    printf("2. Search Package Stub\n");
    printf("0. Return\n");
    log_info("PACKAGE", "Package manager menu invoked.");
    
    /* TODO: Implement package option dispatcher */
}

int package_mgr_install(const char* pkg) {
    (void)pkg;
    log_info("PACKAGE", "package_mgr_install called (stub).");
    /* TODO: fork/exec sudo apt-get install */
    return 0;
}

int package_mgr_remove(const char* pkg) {
    (void)pkg;
    log_info("PACKAGE", "package_mgr_remove called (stub).");
    /* TODO: fork/exec sudo apt-get remove */
    return 0;
}

int package_mgr_search(const char* pkg) {
    (void)pkg;
    log_info("PACKAGE", "package_mgr_search called (stub).");
    /* TODO: fork/exec apt-cache search */
    return 0;
}
