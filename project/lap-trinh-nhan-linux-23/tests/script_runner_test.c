/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: tests/script_runner_test.c
 * Purpose: Diagnostic unit tests for the Shell Manager Script Runner.
 */

#include <stdio.h>
#include <assert.h>
#include "shell_mgr.h"

int main(void) {
    printf("Starting Script Runner - Sprint 2 test program...\n\n");

    /* 1. Verify backup.sh execution */
    printf("--- Test 1: executing 'backup.sh' ---\n");
    int status_backup = shell_mgr_execute("/bin/bash scripts/backup.sh");
    printf("=> Result status: %d\n", status_backup);
    assert(status_backup == 0);
    printf("Test 1 passed successfully.\n\n");

    /* 2. Verify disk_usage.sh execution */
    printf("--- Test 2: executing 'disk_usage.sh' ---\n");
    int status_disk = shell_mgr_execute("/bin/bash scripts/disk_usage.sh");
    printf("=> Result status: %d\n", status_disk);
    assert(status_disk == 0);
    printf("Test 2 passed successfully.\n\n");

    /* 3. Verify show_date.sh execution */
    printf("--- Test 3: executing 'show_date.sh' ---\n");
    int status_date = shell_mgr_execute("/bin/bash scripts/show_date.sh");
    printf("=> Result status: %d\n", status_date);
    assert(status_date == 0);
    printf("Test 3 passed successfully.\n\n");

    /* 4. Verify invalid_script.sh execution (should exit 127 due to bash command failure) */
    printf("--- Test 4: executing 'invalid_script.sh' (expected to fail) ---\n");
    int status_invalid = shell_mgr_execute("/bin/bash scripts/invalid_script.sh");
    printf("=> Result status: %d\n", status_invalid);
    assert(status_invalid == 127);
    printf("Test 4 passed successfully.\n\n");

    printf("All Script Runner tests completed successfully.\n");
    return 0;
}
