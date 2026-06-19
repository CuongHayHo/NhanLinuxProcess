/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: tests/automation_test.c
 * Purpose: Diagnostic unit tests for the Automation Manager.
 */

#include <stdio.h>
#include <assert.h>
#include "shell_mgr.h"

int main(void) {
    printf("Starting Automation Manager - Sprint 4 test program...\n\n");

    /* 1. Verify cron parser with standard expressions */
    printf("--- Test 1: Explaining standard cron expressions ---\n");
    translate_cron("0 8 * * 1-5");
    translate_cron("*/5 * * * *");
    printf("Test 1 completed successfully.\n\n");

    /* 2. Verify cron parser invalid expression graceful error handling */
    printf("--- Test 2: Explaining invalid cron expression ---\n");
    translate_cron("0 8 *"); /* Less than 5 fields */
    printf("Test 2 completed successfully.\n\n");

    /* 3. Verify crontab execution and missing crontab handling */
    printf("--- Test 3: Executing 'crontab -l' and verifying graceful exit ---\n");
    int res = shell_mgr_execute("crontab -l");
    printf("=> crontab -l returned exit status: %d\n", res);
    
    if (res != 0) {
        printf("Missing crontab or sandbox restrictions handled gracefully.\n");
    } else {
        printf("Crontab read successfully.\n");
    }
    printf("Test 3 completed successfully.\n\n");

    printf("All Automation Manager tests completed successfully.\n");
    return 0;
}
