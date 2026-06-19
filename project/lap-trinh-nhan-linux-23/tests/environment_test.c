/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: tests/environment_test.c
 * Purpose: Diagnostic unit tests for the Environment Manager.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "logger.h"

int main(void) {
    printf("Starting Environment Manager - Sprint 3 test program...\n\n");

    /* 1. Test getenv() with default variable */
    printf("--- Test 1: Querying default environment variables ---\n");
    char* path = getenv("PATH");
    printf("=> PATH: %s\n", path ? path : "N/A");
    assert(path != NULL);
    printf("Test 1 passed successfully.\n\n");

    /* 2. Test setenv() and getenv() for temporary variable lifecycle */
    printf("--- Test 2: Setting temporary environment variable ---\n");
    int set_res = setenv("TEST_ENV_VAR", "sprint3_value", 1);
    assert(set_res == 0);
    
    char* query_res = getenv("TEST_ENV_VAR");
    printf("=> Query of 'TEST_ENV_VAR': %s\n", query_res ? query_res : "NULL");
    assert(query_res != NULL);
    assert(strcmp(query_res, "sprint3_value") == 0);
    printf("Test 2 passed successfully.\n\n");

    /* 3. Test setenv() overwrite check */
    printf("--- Test 3: Attempting overwrite with setenv(overwrite = 0) ---\n");
    int set_res_no_ovr = setenv("TEST_ENV_VAR", "new_value", 0);
    assert(set_res_no_ovr == 0);
    
    char* query_res_no_ovr = getenv("TEST_ENV_VAR");
    printf("=> Overwrite check value: %s (expected 'sprint3_value')\n", query_res_no_ovr);
    assert(strcmp(query_res_no_ovr, "sprint3_value") == 0);
    
    printf("--- Test 3b: Overwriting with setenv(overwrite = 1) ---\n");
    int set_res_ovr = setenv("TEST_ENV_VAR", "new_value", 1);
    assert(set_res_ovr == 0);
    
    char* query_res_ovr = getenv("TEST_ENV_VAR");
    printf("=> Overwrite check value: %s (expected 'new_value')\n", query_res_ovr);
    assert(strcmp(query_res_ovr, "new_value") == 0);
    printf("Test 3 passed successfully.\n\n");

    /* 4. Test unsetenv() and lifecycle termination */
    printf("--- Test 4: Removing temporary environment variable ---\n");
    int unset_res = unsetenv("TEST_ENV_VAR");
    assert(unset_res == 0);
    
    char* final_query = getenv("TEST_ENV_VAR");
    printf("=> Query after removal: %s (expected NULL)\n", final_query ? final_query : "NULL");
    assert(final_query == NULL);
    printf("Test 4 passed successfully.\n\n");

    printf("All Environment Manager tests completed successfully.\n");
    return 0;
}
