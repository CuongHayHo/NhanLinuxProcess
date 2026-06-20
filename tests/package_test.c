/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: tests/package_test.c
 * Purpose: Non-destructive regression tests for Shell Programming Package Manager.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "package_mgr.h"
#include "logger.h"

/* Dummy definitions for CLI and TUI subsystems to satisfy links in package_mgr.o */
int is_interactive = 0;
int ui_select_menu(const char* title, const char* options[], int options_count) {
    (void)title; (void)options; (void)options_count;
    return -1;
}
void print_prompt_explanation(const char* prompt_msg) {
    (void)prompt_msg;
}

int main(void) {
    printf("Starting Package Manager Regression Test Suite...\n\n");

    // 1. Verify script exists
    printf("[TEST] Checking if shell/program.sh exists...\n");
    if (access("shell/program.sh", F_OK) == 0) {
        printf("PASS: shell/program.sh exists.\n");
    } else {
        printf("FAIL: shell/program.sh is missing!\n");
        return 1;
    }

    // 2. Verify script executes
    printf("[TEST] Checking if shell/program.sh is executable...\n");
    if (access("shell/program.sh", X_OK) == 0) {
        printf("PASS: shell/program.sh is executable.\n");
    } else {
        printf("FAIL: shell/program.sh is not executable!\n");
        return 1;
    }

    // 3. Verify search works (non-destructive search for 'bash' which exists on all systems)
    printf("[TEST] Verifying package search functionality...\n");
    int search_res = package_mgr_search("bash");
    if (search_res == 0) {
        printf("PASS: package_mgr_search('bash') succeeded.\n");
    } else {
        printf("FAIL: package_mgr_search('bash') failed.\n");
        return 1;
    }

    // 4. Verify information works
    printf("[TEST] Verifying package information functionality...\n");
    int info_res = package_mgr_info("bash");
    if (info_res == 0) {
        printf("PASS: package_mgr_info('bash') succeeded.\n");
    } else {
        printf("FAIL: package_mgr_info('bash') failed.\n");
        return 1;
    }

    // 5. Verify invalid package handling
    printf("[TEST] Verifying invalid package handling...\n");
    int invalid_res = package_mgr_info("nonexistent_package_12345");
    if (invalid_res != 0) {
        printf("PASS: package_mgr_info('nonexistent_package_12345') failed as expected.\n");
    } else {
        printf("FAIL: package_mgr_info('nonexistent_package_12345') unexpectedly succeeded.\n");
        return 1;
    }

    // 6. Verify launcher works
    printf("[TEST] Checking package manager detection...\n");
    const char *pm = package_mgr_detect();
    if (pm) {
        printf("PASS: Package manager detected: %s\n", pm);
    } else {
        printf("FAIL: No supported package manager detected.\n");
        return 1;
    }

    printf("\nAll Package Manager regression tests completed successfully.\n");
    return 0;
}
