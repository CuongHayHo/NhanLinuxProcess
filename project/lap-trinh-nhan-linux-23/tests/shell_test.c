/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: tests/shell_test.c
 * Purpose: Consolidated unit and integration tests for the Shell Manager launcher.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include "shell_mgr.h"

/* Dummy definitions for CLI and TUI subsystems to satisfy links in shell_mgr.o */
int is_interactive = 0;
int ui_select_menu(const char* title, const char* options[], int options_count) {
    (void)title; (void)options; (void)options_count;
    return -1;
}
char* linenoise(const char* prompt) {
    (void)prompt;
    return NULL;
}
void linenoiseFree(void* ptr) {
    (void)ptr;
}

/* 1. Raw command execution tests */
static void run_raw_commands_tests(void) {
    printf("=== Section 1: Raw Command Execution ===\n");

    /* Verify pwd execution */
    printf("--- Test 1.1: executing 'pwd' ---\n");
    int status_pwd = shell_mgr_execute("pwd");
    printf("=> Result status: %d\n", status_pwd);
    assert(status_pwd == 0);
    printf("Test 1.1 passed successfully.\n\n");

    /* Verify whoami execution */
    printf("--- Test 1.2: executing 'whoami' ---\n");
    int status_whoami = shell_mgr_execute("whoami");
    printf("=> Result status: %d\n", status_whoami);
    assert(status_whoami == 0);
    printf("Test 1.2 passed successfully.\n\n");

    /* Verify ls execution */
    printf("--- Test 1.3: executing 'ls' ---\n");
    int status_ls = shell_mgr_execute("ls");
    printf("=> Result status: %d\n", status_ls);
    assert(status_ls == 0);
    printf("Test 1.3 passed successfully.\n\n");

    /* Verify invalid command execution */
    printf("--- Test 1.4: executing 'invalid_command_xyz123' ---\n");
    int status_invalid = shell_mgr_execute("invalid_command_xyz123");
    printf("=> Result status: %d\n", status_invalid);
    assert(status_invalid == 127);
    printf("Test 1.4 passed successfully.\n\n");
}

/* 2. Script runner tests */
static void run_script_runner_tests(void) {
    printf("=== Section 2: Script Runner ===\n");

    /* Verify backup.sh execution */
    printf("--- Test 2.1: executing 'backup.sh' ---\n");
    int status_backup = shell_mgr_execute("/bin/bash scripts/backup.sh");
    printf("=> Result status: %d\n", status_backup);
    assert(status_backup == 0);
    printf("Test 2.1 passed successfully.\n\n");

    /* Verify disk_usage.sh execution */
    printf("--- Test 2.2: executing 'disk_usage.sh' ---\n");
    int status_disk = shell_mgr_execute("/bin/bash scripts/disk_usage.sh");
    printf("=> Result status: %d\n", status_disk);
    assert(status_disk == 0);
    printf("Test 2.2 passed successfully.\n\n");

    /* Verify show_date.sh execution */
    printf("--- Test 2.3: executing 'show_date.sh' ---\n");
    int status_date = shell_mgr_execute("/bin/bash scripts/show_date.sh");
    printf("=> Result status: %d\n", status_date);
    assert(status_date == 0);
    printf("Test 2.3 passed successfully.\n\n");

    /* Verify invalid_script.sh execution (should exit 127) */
    printf("--- Test 2.4: executing 'invalid_script.sh' (expected to fail) ---\n");
    int status_invalid = shell_mgr_execute("/bin/bash scripts/invalid_script.sh");
    printf("=> Result status: %d\n", status_invalid);
    assert(status_invalid == 127);
    printf("Test 2.4 passed successfully.\n\n");
}

/* 3. Environment manager tests */
static void run_environment_tests(void) {
    printf("=== Section 3: Environment Manager ===\n");

    /* Test getenv() with default variable */
    printf("--- Test 3.1: Querying default environment variables ---\n");
    char* path = getenv("PATH");
    printf("=> PATH: %s\n", path ? path : "N/A");
    assert(path != NULL);
    printf("Test 3.1 passed successfully.\n\n");

    /* Test setenv() and getenv() for temporary variable lifecycle */
    printf("--- Test 3.2: Setting temporary environment variable ---\n");
    int set_res = setenv("TEST_ENV_VAR", "sprint3_value", 1);
    assert(set_res == 0);
    
    char* query_res = getenv("TEST_ENV_VAR");
    printf("=> Query of 'TEST_ENV_VAR': %s\n", query_res ? query_res : "NULL");
    assert(query_res != NULL);
    assert(strcmp(query_res, "sprint3_value") == 0);
    printf("Test 3.2 passed successfully.\n\n");

    /* Test setenv() overwrite check */
    printf("--- Test 3.3: Attempting overwrite with setenv(overwrite = 0) ---\n");
    int set_res_no_ovr = setenv("TEST_ENV_VAR", "new_value", 0);
    assert(set_res_no_ovr == 0);
    
    char* query_res_no_ovr = getenv("TEST_ENV_VAR");
    printf("=> Overwrite check value: %s (expected 'sprint3_value')\n", query_res_no_ovr);
    assert(strcmp(query_res_no_ovr, "sprint3_value") == 0);
    
    printf("--- Test 3.3b: Overwriting with setenv(overwrite = 1) ---\n");
    int set_res_ovr = setenv("TEST_ENV_VAR", "new_value", 1);
    assert(set_res_ovr == 0);
    
    char* query_res_ovr = getenv("TEST_ENV_VAR");
    printf("=> Overwrite check value: %s (expected 'new_value')\n", query_res_ovr);
    assert(strcmp(query_res_ovr, "new_value") == 0);
    printf("Test 3.3 passed successfully.\n\n");

    /* Test unsetenv() and lifecycle termination */
    printf("--- Test 3.4: Removing temporary environment variable ---\n");
    int unset_res = unsetenv("TEST_ENV_VAR");
    assert(unset_res == 0);
    
    char* final_query = getenv("TEST_ENV_VAR");
    printf("=> Query after removal: %s (expected NULL)\n", final_query ? final_query : "NULL");
    assert(final_query == NULL);
    printf("Test 3.4 passed successfully.\n\n");
}

/* 4. Sanity presence checks for Part 1 assignment scripts */
static void run_script_presence_tests(void) {
    printf("=== Section 4: Shell Script Sanity Checks ===\n");

    const char* scripts[] = {
        "shell/file.sh",
        "shell/crontab.sh",
        "shell/time.sh",
        "shell/program.sh"
    };

    for (size_t i = 0; i < sizeof(scripts) / sizeof(scripts[0]); i++) {
        printf("--- Test 4.%d: checking existence & readability of '%s' ---\n", (int)i + 1, scripts[i]);
        assert(access(scripts[i], F_OK) == 0);
        assert(access(scripts[i], R_OK) == 0);
        printf("   Passed: '%s' exists and is readable.\n\n", scripts[i]);
    }
}

int main(void) {
    printf("Starting Consolidated Shell Manager unit/regression tests...\n\n");

    run_raw_commands_tests();
    run_script_runner_tests();
    run_environment_tests();
    run_script_presence_tests();

    printf("All Shell Manager unit/regression tests completed successfully.\n");
    return 0;
}
