/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: tests/semaphore_test.c
 * Purpose: Complete Diagnostic tests and demonstrations for POSIX Named Semaphores - Sprint 2.
 */

#include <stdio.h>
#include "ipc_mgr.h"

int main(void) {
    printf("Starting IPC Manager - Sprint 2 (POSIX Named Semaphore) test program...\n\n");

    /* Execute POSIX Named Semaphore demonstration */
    int result = ipc_mgr_sem_demo();
    
    if (result == 0) {
        printf("\nPOSIX Named Semaphore IPC test completed successfully.\n");
    } else {
        printf("\nPOSIX Named Semaphore IPC test failed.\n");
    }

    return result;
}
