/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: tests/shell_test.c
 * Purpose: Diagnostic unit tests for Shell Manager module.
 */

#include <stdio.h>
#include <assert.h>
#include "shell_mgr.h"

int main(void) {
    printf("Starting Shell Manager - Sprint 1 test program...\n\n");

    /* 1. Verify pwd execution */
    printf("--- Test 1: executing 'pwd' ---\n");
    int status_pwd = shell_mgr_execute("pwd");
    printf("=> Result status: %d\n", status_pwd);
    assert(status_pwd == 0);
    printf("Test 1 passed successfully.\n\n");

    /* 2. Verify whoami execution */
    printf("--- Test 2: executing 'whoami' ---\n");
    int status_whoami = shell_mgr_execute("whoami");
    printf("=> Result status: %d\n", status_whoami);
    assert(status_whoami == 0);
    printf("Test 2 passed successfully.\n\n");

    /* 3. Verify ls execution */
    printf("--- Test 3: executing 'ls' ---\n");
    int status_ls = shell_mgr_execute("ls");
    printf("=> Result status: %d\n", status_ls);
    assert(status_ls == 0);
    printf("Test 3 passed successfully.\n\n");

    /* 4. Verify invalid command execution */
    printf("--- Test 4: executing 'invalid_command_xyz123' ---\n");
    int status_invalid = shell_mgr_execute("invalid_command_xyz123");
    printf("=> Result status: %d\n", status_invalid);
    /* Expecting standard exit status 127 for command not found */
    assert(status_invalid == 127);
    printf("Test 4 passed successfully.\n\n");

    printf("All Shell Manager tests completed successfully.\n");
    return 0;
}
